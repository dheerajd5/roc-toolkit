/*
 * Copyright (c) 2020 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_pipeline/transcoder_source.h"
#include "roc_audio/resampler_map.h"
#include "roc_core/log.h"
#include "roc_core/panic.h"

namespace roc {
namespace pipeline {

TranscoderSource::TranscoderSource(const TranscoderConfig& config,
                                   sndio::ISource& input_source,
                                   core::BufferFactory<audio::sample_t>& buffer_factory,
                                   core::IArena& arena)
    : input_source_(input_source)
    , audio_reader_(NULL)
    , config_(config) {
    audio::IFrameReader* areader = &input_source_;

    if (config.input_sample_spec.channel_set()
        != config.output_sample_spec.channel_set()) {
        channel_mapper_reader_.reset(
            new (channel_mapper_reader_) audio::ChannelMapperReader(
                *areader, buffer_factory, config.input_sample_spec,
                audio::SampleSpec(config.input_sample_spec.sample_rate(),
                                  config.output_sample_spec.channel_set())));
        if (!channel_mapper_reader_ || !channel_mapper_reader_->is_valid()) {
            return;
        }
        areader = channel_mapper_reader_.get();
    }

    if (config.input_sample_spec.sample_rate()
        != config.output_sample_spec.sample_rate()) {
        resampler_poisoner_.reset(new (resampler_poisoner_)
                                      audio::PoisonReader(*areader));
        if (!resampler_poisoner_) {
            return;
        }
        areader = resampler_poisoner_.get();

        resampler_.reset(audio::ResamplerMap::instance().new_resampler(
                             config.resampler_backend, arena, buffer_factory,
                             config.resampler_profile,
                             audio::SampleSpec(config.input_sample_spec.sample_rate(),
                                               config.output_sample_spec.channel_set()),
                             config.output_sample_spec),
                         arena);

        if (!resampler_) {
            return;
        }

        resampler_reader_.reset(new (resampler_reader_) audio::ResamplerReader(
            *areader, *resampler_,
            audio::SampleSpec(config.input_sample_spec.sample_rate(),
                              config.output_sample_spec.channel_set()),
            config.output_sample_spec));

        if (!resampler_reader_ || !resampler_reader_->is_valid()) {
            return;
        }
        areader = resampler_reader_.get();
    }

    pipeline_poisoner_.reset(new (pipeline_poisoner_) audio::PoisonReader(*areader));
    if (!pipeline_poisoner_) {
        return;
    }
    areader = pipeline_poisoner_.get();

    if (config.enable_profiling) {
        profiler_.reset(new (profiler_) audio::ProfilingReader(
            *areader, arena, config.output_sample_spec, config.profiler_config));
        if (!profiler_ || !profiler_->is_valid()) {
            return;
        }
        areader = profiler_.get();
    }

    audio_reader_ = areader;
}

bool TranscoderSource::is_valid() {
    return audio_reader_;
}

sndio::DeviceType TranscoderSource::type() const {
    return input_source_.type();
}

sndio::DeviceState TranscoderSource::state() const {
    return input_source_.state();
}

void TranscoderSource::pause() {
    input_source_.pause();
}

bool TranscoderSource::resume() {
    return input_source_.resume();
}

bool TranscoderSource::restart() {
    return input_source_.restart();
}

audio::SampleSpec TranscoderSource::sample_spec() const {
    return config_.output_sample_spec;
}

core::nanoseconds_t TranscoderSource::latency() const {
    return 0;
}

bool TranscoderSource::has_clock() const {
    return input_source_.has_clock();
}

void TranscoderSource::reclock(core::nanoseconds_t timestamp) {
    input_source_.reclock(timestamp);
}

bool TranscoderSource::read(audio::Frame& frame) {
    roc_panic_if(!is_valid());

    return audio_reader_->read(frame);
}

} // namespace pipeline
} // namespace roc
