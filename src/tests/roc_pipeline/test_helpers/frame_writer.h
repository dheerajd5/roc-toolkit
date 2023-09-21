/*
 * Copyright (c) 2015 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ROC_PIPELINE_TEST_HELPERS_FRAME_WRITER_H_
#define ROC_PIPELINE_TEST_HELPERS_FRAME_WRITER_H_

#include <CppUTest/TestHarness.h>

#include "test_helpers/utils.h"

#include "roc_core/buffer_factory.h"
#include "roc_core/noncopyable.h"
#include "roc_core/slice.h"
#include "roc_sndio/isink.h"

namespace roc {
namespace pipeline {
namespace test {

class FrameWriter : public core::NonCopyable<> {
public:
    FrameWriter(sndio::ISink& sink, core::BufferFactory<audio::sample_t>& buffer_factory)
        : sink_(sink)
        , buffer_factory_(buffer_factory)
        , offset_(0)
        , abs_offset_(0)
        , refresh_ts_(0)
        , next_refresh_ts_(0)
        , last_capture_ts_(0) {
    }

    void write_samples(size_t num_samples,
                       const audio::SampleSpec& sample_spec,
                       core::nanoseconds_t base_capture_ts = -1) {
        core::Slice<audio::sample_t> samples = buffer_factory_.new_buffer();
        CHECK(samples);
        samples.reslice(0, num_samples * sample_spec.num_channels());

        for (size_t ns = 0; ns < num_samples; ns++) {
            for (size_t nc = 0; nc < sample_spec.num_channels(); nc++) {
                samples.data()[ns * sample_spec.num_channels() + nc] =
                    nth_sample(offset_);
            }
            offset_++;
        }

        audio::Frame frame(samples.data(), samples.size());

        if (base_capture_ts >= 0) {
            last_capture_ts_ =
                base_capture_ts + sample_spec.samples_per_chan_2_ns(abs_offset_);

            frame.set_capture_timestamp(last_capture_ts_);
        }

        sink_.write(frame);

        abs_offset_ += num_samples;

        refresh_ts_ = next_refresh_ts_;
        next_refresh_ts_ += sample_spec.samples_per_chan_2_ns(num_samples);
    }

    core::nanoseconds_t refresh_ts() const {
        return refresh_ts_;
    }

    core::nanoseconds_t last_capture_ts() const {
        return last_capture_ts_;
    }

private:
    sndio::ISink& sink_;
    core::BufferFactory<audio::sample_t>& buffer_factory_;

    uint8_t offset_;
    size_t abs_offset_;

    core::nanoseconds_t refresh_ts_;
    core::nanoseconds_t next_refresh_ts_;
    core::nanoseconds_t last_capture_ts_;
};

} // namespace test
} // namespace pipeline
} // namespace roc

#endif // ROC_PIPELINE_TEST_HELPERS_FRAME_WRITER_H_
