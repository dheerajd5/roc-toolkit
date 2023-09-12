/*
 * Copyright (c) 2019 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <CppUTest/TestHarness.h>

#include "test_helpers/frame_writer.h"
#include "test_helpers/mock_sink.h"

#include "roc_core/buffer_factory.h"
#include "roc_core/heap_arena.h"
#include "roc_pipeline/transcoder_sink.h"

namespace roc {
namespace pipeline {

namespace {

const audio::ChannelMask Chans_Mono = audio::ChanMask_Surround_Mono;
const audio::ChannelMask Chans_Stereo = audio::ChanMask_Surround_Stereo;

enum {
    MaxBufSize = 1000,

    SampleRate = 44100,

    SamplesPerFrame = 20,
    ManyFrames = 30
};

core::HeapArena arena;
core::BufferFactory<audio::sample_t> sample_buffer_factory(arena, MaxBufSize);

} // namespace

TEST_GROUP(transcoder_sink) {
    audio::SampleSpec input_sample_spec;
    audio::SampleSpec output_sample_spec;

    TranscoderConfig make_config() {
        TranscoderConfig config;

        config.input_sample_spec = input_sample_spec;
        config.output_sample_spec = output_sample_spec;

        config.enable_profiling = true;

        return config;
    }

    void init(int input_sample_rate, audio::ChannelMask input_channels,
              int output_sample_rate, audio::ChannelMask output_channels) {
        input_sample_spec.set_sample_rate((size_t)input_sample_rate);
        input_sample_spec.channel_set().set_layout(audio::ChanLayout_Surround);
        input_sample_spec.channel_set().set_channel_mask(input_channels);

        output_sample_spec.set_sample_rate((size_t)output_sample_rate);
        output_sample_spec.channel_set().set_layout(audio::ChanLayout_Surround);
        output_sample_spec.channel_set().set_channel_mask(output_channels);
    }
};

TEST(transcoder_sink, null) {
    enum { Rate = SampleRate, Chans = Chans_Stereo };

    init(Rate, Chans, Rate, Chans);

    TranscoderSink transcoder(make_config(), NULL, sample_buffer_factory, arena);
    CHECK(transcoder.is_valid());

    test::FrameWriter frame_writer(transcoder, sample_buffer_factory);

    for (size_t nf = 0; nf < ManyFrames; nf++) {
        frame_writer.write_samples(SamplesPerFrame, input_sample_spec);
    }
}

TEST(transcoder_sink, write) {
    enum { Rate = SampleRate, Chans = Chans_Stereo };

    init(Rate, Chans, Rate, Chans);

    test::MockSink mock_sink(output_sample_spec);

    TranscoderSink transcoder(make_config(), &mock_sink, sample_buffer_factory, arena);
    CHECK(transcoder.is_valid());

    test::FrameWriter frame_writer(transcoder, sample_buffer_factory);

    for (size_t nf = 0; nf < ManyFrames; nf++) {
        frame_writer.write_samples(SamplesPerFrame, input_sample_spec);
    }

    mock_sink.expect_frames(ManyFrames);
    mock_sink.expect_samples(ManyFrames * SamplesPerFrame);
}

TEST(transcoder_sink, frame_size_small) {
    enum {
        Rate = SampleRate,
        Chans = Chans_Stereo,
        SamplesPerSmallFrame = SamplesPerFrame / 2 - 3
    };

    init(Rate, Chans, Rate, Chans);

    test::MockSink mock_sink(output_sample_spec);

    TranscoderSink transcoder(make_config(), &mock_sink, sample_buffer_factory, arena);
    CHECK(transcoder.is_valid());

    test::FrameWriter frame_writer(transcoder, sample_buffer_factory);

    for (size_t nf = 0; nf < ManyFrames; nf++) {
        frame_writer.write_samples(SamplesPerSmallFrame, input_sample_spec);
    }

    mock_sink.expect_frames(ManyFrames);
    mock_sink.expect_samples(ManyFrames * SamplesPerSmallFrame);
}

TEST(transcoder_sink, frame_size_large) {
    enum {
        Rate = SampleRate,
        Chans = Chans_Stereo,
        SamplesPerLargeFrame = SamplesPerFrame * 2 + 3
    };

    init(Rate, Chans, Rate, Chans);

    test::MockSink mock_sink(output_sample_spec);

    TranscoderSink transcoder(make_config(), &mock_sink, sample_buffer_factory, arena);
    CHECK(transcoder.is_valid());

    test::FrameWriter frame_writer(transcoder, sample_buffer_factory);

    for (size_t nf = 0; nf < ManyFrames; nf++) {
        frame_writer.write_samples(SamplesPerLargeFrame, input_sample_spec);
    }

    mock_sink.expect_frames(ManyFrames);
    mock_sink.expect_samples(ManyFrames * SamplesPerLargeFrame);
}

TEST(transcoder_sink, channel_mapping_stereo_to_mono) {
    enum { Rate = SampleRate, InputChans = Chans_Stereo, OutputChans = Chans_Mono };

    init(Rate, InputChans, Rate, OutputChans);

    test::MockSink mock_sink(output_sample_spec);

    TranscoderSink transcoder(make_config(), &mock_sink, sample_buffer_factory, arena);
    CHECK(transcoder.is_valid());

    test::FrameWriter frame_writer(transcoder, sample_buffer_factory);

    for (size_t nf = 0; nf < ManyFrames; nf++) {
        frame_writer.write_samples(SamplesPerFrame, input_sample_spec);
    }

    mock_sink.expect_frames(ManyFrames);
    mock_sink.expect_samples(ManyFrames * SamplesPerFrame);
}

TEST(transcoder_sink, channel_mapping_mono_to_stereo) {
    enum { Rate = SampleRate, InputChans = Chans_Mono, OutputChans = Chans_Stereo };

    init(Rate, InputChans, Rate, OutputChans);

    test::MockSink mock_sink(output_sample_spec);

    TranscoderSink transcoder(make_config(), &mock_sink, sample_buffer_factory, arena);
    CHECK(transcoder.is_valid());

    test::FrameWriter frame_writer(transcoder, sample_buffer_factory);

    for (size_t nf = 0; nf < ManyFrames; nf++) {
        frame_writer.write_samples(SamplesPerFrame, input_sample_spec);
    }

    mock_sink.expect_frames(ManyFrames);
    mock_sink.expect_samples(ManyFrames * SamplesPerFrame);
}

} // namespace pipeline
} // namespace roc
