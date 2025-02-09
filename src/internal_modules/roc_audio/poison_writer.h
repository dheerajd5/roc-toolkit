/*
 * Copyright (c) 2019 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_audio/poison_writer.h
//! @brief Poison writer.

#ifndef ROC_AUDIO_POISON_WRITER_H_
#define ROC_AUDIO_POISON_WRITER_H_

#include "roc_audio/frame.h"
#include "roc_audio/iframe_writer.h"
#include "roc_core/noncopyable.h"

namespace roc {
namespace audio {

//! Poisons audio frames after writing them.
class PoisonWriter : public IFrameWriter, public core::NonCopyable<> {
public:
    //! Initialize.
    explicit PoisonWriter(IFrameWriter& writer);

    //! Write audio frame.
    virtual void write(Frame&);

private:
    IFrameWriter& writer_;
};

} // namespace audio
} // namespace roc

#endif // ROC_AUDIO_POISON_WRITER_H_
