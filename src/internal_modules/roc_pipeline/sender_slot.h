/*
 * Copyright (c) 2020 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_pipeline/sender_slot.h
//! @brief Sender slot.

#ifndef ROC_PIPELINE_SENDER_SLOT_H_
#define ROC_PIPELINE_SENDER_SLOT_H_

#include "roc_address/interface.h"
#include "roc_address/protocol.h"
#include "roc_audio/fanout.h"
#include "roc_core/buffer_factory.h"
#include "roc_core/iallocator.h"
#include "roc_core/noncopyable.h"
#include "roc_core/optional.h"
#include "roc_core/ref_counted.h"
#include "roc_packet/packet_factory.h"
#include "roc_pipeline/config.h"
#include "roc_pipeline/sender_endpoint.h"
#include "roc_pipeline/sender_session.h"

namespace roc {
namespace pipeline {

//! Sender slot.
//!
//! Contains:
//!  - one or more related sender endpoints, one per each type
//!  - one session associated with those endpoints
class SenderSlot : public core::RefCounted<SenderSlot, core::StandardAllocation>,
                   public core::ListNode {
    typedef core::RefCounted<SenderSlot, core::StandardAllocation> RefCounted;

public:
    //! Initialize.
    SenderSlot(const SenderConfig& config,
               const rtp::FormatMap& format_map,
               audio::Fanout& fanout,
               packet::PacketFactory& packet_factory,
               core::BufferFactory<uint8_t>& byte_buffer_factory,
               core::BufferFactory<audio::sample_t>& sample_buffer_factory,
               core::IAllocator& allocator);

    //! Add endpoint.
    SenderEndpoint* create_endpoint(address::Interface iface, address::Protocol proto);

    //! Get audio writer.
    //! @returns NULL if slot is not ready.
    audio::IFrameWriter* writer();

    //! Check if slot configuration is done.
    bool is_ready() const;

    //! Get deadline when the pipeline should be updated.
    core::nanoseconds_t get_update_deadline() const;

    //! Update pipeline.
    void update();

private:
    SenderEndpoint* create_source_endpoint_(address::Protocol proto);
    SenderEndpoint* create_repair_endpoint_(address::Protocol proto);
    SenderEndpoint* create_control_endpoint_(address::Protocol proto);

    const SenderConfig& config_;

    audio::Fanout& fanout_;

    core::Optional<SenderEndpoint> source_endpoint_;
    core::Optional<SenderEndpoint> repair_endpoint_;
    core::Optional<SenderEndpoint> control_endpoint_;

    SenderSession session_;
};

} // namespace pipeline
} // namespace roc

#endif // ROC_PIPELINE_SENDER_SLOT_H_
