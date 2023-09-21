/*
 * Copyright (c) 2017 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_pipeline/sender_sink.h
//! @brief Sender sink pipeline.

#ifndef ROC_PIPELINE_SENDER_SINK_H_
#define ROC_PIPELINE_SENDER_SINK_H_

#include "roc_audio/fanout.h"
#include "roc_audio/iframe_encoder.h"
#include "roc_audio/iresampler.h"
#include "roc_audio/packetizer.h"
#include "roc_audio/profiling_writer.h"
#include "roc_core/buffer_factory.h"
#include "roc_core/iarena.h"
#include "roc_core/noncopyable.h"
#include "roc_core/optional.h"
#include "roc_fec/iblock_encoder.h"
#include "roc_fec/writer.h"
#include "roc_packet/interleaver.h"
#include "roc_packet/packet_factory.h"
#include "roc_packet/router.h"
#include "roc_pipeline/config.h"
#include "roc_pipeline/sender_endpoint.h"
#include "roc_pipeline/sender_slot.h"
#include "roc_rtp/format_map.h"
#include "roc_sndio/isink.h"

namespace roc {
namespace pipeline {

//! Sender sink pipeline.
//!
//! Contains:
//!  - one or more sender slots
//!  - fanout, to duplicate audio to all slots
//!
//! Pipeline:
//!  - input: frames
//!  - output: packets
class SenderSink : public sndio::ISink, public core::NonCopyable<> {
public:
    //! Initialize.
    SenderSink(const SenderConfig& config,
               const rtp::FormatMap& format_map,
               packet::PacketFactory& packet_factory,
               core::BufferFactory<uint8_t>& byte_buffer_factory,
               core::BufferFactory<audio::sample_t>& sample_buffer_factory,
               core::IArena& arena);

    //! Check if the pipeline was successfully constructed.
    bool is_valid() const;

    //! Create slot.
    SenderSlot* create_slot();

    //! Delete slot.
    void delete_slot(SenderSlot* slot);

    //! Refresh pipeline according to current time.
    //! @remarks
    //!  Should be invoked after writing each frame.
    //!  Also should be invoked after provided deadline if no frames were
    //!  written until that deadline expires.
    //! @returns
    //!  deadline (absolute time) when refresh should be invoked again
    //!  if there are no frames
    core::nanoseconds_t refresh(core::nanoseconds_t current_time);

    //! Get device type.
    virtual sndio::DeviceType type() const;

    //! Get current receiver state.
    virtual sndio::DeviceState state() const;

    //! Pause reading.
    virtual void pause();

    //! Resume paused reading.
    virtual bool resume();

    //! Restart reading from the beginning.
    virtual bool restart();

    //! Get sample specification of the sink.
    virtual audio::SampleSpec sample_spec() const;

    //! Get latency of the sink.
    virtual core::nanoseconds_t latency() const;

    //! Check if the sink supports latency reports.
    virtual bool has_latency() const;

    //! Check if the sink has own clock.
    virtual bool has_clock() const;

    //! Write audio frame.
    virtual void write(audio::Frame& frame);

private:
    const SenderConfig config_;

    const rtp::FormatMap& format_map_;

    packet::PacketFactory& packet_factory_;
    core::BufferFactory<uint8_t>& byte_buffer_factory_;
    core::BufferFactory<audio::sample_t>& sample_buffer_factory_;

    core::IArena& arena_;

    audio::Fanout fanout_;

    core::List<SenderSlot> slots_;

    core::Optional<audio::ProfilingWriter> profiler_;

    audio::IFrameWriter* audio_writer_;
};

} // namespace pipeline
} // namespace roc

#endif // ROC_PIPELINE_SENDER_SINK_H_
