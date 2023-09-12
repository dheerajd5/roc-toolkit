/*
 * Copyright (c) 2015 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ROC_PIPELINE_TEST_HELPERS_PACKET_SENDER_H_
#define ROC_PIPELINE_TEST_HELPERS_PACKET_SENDER_H_

#include <CppUTest/TestHarness.h>

#include "roc_core/noncopyable.h"
#include "roc_packet/iwriter.h"
#include "roc_packet/packet_factory.h"
#include "roc_packet/queue.h"

namespace roc {
namespace pipeline {
namespace test {

class PacketSender : public packet::IWriter, core::NonCopyable<> {
public:
    PacketSender(packet::PacketFactory& packet_factory,
                 packet::IWriter* source_writer,
                 packet::IWriter* repair_writer,
                 packet::IWriter* control_writer)
        : packet_factory_(packet_factory)
        , source_writer_(source_writer)
        , repair_writer_(repair_writer)
        , control_writer_(control_writer)
        , n_source_(0)
        , n_repair_(0)
        , n_control_(0) {
    }

    virtual void write(const packet::PacketPtr& pp) {
        queue_.write(pp);
    }

    size_t n_source() const {
        return n_source_;
    }

    size_t n_repair() const {
        return n_repair_;
    }

    size_t n_control() const {
        return n_control_;
    }

    void deliver(size_t n_source_packets) {
        for (size_t np = 0; np < n_source_packets;) {
            packet::PacketPtr pp = queue_.read();
            if (!pp) {
                break;
            }

            if (pp->flags() & packet::Packet::FlagControl) {
                CHECK(control_writer_);
                control_writer_->write(copy_packet_(pp));
                n_control_++;
            } else if (pp->flags() & packet::Packet::FlagRepair) {
                CHECK(repair_writer_);
                repair_writer_->write(copy_packet_(pp));
                n_repair_++;
            } else {
                CHECK(source_writer_);
                source_writer_->write(copy_packet_(pp));
                n_source_++;
                np++;
            }
        }
    }

private:
    // creates a new packet with the same buffer, clearing all meta-information
    // like flags, parsed fields, etc; this way we simulate delivering packet
    // over network
    packet::PacketPtr copy_packet_(const packet::PacketPtr& pa) {
        packet::PacketPtr pb = packet_factory_.new_packet();
        CHECK(pb);

        CHECK(pa->flags() & packet::Packet::FlagUDP);
        pb->add_flags(packet::Packet::FlagUDP);
        *pb->udp() = *pa->udp();

        pb->set_data(pa->data());

        return pb;
    }

    packet::PacketFactory& packet_factory_;

    packet::IWriter* source_writer_;
    packet::IWriter* repair_writer_;
    packet::IWriter* control_writer_;

    size_t n_source_;
    size_t n_repair_;
    size_t n_control_;

    packet::Queue queue_;
};

} // namespace test
} // namespace pipeline
} // namespace roc

#endif // ROC_PIPELINE_TEST_HELPERS_PACKET_SENDER_H_
