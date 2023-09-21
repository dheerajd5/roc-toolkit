/*
 * Copyright (c) 2023 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <CppUTest/TestHarness.h>

#include "roc_core/buffer_factory.h"
#include "roc_core/heap_arena.h"
#include "roc_core/scoped_ptr.h"
#include "roc_core/stddefs.h"
#include "roc_packet/packet_factory.h"
#include "roc_packet/queue.h"
#include "roc_packet/units.h"
#include "roc_rtp/composer.h"
#include "roc_rtp/format_map.h"
#include "roc_rtp/parser.h"
#include "roc_rtp/timestamp_injector.h"

namespace roc {
namespace rtp {

namespace {

core::HeapArena arena;
static packet::PacketFactory packet_factory(arena);

packet::PacketPtr new_packet(packet::seqnum_t sn, packet::timestamp_t ts) {
    packet::PacketPtr packet = packet_factory.new_packet();
    CHECK(packet);

    packet->add_flags(packet::Packet::FlagRTP);
    packet->rtp()->seqnum = sn;
    packet->rtp()->timestamp = ts;

    return packet;
}
} // namespace

TEST_GROUP(timestamp_injector) {};

TEST(timestamp_injector, negative_and_positive_dn) {
    enum {
        NumCh = 2,
        ChMask = 3,
        PacketSz = 128,
        NPackets = 128,
    };

    const float sample_rate = 48000.;
    const audio::SampleSpec sample_spec =
        audio::SampleSpec((size_t)sample_rate, audio::ChanLayout_Surround, ChMask);

    packet::timestamp_t rtp_ts = 2222;
    packet::timestamp_t packet_rtp_ts = (packet::timestamp_t)-4444;
    const core::nanoseconds_t epsilon = core::nanoseconds_t(1.f / sample_rate * 1e9f);

    core::nanoseconds_t cur_packet_capt_ts = 1691499037871419405;
    const core::nanoseconds_t reference_capt_ts = cur_packet_capt_ts
        + sample_spec.samples_per_chan_2_ns(
            (size_t)packet::timestamp_diff(rtp_ts, packet_rtp_ts));

    DOUBLES_EQUAL((reference_capt_ts - cur_packet_capt_ts) * 1e-9f * sample_rate,
                  rtp_ts - packet_rtp_ts, 1e-3);

    packet::Queue queue;
    TimestampInjector injector(queue, sample_spec);
    injector.update_mapping(reference_capt_ts, rtp_ts);

    LONGS_EQUAL(0, queue.size());
    for (size_t i = 0; i < NPackets; i++) {
        queue.write(new_packet((packet::seqnum_t)i,
                               (packet::timestamp_t)(packet_rtp_ts + i * PacketSz)));
    }
    LONGS_EQUAL(NPackets, queue.size());

    core::nanoseconds_t ts_step = sample_spec.samples_per_chan_2_ns(PacketSz);
    for (size_t i = 0; i < NPackets; i++) {
        packet::PacketPtr packet = injector.read();
        const core::nanoseconds_t pkt_capt_ts = packet->rtp()->capture_timestamp;

        // Assume error must be less than 0.1 of samples period.
        CHECK(core::ns_equal_delta(cur_packet_capt_ts, pkt_capt_ts, epsilon));
        cur_packet_capt_ts += ts_step;
    }
}

} // namespace rtp
} // namespace roc
