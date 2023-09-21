/*
 * Copyright (c) 2015 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ROC_SNDIO_TARGET_SOX_TEST_HELPERS_MOCK_SINK_H_
#define ROC_SNDIO_TARGET_SOX_TEST_HELPERS_MOCK_SINK_H_

#include <CppUTest/TestHarness.h>

#include "roc_sndio/isink.h"

namespace roc {
namespace sndio {
namespace test {

class MockSink : public ISink {
public:
    MockSink()
        : pos_(0) {
    }

    virtual DeviceType type() const {
        return DeviceType_Sink;
    }

    virtual DeviceState state() const {
        return DeviceState_Active;
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
        CHECK(pos_ + frame.num_samples() <= MaxSz);

        memcpy(samples_ + pos_, frame.samples(),
               frame.num_samples() * sizeof(audio::sample_t));
        pos_ += frame.num_samples();
    }

    void check(size_t offset, size_t size) {
        UNSIGNED_LONGS_EQUAL(pos_, size);

        for (size_t n = 0; n < size; n++) {
            DOUBLES_EQUAL((double)samples_[n], (double)nth_sample_(offset + n), 0.0001);
        }
    }

private:
    enum { MaxSz = 256 * 1024 };

    audio::sample_t nth_sample_(size_t n) {
        return audio::sample_t(uint8_t(n)) / audio::sample_t(1 << 8);
    }

    audio::sample_t samples_[MaxSz];
    size_t pos_;
};

} // namespace test
} // namespace sndio
} // namespace roc

#endif // ROC_SNDIO_TARGET_SOX_TEST_HELPERS_MOCK_SINK_H_
