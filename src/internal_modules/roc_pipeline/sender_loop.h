/*
 * Copyright (c) 2017 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_pipeline/sender_loop.h
//! @brief Sender pipeline loop.

#ifndef ROC_PIPELINE_SENDER_LOOP_H_
#define ROC_PIPELINE_SENDER_LOOP_H_

#include "roc_core/buffer_factory.h"
#include "roc_core/iarena.h"
#include "roc_core/mutex.h"
#include "roc_core/ticker.h"
#include "roc_pipeline/config.h"
#include "roc_pipeline/metrics.h"
#include "roc_pipeline/pipeline_loop.h"
#include "roc_pipeline/sender_sink.h"
#include "roc_sndio/isink.h"

namespace roc {
namespace pipeline {

//! Sender pipeline loop.
//
//! This class acts as a task-based facade for the sender pipeline subsystem of
//! roc_pipeline module (SenderSink, SenderSlot, SenderEndpoint, SenderSession).
//!
//! It provides two interfaces:
//!
//!  - sndio::ISink - can be used to pass samples to the pipeline
//!    (should be used from sndio thread)
//!
//!  - PipelineLoop - can be used to schedule tasks on the pipeline
//!    (can be used from any thread)
//!
//! @note
//!  Private inheritance from ISink is used to decorate actual implementation
//!  of ISink - SenderSource, in order to integrate it with PipelineLoop.
class SenderLoop : public PipelineLoop, private sndio::ISink {
public:
    //! Opaque slot handle.
    typedef struct SlotHandle* SlotHandle;

    //! Opaque endpoint handle.
    typedef struct EndpointHandle* EndpointHandle;

    //! Base task class.
    class Task : public PipelineTask {
    protected:
        friend class SenderLoop;

        Task();

        bool (SenderLoop::*func_)(Task&); //!< Task implementation method.

        SenderSlot* slot_;                   //!< Slot.
        SenderEndpoint* endpoint_;           //!< Endpoint.
        address::Interface iface_;           //!< Interface.
        address::Protocol proto_;            //!< Protocol.
        address::SocketAddr address_;        //!< Destination address.
        packet::IWriter* writer_;            //!< Destination writer.
        SenderSlotMetrics* slot_metrics_;    //!< Output for slot metrics.
        SenderSessionMetrics* sess_metrics_; //!< Output for session metrics.
    };

    //! Subclasses for specific tasks.
    class Tasks {
    public:
        //! Create new slot.
        class CreateSlot : public Task {
        public:
            //! Set task parameters.
            CreateSlot();

            //! Get created slot handle.
            SlotHandle get_handle() const;
        };

        //! Delete existing slot.
        class DeleteSlot : public Task {
        public:
            //! Set task parameters.
            DeleteSlot(SlotHandle slot);
        };

        //! Query slot metrics.
        class QuerySlot : public Task {
        public:
            //! Set task parameters.
            //! @remarks
            //!  Metrics are written to provided structs.
            QuerySlot(SlotHandle slot,
                      SenderSlotMetrics& slot_metrics,
                      SenderSessionMetrics* sess_metrics);
        };

        //! Create endpoint on given interface of the slot.
        class AddEndpoint : public Task {
        public:
            //! Set task parameters.
            //! @remarks
            //!  Each slot can have one source and zero or one repair endpoint.
            //!  The protocols of endpoints in one slot should be compatible.
            AddEndpoint(SlotHandle slot,
                        address::Interface iface,
                        address::Protocol proto,
                        const address::SocketAddr& dest_address,
                        packet::IWriter& dest_writer);

            //! Get created endpoint handle.
            EndpointHandle get_handle() const;
        };
    };

    //! Initialize.
    SenderLoop(IPipelineTaskScheduler& scheduler,
               const SenderConfig& config,
               const rtp::FormatMap& format_map,
               packet::PacketFactory& packet_factory,
               core::BufferFactory<uint8_t>& byte_buffer_factory,
               core::BufferFactory<audio::sample_t>& sample_buffer_factory,
               core::IArena& arena);

    //! Check if the pipeline was successfully constructed.
    bool is_valid() const;

    //! Get sender sink.
    //! @remarks
    //!  Samples written to the sink are sent to remote peers.
    sndio::ISink& sink();

private:
    // Methods of sndio::ISink
    virtual sndio::DeviceType type() const;
    virtual sndio::DeviceState state() const;
    virtual void pause();
    virtual bool resume();
    virtual bool restart();
    virtual audio::SampleSpec sample_spec() const;
    virtual core::nanoseconds_t latency() const;
    virtual bool has_latency() const;
    virtual bool has_clock() const;
    virtual void write(audio::Frame& frame);

    // Methods of PipelineLoop
    virtual core::nanoseconds_t timestamp_imp() const;
    virtual uint64_t tid_imp() const;
    virtual bool process_subframe_imp(audio::Frame&);
    virtual bool process_task_imp(PipelineTask&);

    // Methods for tasks
    bool task_create_slot_(Task&);
    bool task_delete_slot_(Task&);
    bool task_query_slot_(Task&);
    bool task_add_endpoint_(Task&);

    SenderSink sink_;
    core::Mutex sink_mutex_;

    core::Optional<core::Ticker> ticker_;
    core::Ticker::ticks_t ticker_ts_;

    bool auto_cts_;

    bool valid_;
};

} // namespace pipeline
} // namespace roc

#endif // ROC_PIPELINE_SENDER_LOOP_H_
