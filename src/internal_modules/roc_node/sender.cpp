/*
 * Copyright (c) 2020 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_node/sender.h"
#include "roc_address/endpoint_uri_to_str.h"
#include "roc_address/socket_addr.h"
#include "roc_address/socket_addr_to_str.h"
#include "roc_core/log.h"
#include "roc_core/panic.h"

namespace roc {
namespace node {

Sender::Sender(Context& context, const pipeline::SenderConfig& pipeline_config)
    : Node(context)
    , pipeline_(*this,
                pipeline_config,
                context.format_map(),
                context.packet_factory(),
                context.byte_buffer_factory(),
                context.sample_buffer_factory(),
                context.arena())
    , processing_task_(pipeline_)
    , slot_pool_("slot_pool", context.arena())
    , slot_map_(context.arena())
    , valid_(false) {
    roc_log(LogDebug, "sender node: initializing");

    memset(used_interfaces_, 0, sizeof(used_interfaces_));
    memset(used_protocols_, 0, sizeof(used_protocols_));

    if (!pipeline_.is_valid()) {
        return;
    }

    valid_ = true;
}

Sender::~Sender() {
    roc_log(LogDebug, "sender node: deinitializing");

    // First remove all slots. This may involve usage of processing task.
    while (core::SharedPtr<Slot> slot = slot_map_.front()) {
        cleanup_slot_(*slot);
        slot_map_.remove(*slot);
    }

    // Then wait until processing task is fully completed, before
    // proceeding to its destruction.
    context().control_loop().wait(processing_task_);
}

bool Sender::is_valid() const {
    return valid_;
}

bool Sender::configure(slot_index_t slot_index,
                       address::Interface iface,
                       const netio::UdpSenderConfig& config) {
    core::Mutex::Lock lock(mutex_);

    roc_panic_if_not(is_valid());

    roc_panic_if(iface < 0);
    roc_panic_if(iface >= (int)address::Iface_Max);

    roc_log(LogDebug, "sender node: configuring %s interface of slot %lu",
            address::interface_to_str(iface), (unsigned long)slot_index);

    core::SharedPtr<Slot> slot = get_slot_(slot_index, true);
    if (!slot) {
        roc_log(LogError,
                "sender node:"
                " can't configure %s interface of slot %lu:"
                " can't create slot",
                address::interface_to_str(iface), (unsigned long)slot_index);
        return false;
    }

    if (slot->broken) {
        roc_log(LogError,
                "sender node:"
                " can't configure %s interface of slot %lu:"
                " slot is marked broken and should be unlinked",
                address::interface_to_str(iface), (unsigned long)slot_index);
        return false;
    }

    if (slot->ports[iface].handle) {
        roc_log(LogError,
                "sender node:"
                " can't configure %s interface of slot %lu:"
                " interface is already bound or connected",
                address::interface_to_str(iface), (unsigned long)slot_index);
        break_slot_(*slot);
        return false;
    }

    slot->ports[iface].config = config;

    return true;
}

bool Sender::connect(slot_index_t slot_index,
                     address::Interface iface,
                     const address::EndpointUri& uri) {
    core::Mutex::Lock lock(mutex_);

    roc_panic_if_not(is_valid());

    roc_panic_if(iface < 0);
    roc_panic_if(iface >= (int)address::Iface_Max);

    roc_log(LogInfo, "sender node: connecting %s interface of slot %lu to %s",
            address::interface_to_str(iface), (unsigned long)slot_index,
            address::endpoint_uri_to_str(uri).c_str());

    core::SharedPtr<Slot> slot = get_slot_(slot_index, true);
    if (!slot) {
        roc_log(LogError,
                "sender node:"
                " can't connect %s interface of slot %lu:"
                " can't create slot",
                address::interface_to_str(iface), (unsigned long)slot_index);
        return false;
    }

    if (slot->broken) {
        roc_log(LogError,
                "sender node:"
                " can't connect %s interface of slot %lu:"
                " slot is marked broken and should be unlinked",
                address::interface_to_str(iface), (unsigned long)slot_index);
        return false;
    }

    if (!uri.verify(address::EndpointUri::Subset_Full)) {
        roc_log(LogError,
                "sender node: can't connect %s interface of slot %lu: invalid uri",
                address::interface_to_str(iface), (unsigned long)slot_index);
        break_slot_(*slot);
        return false;
    }

    if (!check_compatibility_(iface, uri)) {
        roc_log(LogError,
                "sender node:"
                " can't connect %s interface of slot %lu:"
                " incompatible with other slots",
                address::interface_to_str(iface), (unsigned long)slot_index);
        break_slot_(*slot);
        return false;
    }

    netio::NetworkLoop::Tasks::ResolveEndpointAddress resolve_task(uri);

    if (!context().network_loop().schedule_and_wait(resolve_task)) {
        roc_log(LogError,
                "sender node:"
                " can't connect %s interface of slot %lu:"
                " can't resolve endpoint address",
                address::interface_to_str(iface), (unsigned long)slot_index);
        break_slot_(*slot);
        return false;
    }

    const address::SocketAddr& address = resolve_task.get_address();

    Port& port = select_outgoing_port_(*slot, iface, address.family());

    if (!setup_outgoing_port_(port, iface, address.family())) {
        roc_log(LogError,
                "sender node:"
                " can't connect %s interface of slot %lu:"
                " can't bind to local port",
                address::interface_to_str(iface), (unsigned long)slot_index);
        break_slot_(*slot);
        return false;
    }

    pipeline::SenderLoop::Tasks::AddEndpoint endpoint_task(
        slot->handle, iface, uri.proto(), address, *port.writer);
    if (!pipeline_.schedule_and_wait(endpoint_task)) {
        roc_log(LogError,
                "sender node:"
                " can't connect %s interface of slot %lu:"
                " can't add endpoint to pipeline",
                address::interface_to_str(iface), (unsigned long)slot_index);
        break_slot_(*slot);
        return false;
    }

    update_compatibility_(iface, uri);

    return true;
}

bool Sender::unlink(slot_index_t slot_index) {
    core::Mutex::Lock lock(mutex_);

    roc_panic_if_not(is_valid());

    roc_log(LogDebug, "sender node: unlinking slot %lu", (unsigned long)slot_index);

    core::SharedPtr<Slot> slot = get_slot_(slot_index, false);
    if (!slot) {
        roc_log(LogError,
                "sender node:"
                " can't unlink slot %lu: can't find slot",
                (unsigned long)slot_index);
        return false;
    }

    cleanup_slot_(*slot);
    slot_map_.remove(*slot);

    return true;
}

bool Sender::get_metrics(slot_index_t slot_index,
                         pipeline::SenderSlotMetrics& slot_metrics,
                         pipeline::SenderSessionMetrics& sess_metrics) {
    core::Mutex::Lock lock(mutex_);

    roc_panic_if_not(is_valid());

    core::SharedPtr<Slot> slot = get_slot_(slot_index, false);
    if (!slot) {
        roc_log(LogError,
                "sender node:"
                " can't get metrics of slot %lu: can't find slot",
                (unsigned long)slot_index);
        return false;
    }

    pipeline::SenderLoop::Tasks::QuerySlot task(slot->handle, slot_metrics,
                                                &sess_metrics);
    if (!pipeline_.schedule_and_wait(task)) {
        roc_log(LogError,
                "sender node:"
                " can't get metrics of slot %lu: operation failed",
                (unsigned long)slot_index);
        return false;
    }

    return true;
}

bool Sender::has_incomplete() {
    core::Mutex::Lock lock(mutex_);

    roc_panic_if_not(is_valid());

    for (core::SharedPtr<Slot> slot = slot_map_.front(); slot;
         slot = slot_map_.nextof(*slot)) {
        if (slot->broken) {
            return true;
        }

        if (slot->handle) {
            pipeline::SenderSlotMetrics slot_metrics;
            pipeline::SenderLoop::Tasks::QuerySlot task(slot->handle, slot_metrics, NULL);
            if (!pipeline_.schedule_and_wait(task)) {
                return true;
            }
            if (!slot_metrics.is_complete) {
                return true;
            }
        }
    }

    return false;
}

bool Sender::has_broken() {
    core::Mutex::Lock lock(mutex_);

    roc_panic_if_not(is_valid());

    for (core::SharedPtr<Slot> slot = slot_map_.front(); slot;
         slot = slot_map_.nextof(*slot)) {
        if (slot->broken) {
            return true;
        }
    }

    return false;
}

sndio::ISink& Sender::sink() {
    roc_panic_if_not(is_valid());

    return pipeline_.sink();
}

bool Sender::check_compatibility_(address::Interface iface,
                                  const address::EndpointUri& uri) {
    if (used_interfaces_[iface] && used_protocols_[iface] != uri.proto()) {
        roc_log(LogError,
                "sender node: same interface of all slots should use same protocols:"
                " other slot uses %s, but this slot tries to use %s",
                address::proto_to_str(used_protocols_[iface]),
                address::proto_to_str(uri.proto()));
        return false;
    }

    return true;
}

void Sender::update_compatibility_(address::Interface iface,
                                   const address::EndpointUri& uri) {
    used_interfaces_[iface] = true;
    used_protocols_[iface] = uri.proto();
}

core::SharedPtr<Sender::Slot> Sender::get_slot_(slot_index_t slot_index,
                                                bool auto_create) {
    core::SharedPtr<Slot> slot = slot_map_.find(slot_index);

    if (!slot) {
        if (auto_create) {
            pipeline::SenderLoop::Tasks::CreateSlot task;
            if (!pipeline_.schedule_and_wait(task)) {
                roc_log(LogError, "sender node: failed to create slot %lu",
                        (unsigned long)slot_index);
                return NULL;
            }

            slot = new (slot_pool_) Slot(slot_pool_, slot_index, task.get_handle());
            if (!slot) {
                roc_log(LogError, "sender node: failed to create slot %lu",
                        (unsigned long)slot_index);
                return NULL;
            }

            if (!slot_map_.grow()) {
                roc_log(LogError, "sender node: failed to create slot %lu",
                        (unsigned long)slot_index);
                return NULL;
            }

            slot_map_.insert(*slot);
        } else {
            roc_log(LogError, "sender node: failed to find slot %lu",
                    (unsigned long)slot_index);
            return NULL;
        }
    }

    return slot;
}

void Sender::cleanup_slot_(Slot& slot) {
    // First remove pipeline slot, because it writes to network ports.
    if (slot.handle) {
        pipeline::SenderLoop::Tasks::DeleteSlot task(slot.handle);
        if (!pipeline_.schedule_and_wait(task)) {
            roc_panic("sender node: can't remove pipeline slot %lu",
                      (unsigned long)slot.index);
        }
        slot.handle = NULL;
    }

    // Then remove network ports.
    for (size_t p = 0; p < address::Iface_Max; p++) {
        if (slot.ports[p].handle) {
            netio::NetworkLoop::Tasks::RemovePort task(slot.ports[p].handle);
            if (!context().network_loop().schedule_and_wait(task)) {
                roc_panic("sender node: can't remove network port of slot %lu",
                          (unsigned long)slot.index);
            }
            slot.ports[p].handle = NULL;
        }
    }
}

void Sender::break_slot_(Slot& slot) {
    roc_log(LogError, "sender node: marking slot %lu as broken, it needs to be unlinked",
            (unsigned long)slot.index);

    slot.broken = true;
    cleanup_slot_(slot);
}

Sender::Port& Sender::select_outgoing_port_(Slot& slot,
                                            address::Interface iface,
                                            address::AddrFamily family) {
    // We try to share outgoing port for source and repair interfaces, if they have
    // identical configuratrion. This should not harm, and it may help receiver to
    // associate source and repair streams together, in case when no control and
    // signaling protocol is used, by source addresses. This technique is neither
    // standard nor universal, but in many cases it allows us to work even without
    // protocols like RTCP or RTSP.
    const bool share_interface_ports =
        (iface == address::Iface_AudioSource || iface == address::Iface_AudioRepair);

    if (share_interface_ports && !slot.ports[iface].handle) {
        for (size_t i = 0; i < address::Iface_Max; i++) {
            if ((int)i == iface) {
                continue;
            }
            if (!slot.ports[i].handle) {
                continue;
            }
            if (!(slot.ports[i].orig_config == slot.ports[iface].config)) {
                continue;
            }
            if (!(slot.ports[i].config.bind_address.family() == family)) {
                continue;
            }

            roc_log(LogDebug, "sender node: sharing %s interface port with %s interface",
                    address::interface_to_str(address::Interface(i)),
                    address::interface_to_str(iface));

            return slot.ports[i];
        }
    }

    return slot.ports[iface];
}

bool Sender::setup_outgoing_port_(Port& port,
                                  address::Interface iface,
                                  address::AddrFamily family) {
    if (port.config.bind_address.has_host_port()) {
        if (port.config.bind_address.family() != family) {
            roc_log(LogError,
                    "sender node:"
                    " %s interface is configured to use %s,"
                    " but tried to be connected to %s address",
                    address::interface_to_str(iface),
                    address::addr_family_to_str(port.config.bind_address.family()),
                    address::addr_family_to_str(family));
            return false;
        }
    }

    if (!port.handle) {
        port.orig_config = port.config;

        if (!port.config.bind_address.has_host_port()) {
            if (family == address::Family_IPv4) {
                if (!port.config.bind_address.set_host_port(address::Family_IPv4,
                                                            "0.0.0.0", 0)) {
                    roc_panic("sender node: can't set reset %s interface ipv4 address",
                              address::interface_to_str(iface));
                }
            } else {
                if (!port.config.bind_address.set_host_port(address::Family_IPv6,
                                                            "::", 0)) {
                    roc_panic("sender node: can't set reset %s interface ipv6 address",
                              address::interface_to_str(iface));
                }
            }
        }

        netio::NetworkLoop::Tasks::AddUdpSenderPort port_task(port.config);

        if (!context().network_loop().schedule_and_wait(port_task)) {
            roc_log(LogError, "sender node: can't bind %s interface to local port",
                    address::interface_to_str(iface));
            return false;
        }

        port.handle = port_task.get_handle();
        port.writer = port_task.get_writer();

        roc_log(LogInfo, "sender node: bound %s interface to %s",
                address::interface_to_str(iface),
                address::socket_addr_to_str(port.config.bind_address).c_str());
    }

    return true;
}

void Sender::schedule_task_processing(pipeline::PipelineLoop&,
                                      core::nanoseconds_t deadline) {
    context().control_loop().schedule_at(processing_task_, deadline, NULL);
}

void Sender::cancel_task_processing(pipeline::PipelineLoop&) {
    context().control_loop().async_cancel(processing_task_);
}

} // namespace node
} // namespace roc
