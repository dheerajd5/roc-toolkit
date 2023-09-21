/*
 * Copyright (c) 2019 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ROC_PIPELINE_TEST_HELPERS_MOCK_SINK_H_
#define ROC_PIPELINE_TEST_HELPERS_MOCK_SINK_H_

#include <CppUTest/TestHarness.h>

#include "test_helpers/utils.h"

#include "roc_core/noncopyable.h"
#include "roc_sndio/isink.h"

namespace roc {
namespace pipeline {
namespace test {

class MockSink : public sndio::ISink, public core::NonCopyable<> {
public:
    MockSink(const audio::SampleSpec& sample_spec)
        : off_(0)
        , n_frames_(0)
        , n_samples_(0)
        , n_chans_(sample_spec.num_channels()) {
    }

    virtual sndio::DeviceType type() const {
        return sndio::DeviceType_Sink;
    }

    virtual sndio::DeviceState state() const {
        return sndio::DeviceState_Active;
    }

    virtual void pause() {
        FAIL("not implemented");
    }

    virtual bool resume() {
        FAIL("not implemented");
        return false;
    }

    virtual bool restart() {
        FAIL("not implemented");
        return false;
    }

    virtual audio::SampleSpec sample_spec() const {
        return audio::SampleSpec();
    }

    virtual core::nanoseconds_t latency() const {
        return 0;
    }

    virtual bool has_latency() const {
        return false;
    }

    virtual bool has_clock() const {
        return false;
    }

    virtual void write(audio::Frame& frame) {
        CHECK(frame.num_samples() % n_chans_ == 0);

        for (size_t ns = 0; ns < frame.num_samples() / n_chans_; ns++) {
            for (size_t nc = 0; nc < n_chans_; nc++) {
                DOUBLES_EQUAL((double)frame.samples()[ns * n_chans_ + nc],
                              (double)nth_sample(off_), SampleEpsilon);
                n_samples_++;
            }
            off_++;
        }
        n_frames_++;

        CHECK(frame.capture_timestamp() == 0);
    }

    void expect_frames(size_t total) {
        UNSIGNED_LONGS_EQUAL(total, n_frames_);
    }

    void expect_samples(size_t total) {
        UNSIGNED_LONGS_EQUAL(total * n_chans_, n_samples_);
    }

private:
    uint8_t off_;

    size_t n_frames_;
    size_t n_samples_;
    size_t n_chans_;
};

} // namespace test
} // namespace pipeline
} // namespace roc

#endif // ROC_PIPELINE_TEST_HELPERS_MOCK_SINK_H_
