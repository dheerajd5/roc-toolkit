/*
 * Copyright (c) 2017 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_pipeline/receiver_source.h"
#include "roc_core/log.h"
#include "roc_core/panic.h"
#include "roc_core/shared_ptr.h"
#include "roc_sndio/device_type.h"

namespace roc {
namespace pipeline {

ReceiverSource::ReceiverSource(
    const ReceiverConfig& config,
    const rtp::FormatMap& format_map,
    packet::PacketFactory& packet_factory,
    core::BufferFactory<uint8_t>& byte_buffer_factory,
    core::BufferFactory<audio::sample_t>& sample_buffer_factory,
    core::IArena& arena)
    : format_map_(format_map)
    , packet_factory_(packet_factory)
    , byte_buffer_factory_(byte_buffer_factory)
    , sample_buffer_factory_(sample_buffer_factory)
    , arena_(arena)
    , audio_reader_(NULL)
    , config_(config) {
    mixer_.reset(new (mixer_) audio::Mixer(sample_buffer_factory, true));
    if (!mixer_ || !mixer_->is_valid()) {
        return;
    }
    audio::IFrameReader* areader = mixer_.get();

    poisoner_.reset(new (poisoner_) audio::PoisonReader(*areader));
    if (!poisoner_) {
        return;
    }
    areader = poisoner_.get();

    if (config.common.enable_profiling) {
        profiler_.reset(new (profiler_) audio::ProfilingReader(
            *areader, arena, config.common.output_sample_spec,
            config.common.profiler_config));
        if (!profiler_ || !profiler_->is_valid()) {
            return;
        }
        areader = profiler_.get();
    }

    audio_reader_ = areader;
}

bool ReceiverSource::is_valid() const {
    return audio_reader_;
}

ReceiverSlot* ReceiverSource::create_slot() {
    roc_panic_if(!is_valid());

    roc_log(LogInfo, "receiver source: adding slot");

    core::SharedPtr<ReceiverSlot> slot =
        new (arena_) ReceiverSlot(config_, state_, *mixer_, format_map_, packet_factory_,
                                  byte_buffer_factory_, sample_buffer_factory_, arena_);

    if (!slot) {
        return NULL;
    }

    slots_.push_back(*slot);
    return slot.get();
}

void ReceiverSource::delete_slot(ReceiverSlot* slot) {
    roc_panic_if(!is_valid());

    roc_log(LogInfo, "receiver source: removing slot");

    slots_.remove(*slot);
}

size_t ReceiverSource::num_sessions() const {
    return state_.num_sessions();
}

core::nanoseconds_t ReceiverSource::refresh(core::nanoseconds_t current_time) {
    roc_panic_if(!is_valid());

    core::nanoseconds_t next_deadline = 0;

    for (core::SharedPtr<ReceiverSlot> slot = slots_.front(); slot;
         slot = slots_.nextof(*slot)) {
        const core::nanoseconds_t slot_deadline = slot->refresh(current_time);

        if (slot_deadline != 0) {
            if (next_deadline == 0) {
                next_deadline = slot_deadline;
            } else {
                next_deadline = std::min(next_deadline, slot_deadline);
            }
        }
    }

    return next_deadline;
}

sndio::DeviceType ReceiverSource::type() const {
    return sndio::DeviceType_Source;
}

sndio::DeviceState ReceiverSource::state() const {
    roc_panic_if(!is_valid());

    if (state_.num_sessions() != 0) {
        // we have sessions and they're producing some sound
        return sndio::DeviceState_Active;
    }

    if (state_.has_pending_packets()) {
        // we don't have sessions, but we have packets that may create sessions
        return sndio::DeviceState_Active;
    }

    // no sessions and packets; we can sleep until there are some
    return sndio::DeviceState_Idle;
}

void ReceiverSource::pause() {
    // no-op
}

bool ReceiverSource::resume() {
    return true;
}

bool ReceiverSource::restart() {
    return true;
}

audio::SampleSpec ReceiverSource::sample_spec() const {
    return config_.common.output_sample_spec;
}

core::nanoseconds_t ReceiverSource::latency() const {
    return 0;
}

bool ReceiverSource::has_latency() const {
    return false;
}

bool ReceiverSource::has_clock() const {
    return config_.common.enable_timing;
}

void ReceiverSource::reclock(core::nanoseconds_t playback_time) {
    roc_panic_if(!is_valid());

    for (core::SharedPtr<ReceiverSlot> slot = slots_.front(); slot;
         slot = slots_.nextof(*slot)) {
        slot->reclock(playback_time);
    }
}

bool ReceiverSource::read(audio::Frame& frame) {
    roc_panic_if(!is_valid());

    return audio_reader_->read(frame);
}

} // namespace pipeline
} // namespace roc
