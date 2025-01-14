/*
 * Copyright (c) 2020 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_pipeline/receiver_session_group.h
//! @brief Receiver session group.

#ifndef ROC_PIPELINE_RECEIVER_SESSION_GROUP_H_
#define ROC_PIPELINE_RECEIVER_SESSION_GROUP_H_

#include "roc_audio/mixer.h"
#include "roc_core/iallocator.h"
#include "roc_core/list.h"
#include "roc_core/noncopyable.h"
#include "roc_pipeline/receiver_session.h"
#include "roc_pipeline/receiver_state.h"
#include "roc_rtcp/composer.h"
#include "roc_rtcp/session.h"

namespace roc {
namespace pipeline {

//! Receiver session group.
//!
//! Contains:
//!  - a set of related receiver sessions
class ReceiverSessionGroup : public core::NonCopyable<>, private rtcp::IReceiverHooks {
public:
    //! Initialize.
    ReceiverSessionGroup(const ReceiverConfig& receiver_config,
                         ReceiverState& receiver_state,
                         audio::Mixer& mixer,
                         const rtp::FormatMap& format_map,
                         packet::PacketFactory& packet_factory,
                         core::BufferFactory<uint8_t>& byte_buffer_factory,
                         core::BufferFactory<audio::sample_t>& sample_buffer_factory,
                         core::IAllocator& allocator);

    //! Route packet to session.
    void route_packet(const packet::PacketPtr& packet);

    //! Advance session timestamp.
    void advance_sessions(packet::timestamp_t timestamp);

    //! Adjust session clock to match consumer clock.
    void reclock_sessions(packet::ntp_timestamp_t timestamp);

    //! Get number of alive sessions.
    size_t num_sessions() const;

private:
    // Implementation of rtcp::IReceiverHooks interface.
    // These methods are invoked by rtcp::Session.
    virtual void on_update_source(packet::source_t ssrc, const char* cname);
    virtual void on_remove_source(packet::source_t ssrc);
    virtual size_t on_get_num_sources();
    virtual rtcp::ReceptionMetrics on_get_reception_metrics(size_t source_index);
    virtual void on_add_sending_metrics(const rtcp::SendingMetrics& metrics);
    virtual void on_add_link_metrics(const rtcp::LinkMetrics& metrics);

    void route_transport_packet_(const packet::PacketPtr& packet);
    void route_control_packet_(const packet::PacketPtr& packet);

    bool can_create_session_(const packet::PacketPtr& packet);

    void create_session_(const packet::PacketPtr& packet);
    void remove_session_(ReceiverSession& sess);

    ReceiverSessionConfig make_session_config_(const packet::PacketPtr& packet) const;

    core::IAllocator& allocator_;

    packet::PacketFactory& packet_factory_;
    core::BufferFactory<uint8_t>& byte_buffer_factory_;
    core::BufferFactory<audio::sample_t>& sample_buffer_factory_;

    const rtp::FormatMap& format_map_;

    audio::Mixer& mixer_;

    ReceiverState& receiver_state_;
    const ReceiverConfig& receiver_config_;

    core::Optional<rtcp::Composer> rtcp_composer_;
    core::Optional<rtcp::Session> rtcp_session_;

    core::List<ReceiverSession> sessions_;
};

} // namespace pipeline
} // namespace roc

#endif // ROC_PIPELINE_RECEIVER_SESSION_GROUP_H_
