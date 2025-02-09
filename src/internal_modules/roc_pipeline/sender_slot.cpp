/*
 * Copyright (c) 2020 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_pipeline/sender_slot.h"
#include "roc_core/log.h"
#include "roc_core/panic.h"
#include "roc_pipeline/endpoint_helpers.h"

namespace roc {
namespace pipeline {

SenderSlot::SenderSlot(const SenderConfig& config,
                       const rtp::FormatMap& format_map,
                       audio::Fanout& fanout,
                       packet::PacketFactory& packet_factory,
                       core::BufferFactory<uint8_t>& byte_buffer_factory,
                       core::BufferFactory<audio::sample_t>& sample_buffer_factory,
                       core::IAllocator& allocator)
    : RefCounted(allocator)
    , config_(config)
    , fanout_(fanout)
    , session_(config,
               format_map,
               packet_factory,
               byte_buffer_factory,
               sample_buffer_factory,
               allocator) {
}

SenderEndpoint* SenderSlot::create_endpoint(address::Interface iface,
                                            address::Protocol proto) {
    roc_log(LogDebug, "sender slot: adding %s endpoint %s",
            address::interface_to_str(iface), address::proto_to_str(proto));

    SenderEndpoint* endpoint = NULL;

    switch (iface) {
    case address::Iface_AudioSource:
        if (!(endpoint = create_source_endpoint_(proto))) {
            return NULL;
        }
        break;

    case address::Iface_AudioRepair:
        if (!(endpoint = create_repair_endpoint_(proto))) {
            return NULL;
        }
        break;

    case address::Iface_AudioControl:
        if (!(endpoint = create_control_endpoint_(proto))) {
            return NULL;
        }
        break;

    default:
        roc_log(LogError, "sender slot: unsupported interface");
        return NULL;
    }

    switch (iface) {
    case address::Iface_AudioSource:
    case address::Iface_AudioRepair:
        if (source_endpoint_
            && (repair_endpoint_ || config_.fec_encoder.scheme == packet::FEC_None)) {
            if (!session_.create_transport_pipeline(source_endpoint_.get(),
                                                    repair_endpoint_.get())) {
                return NULL;
            }
        }
        if (session_.writer()) {
            if (!fanout_.has_output(*session_.writer())) {
                fanout_.add_output(*session_.writer());
            }
        }
        break;

    case address::Iface_AudioControl:
        if (control_endpoint_) {
            if (!session_.create_control_pipeline(control_endpoint_.get())) {
                return NULL;
            }
        }
        break;

    default:
        break;
    }

    return endpoint;
}

audio::IFrameWriter* SenderSlot::writer() {
    return session_.writer();
}

bool SenderSlot::is_ready() const {
    return session_.writer() && source_endpoint_->has_destination_writer()
        && (!repair_endpoint_ || repair_endpoint_->has_destination_writer());
}

core::nanoseconds_t SenderSlot::get_update_deadline() const {
    return session_.get_update_deadline();
}

void SenderSlot::update() {
    session_.update();
}

SenderEndpoint* SenderSlot::create_source_endpoint_(address::Protocol proto) {
    if (source_endpoint_) {
        roc_log(LogError, "sender slot: audio source endpoint is already set");
        return NULL;
    }

    if (!validate_endpoint(address::Iface_AudioSource, proto)) {
        return NULL;
    }

    if (repair_endpoint_) {
        if (!validate_endpoint_pair_consistency(proto, repair_endpoint_->proto())) {
            return NULL;
        }
    }

    if (!validate_endpoint_and_pipeline_consistency(config_.fec_encoder.scheme,
                                                    address::Iface_AudioSource, proto)) {
        return NULL;
    }

    source_endpoint_.reset(new (source_endpoint_) SenderEndpoint(proto, allocator()));
    if (!source_endpoint_ || !source_endpoint_->valid()) {
        roc_log(LogError, "sender slot: can't create source endpoint");
        source_endpoint_.reset(NULL);
        return NULL;
    }

    return source_endpoint_.get();
}

SenderEndpoint* SenderSlot::create_repair_endpoint_(address::Protocol proto) {
    if (repair_endpoint_) {
        roc_log(LogError, "sender slot: audio repair endpoint is already set");
        return NULL;
    }

    if (!validate_endpoint(address::Iface_AudioRepair, proto)) {
        return NULL;
    }

    if (source_endpoint_) {
        if (!validate_endpoint_pair_consistency(source_endpoint_->proto(), proto)) {
            return NULL;
        }
    }

    if (!validate_endpoint_and_pipeline_consistency(config_.fec_encoder.scheme,
                                                    address::Iface_AudioRepair, proto)) {
        return NULL;
    }

    repair_endpoint_.reset(new (repair_endpoint_) SenderEndpoint(proto, allocator()));
    if (!repair_endpoint_ || !repair_endpoint_->valid()) {
        roc_log(LogError, "sender slot: can't create repair endpoint");
        repair_endpoint_.reset(NULL);
        return NULL;
    }

    return repair_endpoint_.get();
}

SenderEndpoint* SenderSlot::create_control_endpoint_(address::Protocol proto) {
    if (control_endpoint_) {
        roc_log(LogError, "sender slot: audio control endpoint is already set");
        return NULL;
    }

    if (!validate_endpoint(address::Iface_AudioControl, proto)) {
        return NULL;
    }

    control_endpoint_.reset(new (control_endpoint_) SenderEndpoint(proto, allocator()));
    if (!control_endpoint_ || !control_endpoint_->valid()) {
        roc_log(LogError, "sender slot: can't create control endpoint");
        control_endpoint_.reset(NULL);
        return NULL;
    }

    return control_endpoint_.get();
}

} // namespace pipeline
} // namespace roc
