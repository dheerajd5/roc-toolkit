/*
 * Copyright (c) 2019 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_audio/iresampler.h
//! @brief Audio resampler interface.

#ifndef ROC_AUDIO_IRESAMPLER_H_
#define ROC_AUDIO_IRESAMPLER_H_

#include "roc_audio/frame.h"
#include "roc_core/noncopyable.h"
#include "roc_core/slice.h"

namespace roc {
namespace audio {

//! Audio writer interface.
class IResampler {
public:
    virtual ~IResampler();

    //! Check if object is successfully constructed.
    virtual bool is_valid() const = 0;

    //! Set new resample factor.
    //! @remarks
    //!  Returns false if the scaling is invalid or out of bounds.
    virtual bool set_scaling(size_t input_rate, size_t output_rate, float multiplier) = 0;

    //! Get buffer to be filled with input data.
    //! @remarks
    //!  After this call, the caller should fill returned buffer with input
    //!  data and invoke end_push_input().
    virtual const core::Slice<sample_t>& begin_push_input() = 0;

    //! Commit buffer with input data.
    //! @remarks
    //!  Should be called after begin_push_input() to commit the push operation.
    virtual void end_push_input() = 0;

    //! Read samples from input buffer and fill output frame.
    //! @remarks
    //!  May return lesser samples than requested if there are no more samples in
    //!  the input ring buffer. In this case the caller should provide resampler
    //!  with more input samples using begin_push_input() and end_push_input().
    virtual size_t pop_output(Frame& out) = 0;

    //! How many samples were pushed but not processed yet.
    //! @remarks
    //!  It is float, as a resampler backend could possibly keep track of current
    //!  position from output stream perspective.
    virtual float n_left_to_process() const = 0;
};

} // namespace audio
} // namespace roc

#endif // ROC_AUDIO_IRESAMPLER_H_
