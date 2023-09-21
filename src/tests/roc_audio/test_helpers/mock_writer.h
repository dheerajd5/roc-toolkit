/*
 * Copyright (c) 2020 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ROC_AUDIO_TEST_HELPERS_MOCK_WRITER_H_
#define ROC_AUDIO_TEST_HELPERS_MOCK_WRITER_H_

#include <CppUTest/TestHarness.h>

#include "roc_audio/iframe_writer.h"
#include "roc_core/stddefs.h"

namespace roc {
namespace audio {
namespace test {

class MockWriter : public IFrameWriter {
public:
    MockWriter()
        : pos_(0)
        , size_(0)
        , n_writes_(0) {
    }

    virtual void write(Frame& frame) {
        CHECK(size_ + frame.num_samples() <= MaxSz);

        memcpy(samples_ + size_, frame.samples(), frame.num_samples() * sizeof(sample_t));
        size_ += frame.num_samples();

        CHECK(n_writes_ < MaxWrites);

        frame_sizes_[n_writes_] = frame.num_samples();
        frame_flags_[n_writes_] = frame.flags();
        frame_timestamps_[n_writes_] = frame.capture_timestamp();

        n_writes_++;
    }

    sample_t get() {
        CHECK(pos_ < size_);

        return samples_[pos_++];
    }

    size_t num_unread() const {
        return size_ - pos_;
    }

    size_t n_writes() const {
        return n_writes_;
    }

    size_t frame_size(size_t n) const {
        CHECK(n < n_writes_);
        return frame_sizes_[n];
    }

    unsigned frame_flags(size_t n) const {
        CHECK(n < n_writes_);
        return frame_flags_[n];
    }

    core::nanoseconds_t frame_timestamp(size_t n) const {
        CHECK(n < n_writes_);
        return frame_timestamps_[n];
    }

private:
    enum { MaxWrites = 1000, MaxSz = 64 * 1024 };

    sample_t samples_[MaxSz];
    size_t pos_;
    size_t size_;

    size_t n_writes_;

    size_t frame_sizes_[MaxWrites];
    unsigned frame_flags_[MaxWrites];
    core::nanoseconds_t frame_timestamps_[MaxWrites];
};

} // namespace test
} // namespace audio
} // namespace roc

#endif // ROC_AUDIO_TEST_HELPERS_MOCK_WRITER_H_
