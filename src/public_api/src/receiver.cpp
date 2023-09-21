/*
 * Copyright (c) 2017 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc/receiver.h"

#include "adapters.h"

#include "roc_core/log.h"
#include "roc_core/scoped_ptr.h"
#include "roc_node/receiver.h"

using namespace roc;

int roc_receiver_open(roc_context* context,
                      const roc_receiver_config* config,
                      roc_receiver** result) {
    roc_log(LogInfo, "roc_receiver_open(): opening receiver");

    if (!result) {
        roc_log(LogError, "roc_receiver_open(): invalid arguments: result is null");
        return -1;
    }

    if (!context) {
        roc_log(LogError, "roc_receiver_open(): invalid arguments: context is null");
        return -1;
    }

    node::Context* imp_context = (node::Context*)context;

    if (!config) {
        roc_log(LogError, "roc_receiver_open(): invalid arguments: config is null");
        return -1;
    }

    pipeline::ReceiverConfig imp_config;
    if (!api::receiver_config_from_user(*imp_context, imp_config, *config)) {
        roc_log(LogError, "roc_receiver_open(): invalid arguments: bad config");
        return -1;
    }

    core::ScopedPtr<node::Receiver> imp_receiver(
        new (imp_context->arena()) node::Receiver(*imp_context, imp_config),
        imp_context->arena());

    if (!imp_receiver) {
        roc_log(LogError, "roc_receiver_open(): can't allocate receiver");
        return -1;
    }

    if (!imp_receiver->is_valid()) {
        roc_log(LogError, "roc_receiver_open(): can't initialize receiver");
        return -1;
    }

    *result = (roc_receiver*)imp_receiver.release();
    return 0;
}

int roc_receiver_configure(roc_receiver* receiver,
                           roc_slot slot,
                           roc_interface iface,
                           const roc_interface_config* config) {
    if (!receiver) {
        roc_log(LogError,
                "roc_receiver_configure(): invalid arguments: receiver is null");
        return -1;
    }

    node::Receiver* imp_receiver = (node::Receiver*)receiver;

    address::Interface imp_iface;
    if (!api::interface_from_user(imp_iface, iface)) {
        roc_log(LogError, "roc_receiver_configure(): invalid arguments: bad interface");
        return -1;
    }

    if (!config) {
        roc_log(LogError, "roc_receiver_configure(): invalid arguments: config is null");
        return -1;
    }

    netio::UdpReceiverConfig imp_config;
    if (!api::receiver_interface_config_from_user(imp_config, *config)) {
        roc_log(LogError, "roc_receiver_configure(): invalid arguments: bad config");
        return -1;
    }

    if (!imp_receiver->configure(slot, imp_iface, imp_config)) {
        roc_log(LogError, "roc_receiver_configure(): operation failed");
        return -1;
    }

    return 0;
}

int roc_receiver_bind(roc_receiver* receiver,
                      roc_slot slot,
                      roc_interface iface,
                      roc_endpoint* endpoint) {
    if (!receiver) {
        roc_log(LogError, "roc_receiver_bind(): invalid arguments: receiver is null");
        return -1;
    }

    node::Receiver* imp_receiver = (node::Receiver*)receiver;

    if (!endpoint) {
        roc_log(LogError, "roc_receiver_bind(): invalid arguments: endpoint is null");
        return -1;
    }

    address::EndpointUri& imp_endpoint = *(address::EndpointUri*)endpoint;

    address::Interface imp_iface;
    if (!api::interface_from_user(imp_iface, iface)) {
        roc_log(LogError, "roc_receiver_bind(): invalid arguments: bad interface");
        return -1;
    }

    if (!imp_receiver->bind(slot, imp_iface, imp_endpoint)) {
        roc_log(LogError, "roc_receiver_bind(): operation failed");
        return -1;
    }

    return 0;
}

int roc_receiver_unlink(roc_receiver* receiver, roc_slot slot) {
    if (!receiver) {
        roc_log(LogError, "roc_receiver_unlink(): invalid arguments: receiver is null");
        return -1;
    }

    node::Receiver* imp_receiver = (node::Receiver*)receiver;

    if (!imp_receiver->unlink(slot)) {
        roc_log(LogError, "roc_receiver_unlink(): operation failed");
        return -1;
    }

    return 0;
}

int roc_receiver_query(roc_receiver* receiver,
                       roc_slot slot,
                       roc_receiver_metrics* metrics) {
    if (!receiver) {
        roc_log(LogError, "roc_receiver_query(): invalid arguments: receiver is null");
        return -1;
    }

    if (!metrics) {
        roc_log(LogError, "roc_receiver_query(): invalid arguments: metrics are null");
        return -1;
    }

    node::Receiver* imp_receiver = (node::Receiver*)receiver;

    pipeline::ReceiverSlotMetrics slot_metrics;

    if (!imp_receiver->get_metrics(slot, slot_metrics,
                                   api::receiver_session_metrics_to_user,
                                   &metrics->sessions_size, metrics->sessions)) {
        roc_log(LogError, "roc_receiver_query(): operation failed");
        return -1;
    }

    api::receiver_slot_metrics_to_user(*metrics, slot_metrics);

    return 0;
}

int roc_receiver_read(roc_receiver* receiver, roc_frame* frame) {
    if (!receiver) {
        roc_log(LogError, "roc_receiver_read(): invalid arguments: receiver is null");
        return -1;
    }

    node::Receiver* imp_receiver = (node::Receiver*)receiver;

    sndio::ISource& imp_source = imp_receiver->source();

    if (!frame) {
        roc_log(LogError, "roc_receiver_read(): invalid arguments: frame is null");
        return -1;
    }

    if (frame->samples_size == 0) {
        return 0;
    }

    const size_t factor = imp_source.sample_spec().num_channels() * sizeof(float);

    if (frame->samples_size % factor != 0) {
        roc_log(LogError,
                "roc_receiver_read(): invalid arguments:"
                " # of samples should be multiple of %u",
                (unsigned)factor);
        return -1;
    }

    if (!frame->samples) {
        roc_log(LogError, "roc_receiver_read(): invalid arguments: samples is null");
        return -1;
    }

    audio::Frame imp_frame((float*)frame->samples, frame->samples_size / sizeof(float));

    if (!imp_source.read(imp_frame)) {
        roc_log(LogError, "roc_receiver_read(): got unexpected eof from source");
        return -1;
    }

    return 0;
}

int roc_receiver_close(roc_receiver* receiver) {
    if (!receiver) {
        roc_log(LogError, "roc_receiver_close(): invalid arguments: receiver is null");
        return -1;
    }

    node::Receiver* imp_receiver = (node::Receiver*)receiver;
    imp_receiver->context().arena().destroy_object(*imp_receiver);

    roc_log(LogInfo, "roc_receiver_close(): closed receiver");

    return 0;
}
