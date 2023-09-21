/*
 * Copyright (c) 2015 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_packet/print_packet.h"
#include "roc_address/socket_addr_to_str.h"
#include "roc_core/print_buffer.h"
#include "roc_core/printer.h"
#include "roc_packet/fec_scheme_to_str.h"
#include "roc_packet/packet.h"

namespace roc {
namespace packet {

void print_packet(const Packet& pkt, int flags) {
    core::Printer p;

    p.writef("@ packet [%p]\n", (const void*)&pkt);

    if (pkt.udp()) {
        p.writef(" udp: src=%s dst=%s\n",
                 address::socket_addr_to_str(pkt.udp()->src_addr).c_str(),
                 address::socket_addr_to_str(pkt.udp()->dst_addr).c_str());
    }

    if (pkt.rtp()) {
        p.writef(
            " rtp: src=%lu m=%d sn=%lu ts=%lu dur=%lu cts=%lld pt=%u payload_sz=%lu\n",
            (unsigned long)pkt.rtp()->source, (int)pkt.rtp()->marker,
            (unsigned long)pkt.rtp()->seqnum, (unsigned long)pkt.rtp()->timestamp,
            (unsigned long)pkt.rtp()->duration, (long long)pkt.rtp()->capture_timestamp,
            (unsigned int)pkt.rtp()->payload_type,
            (unsigned long)pkt.rtp()->payload.size());

        if ((flags & PrintPayload) && pkt.rtp()->payload) {
            core::print_buffer(pkt.rtp()->payload.data(), pkt.rtp()->payload.size());
        }
    }

    if (pkt.fec()) {
        p.writef(" fec: %s esi=%lu sbn=%lu sblen=%lu blen=%lu payload_sz=%lu\n",
                 fec_scheme_to_str(pkt.fec()->fec_scheme),
                 (unsigned long)pkt.fec()->encoding_symbol_id,
                 (unsigned long)pkt.fec()->source_block_number,
                 (unsigned long)pkt.fec()->source_block_length,
                 (unsigned long)pkt.fec()->block_length,
                 (unsigned long)pkt.fec()->payload.size());

        if ((flags & PrintPayload) && pkt.fec()->payload) {
            core::print_buffer(pkt.fec()->payload.data(), pkt.fec()->payload.size());
        }
    }

    if (pkt.rtcp()) {
        p.writef(" rtcp: size=%lu\n", (unsigned long)pkt.rtcp()->data.size());
    }
}

} // namespace packet
} // namespace roc
