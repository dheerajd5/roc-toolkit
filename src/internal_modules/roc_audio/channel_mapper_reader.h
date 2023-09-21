/*
 * Copyright (c) 2022 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_audio/channel_mapper_reader.h
//! @brief Channel mapper reader.

#ifndef ROC_AUDIO_CHANNEL_MAPPER_READER_H_
#define ROC_AUDIO_CHANNEL_MAPPER_READER_H_

#include "roc_audio/channel_mapper.h"
#include "roc_audio/iframe_reader.h"
#include "roc_audio/sample_spec.h"
#include "roc_core/buffer_factory.h"
#include "roc_core/noncopyable.h"
#include "roc_core/slice.h"
#include "roc_core/stddefs.h"

namespace roc {
namespace audio {

//! Channel mapper reader.
//! Reads frames from nested reader and maps them to another channel mask.
class ChannelMapperReader : public IFrameReader, public core::NonCopyable<> {
public:
    //! Initialize.
    ChannelMapperReader(IFrameReader& reader,
                        core::BufferFactory<sample_t>& buffer_factory,
                        const SampleSpec& in_spec,
                        const SampleSpec& out_spec);

    //! Check if the object was succefully constructed.
    bool is_valid() const;

    //! Read audio frame.
    virtual bool read(Frame& frame);

private:
    bool read_(sample_t* out_samples,
               size_t n_samples,
               unsigned& flags,
               core::nanoseconds_t& capt_ts);

    IFrameReader& input_reader_;
    core::Slice<sample_t> input_buf_;

    ChannelMapper mapper_;

    const SampleSpec in_spec_;
    const SampleSpec out_spec_;

    bool valid_;
};

} // namespace audio
} // namespace roc

#endif // ROC_AUDIO_CHANNEL_MAPPER_READER_H_
