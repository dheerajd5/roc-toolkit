/*
 * Copyright (c) 2023 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <CppUTest/TestHarness.h>

#include "test_helpers/mock_reader.h"

#include "roc_audio/channel_layout.h"
#include "roc_audio/channel_mapper_reader.h"
#include "roc_core/buffer_factory.h"
#include "roc_core/heap_arena.h"
#include "roc_core/stddefs.h"
#include "roc_core/time.h"

namespace roc {
namespace audio {

namespace {

const double Epsilon = 0.00001;

enum { MaxSz = 500 };

core::HeapArena arena;
core::BufferFactory<sample_t> buffer_factory(arena, MaxSz);

void add_mono(test::MockReader& mock_reader,
              size_t size,
              sample_t value,
              unsigned flags) {
    CHECK(size > 0);

    for (size_t n = 0; n < size; n++) {
        mock_reader.add_samples(1, value, flags);
    }
}

void add_stereo(test::MockReader& mock_reader,
                size_t size,
                sample_t left_value,
                sample_t right_value,
                unsigned flags) {
    CHECK(size > 0);
    CHECK(size % 2 == 0);

    for (size_t n = 0; n < size; n += 2) {
        mock_reader.add_samples(1, left_value, flags);
        mock_reader.add_samples(1, right_value, flags);
    }
}

void expect_mono(const Frame& frame, sample_t value) {
    CHECK(frame.num_samples() > 0);

    for (size_t n = 0; n < frame.num_samples(); n++) {
        DOUBLES_EQUAL((double)value, (double)frame.samples()[n], Epsilon);
    }
}

void expect_stereo(const Frame& frame, sample_t left_value, sample_t right_value) {
    CHECK(frame.num_samples() > 0);
    CHECK(frame.num_samples() % 2 == 0);

    for (size_t n = 0; n < frame.num_samples(); n += 2) {
        DOUBLES_EQUAL((double)left_value, (double)frame.samples()[n + 0], Epsilon);
        DOUBLES_EQUAL((double)right_value, (double)frame.samples()[n + 1], Epsilon);
    }
}

} // namespace

TEST_GROUP(channel_mapper_reader) {};

TEST(channel_mapper_reader, small_frame_upmix) {
    enum { FrameSz = MaxSz / 2 };

    const SampleSpec in_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Mono);
    const SampleSpec out_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Stereo);

    const core::nanoseconds_t start_ts = 1000000;

    test::MockReader mock_reader;
    ChannelMapperReader mapper_reader(mock_reader, buffer_factory, in_spec, out_spec);

    const unsigned flags = Frame::FlagIncomplete;

    mock_reader.enable_timestamps(start_ts, in_spec);
    add_mono(mock_reader, FrameSz / 2, 0.3f, flags);

    sample_t samples[FrameSz] = {};
    Frame frame(samples, FrameSz);

    CHECK(mapper_reader.read(frame));

    CHECK_EQUAL(1, mock_reader.total_reads());
    CHECK_EQUAL(0, mock_reader.num_unread());

    CHECK_EQUAL(flags, frame.flags());
    CHECK_EQUAL(start_ts, frame.capture_timestamp());

    expect_stereo(frame, 0.3f, 0.3f);
}

TEST(channel_mapper_reader, small_frame_downmix) {
    enum { FrameSz = MaxSz / 2 };

    const SampleSpec in_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Stereo);
    const SampleSpec out_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Mono);

    const core::nanoseconds_t start_ts = 1000000;

    test::MockReader mock_reader;
    ChannelMapperReader mapper_reader(mock_reader, buffer_factory, in_spec, out_spec);

    const unsigned flags = Frame::FlagIncomplete;

    mock_reader.enable_timestamps(start_ts, in_spec);
    add_stereo(mock_reader, FrameSz * 2, 0.2f, 0.4f, flags);

    sample_t samples[FrameSz] = {};
    Frame frame(samples, FrameSz);

    CHECK(mapper_reader.read(frame));

    CHECK_EQUAL(1, mock_reader.total_reads());
    CHECK_EQUAL(0, mock_reader.num_unread());

    CHECK_EQUAL(flags, frame.flags());
    CHECK_EQUAL(start_ts, frame.capture_timestamp());

    expect_mono(frame, 0.3f);
}

TEST(channel_mapper_reader, small_frame_nocts) {
    enum { FrameSz = MaxSz / 2 };

    const SampleSpec in_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Stereo);
    const SampleSpec out_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Mono);

    test::MockReader mock_reader;
    ChannelMapperReader mapper_reader(mock_reader, buffer_factory, in_spec, out_spec);

    const unsigned flags = Frame::FlagIncomplete;

    add_stereo(mock_reader, FrameSz * 2, 0.2f, 0.4f, flags);

    sample_t samples[FrameSz] = {};
    Frame frame(samples, FrameSz);

    CHECK(mapper_reader.read(frame));

    CHECK_EQUAL(1, mock_reader.total_reads());
    CHECK_EQUAL(0, mock_reader.num_unread());

    CHECK_EQUAL(flags, frame.flags());
    CHECK_EQUAL(0, frame.capture_timestamp());

    expect_mono(frame, 0.3f);
}

TEST(channel_mapper_reader, large_frame_upmix) {
    enum { FrameSz = MaxSz * 4 };

    const SampleSpec in_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Mono);
    const SampleSpec out_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Stereo);

    const core::nanoseconds_t start_ts = 1000000;

    test::MockReader mock_reader;
    ChannelMapperReader mapper_reader(mock_reader, buffer_factory, in_spec, out_spec);

    const unsigned flags1 = Frame::FlagIncomplete;
    const unsigned flags2 = Frame::FlagDrops;

    mock_reader.enable_timestamps(start_ts, in_spec);
    add_mono(mock_reader, MaxSz, 0.3f, flags1);
    add_mono(mock_reader, MaxSz, 0.3f, flags2);

    sample_t samples[FrameSz] = {};
    Frame frame(samples, FrameSz);

    CHECK(mapper_reader.read(frame));

    CHECK_EQUAL(2, mock_reader.total_reads());
    CHECK_EQUAL(0, mock_reader.num_unread());

    CHECK_EQUAL(flags1 | flags2, frame.flags());
    CHECK_EQUAL(start_ts, frame.capture_timestamp());

    expect_stereo(frame, 0.3f, 0.3f);
}

TEST(channel_mapper_reader, large_frame_downmix) {
    enum { FrameSz = MaxSz };

    const SampleSpec in_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Stereo);
    const SampleSpec out_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Mono);

    const core::nanoseconds_t start_ts = 1000000;

    test::MockReader mock_reader;
    ChannelMapperReader mapper_reader(mock_reader, buffer_factory, in_spec, out_spec);

    const unsigned flags1 = Frame::FlagIncomplete;
    const unsigned flags2 = Frame::FlagDrops;

    mock_reader.enable_timestamps(start_ts, in_spec);
    add_stereo(mock_reader, MaxSz, 0.2f, 0.4f, flags1);
    add_stereo(mock_reader, MaxSz, 0.2f, 0.4f, flags2);

    sample_t samples[FrameSz] = {};
    Frame frame(samples, FrameSz);

    CHECK(mapper_reader.read(frame));

    CHECK_EQUAL(2, mock_reader.total_reads());
    CHECK_EQUAL(0, mock_reader.num_unread());

    CHECK_EQUAL(flags1 | flags2, frame.flags());
    CHECK_EQUAL(start_ts, frame.capture_timestamp());

    expect_mono(frame, 0.3f);
}

TEST(channel_mapper_reader, large_frame_nocts) {
    enum { FrameSz = MaxSz };

    const SampleSpec in_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Stereo);
    const SampleSpec out_spec(MaxSz, ChanLayout_Surround, ChanMask_Surround_Mono);

    test::MockReader mock_reader;
    ChannelMapperReader mapper_reader(mock_reader, buffer_factory, in_spec, out_spec);

    const unsigned flags1 = Frame::FlagIncomplete;
    const unsigned flags2 = Frame::FlagDrops;

    add_stereo(mock_reader, MaxSz, 0.2f, 0.4f, flags1);
    add_stereo(mock_reader, MaxSz, 0.2f, 0.4f, flags2);

    sample_t samples[FrameSz] = {};
    Frame frame(samples, FrameSz);

    CHECK(mapper_reader.read(frame));

    CHECK_EQUAL(2, mock_reader.total_reads());
    CHECK_EQUAL(0, mock_reader.num_unread());

    CHECK_EQUAL(flags1 | flags2, frame.flags());
    CHECK_EQUAL(0, frame.capture_timestamp());

    expect_mono(frame, 0.3f);
}

} // namespace audio
} // namespace roc
