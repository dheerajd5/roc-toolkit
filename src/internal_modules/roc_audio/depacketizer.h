/*
 * Copyright (c) 2015 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_audio/depacketizer.h
//! @brief Depacketizer.

#ifndef ROC_AUDIO_DEPACKETIZER_H_
#define ROC_AUDIO_DEPACKETIZER_H_

#include "roc_audio/iframe_decoder.h"
#include "roc_audio/iframe_reader.h"
#include "roc_audio/sample.h"
#include "roc_audio/sample_spec.h"
#include "roc_core/noncopyable.h"
#include "roc_core/rate_limiter.h"
#include "roc_packet/ireader.h"

namespace roc {
namespace audio {

//! Depacketizer.
//! @remarks
//!  Reads packets from a packet reader, decodes samples from packets using a
//!  decoder, and produces an audio stream.
class Depacketizer : public IFrameReader, public core::NonCopyable<> {
public:
    //! Initialization.
    //!
    //! @b Parameters
    //!  - @p reader is used to read packets
    //!  - @p payload_decoder is used to extract samples from packets
    //!  - @p sample_spec defines a set of channels in the output frames
    //!  - @p beep enables weird beeps instead of silence on packet loss
    Depacketizer(packet::IReader& reader,
                 IFrameDecoder& payload_decoder,
                 const audio::SampleSpec& sample_spec,
                 bool beep);

    //! Was depacketizer constructed without errors?
    bool is_valid() const;

    //! Did depacketizer catch first packet?
    bool is_started() const;

    //! Read audio frame.
    virtual bool read(Frame& frame);

    //! Get next timestamp to be rendered.
    //! @pre
    //!  is_started() should return true
    packet::timestamp_t next_timestamp() const;

private:
    struct FrameInfo {
        // Number of samples decoded from packets into the frame.
        size_t n_decoded_samples;

        // Number of packets dropped during frame construction.
        size_t n_dropped_packets;

        // This frame first sample timestamp.
        core::nanoseconds_t capture_ts;

        FrameInfo()
            : n_decoded_samples(0)
            , n_dropped_packets(0)
            , capture_ts(0) {
        }
    };

    void read_frame_(Frame& frame);

    sample_t* read_samples_(sample_t* buff_ptr, sample_t* buff_end, FrameInfo& info);

    sample_t* read_packet_samples_(sample_t* buff_ptr, sample_t* buff_end);
    sample_t* read_missing_samples_(sample_t* buff_ptr, sample_t* buff_end);

    void update_packet_(FrameInfo& info);
    packet::PacketPtr read_packet_();

    void set_frame_props_(Frame& frame, const FrameInfo& info);

    void report_stats_();

    packet::IReader& reader_;
    IFrameDecoder& payload_decoder_;

    const audio::SampleSpec sample_spec_;

    packet::PacketPtr packet_;

    packet::timestamp_t timestamp_;
    core::nanoseconds_t next_capture_ts_;
    bool valid_capture_ts_;

    packet::timestamp_t zero_samples_;
    packet::timestamp_t missing_samples_;
    packet::timestamp_t packet_samples_;

    core::RateLimiter rate_limiter_;

    const bool beep_;

    bool first_packet_;
    bool valid_;
};

} // namespace audio
} // namespace roc

#endif // ROC_AUDIO_DEPACKETIZER_H_
