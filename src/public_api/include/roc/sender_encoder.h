/*
 * Copyright (c) 2023 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * \file roc/sender_encoder.h
 * \brief Roc sender encoder.
 */

#ifndef ROC_SENDER_ENCODER_H_
#define ROC_SENDER_ENCODER_H_

#include "roc/config.h"
#include "roc/context.h"
#include "roc/frame.h"
#include "roc/metrics.h"
#include "roc/packet.h"
#include "roc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Sender encoder node.
 *
 * Sender encoder gets an audio stream from the user, encodes it into network packets, and
 * provides encoded packets back to the user.
 *
 * Sender encoder is a simplified networkless version of \ref roc_sender. It implements
 * the same pipeline, but instead of sending packets, it just provides them to the user.
 * The user is responsible for delivering these packets to receiver.
 *
 * For detailed description of sender pipeline, see documentation for \ref roc_sender.
 *
 * **Life cycle**
 *
 * - Encoder is created using roc_sender_encoder_open().
 *
 * - The user activates one or more interfaces by invoking roc_sender_encoder_activate().
 *   This tells encoder what types of streams to produces and what protocols to use for
 *   them (e.g. only audio packets or also redundancy packets).
 *
 * - The audio stream is iteratively pushed to the encoder using
 *   roc_sender_encoder_push(). The sender encodes the stream into packets and accumulates
 *   them in internal queue.
 *
 * - The packet stream is iteratively popped from the encoder internal queue using
 *   roc_sender_encoder_pop(). User should retrieve all available packets from all
 *   activated interfaces every time after pushing a frame.
 *
 * - User is responsible for delivering packets to \ref roc_receiver_decoder and pushing
 *   them to appropriate interfaces of decoder.
 *
 * - The sender is eventually destroyed using roc_sender_encoder_close().
 *
 * **Interfaces and protocols**
 *
 * Sender encoder may have one or several *interfaces*, as defined in \ref roc_interface.
 * The interface defines the type of the communication with the remote node and the set
 * of the protocols supported by it.
 *
 * Each interface has its own packet queue. When a frame is pushed to the encoder, it
 * may produce multiple packets for each interface queue. The user then should pop
 * packets from each interface that was activated.
 *
 * **Thread safety**
 *
 * Can be used concurrently.
 */
typedef struct roc_sender_encoder roc_sender_encoder;

/** Open a new encoder.
 *
 * Allocates and initializes a new encoder, and attaches it to the context.
 *
 * **Parameters**
 *  - \p context should point to an opened context
 *  - \p config should point to an initialized config
 *  - \p result should point to an unitialized roc_sender_encoder pointer
 *
 * **Returns**
 *  - returns zero if the encoder was successfully created
 *  - returns a negative value if the arguments are invalid
 *  - returns a negative value on resource allocation failure
 *
 * **Ownership**
 *  - doesn't take or share the ownership of \p config; it may be safely deallocated
 *    after the function returns
 *  - passes the ownership of \p result to the user; the user is responsible to call
 *    roc_sender_encoder_close() to free it
 *  - attaches created encoder to \p context; the user should not close context
 *    before closing encoder
 */
ROC_API int roc_sender_encoder_open(roc_context* context,
                                    const roc_sender_config* config,
                                    roc_sender_encoder** result);

/** Activate encoder interface.
 *
 * Checks that the protocol is valid and supported by the interface, and
 * initializes given interface with given protocol.
 *
 * The user should invoke roc_sender_encoder_pop() for all activated interfaces
 * and deliver packets to appropriate interfaces of \ref roc_receiver_decoder.
 *
 * **Parameters**
 *  - \p encoder should point to an opened encoder
 *  - \p iface specifies the encoder interface
 *  - \p proto specifies the encoder protocol
 *
 * **Returns**
 *  - returns zero if interface was successfully activated
 *  - returns a negative value if the arguments are invalid
 *  - returns a negative value on resource allocation failure
 */
ROC_API int roc_sender_encoder_activate(roc_sender_encoder* encoder,
                                        roc_interface iface,
                                        roc_protocol proto);

/** Query encoder metrics.
 *
 * Reads encoder metrics into provided struct.
 *
 * **Parameters**
 *  - \p encoder should point to an opened encoder
 *  - \p metrics specifies struct where to write metrics
 *
 * **Returns**
 *  - returns zero if the slot was successfully removed
 *  - returns a negative value if the arguments are invalid
 *  - returns a negative value if the slot does not exist
 *
 * **Ownership**
 *  - doesn't take or share the ownership of \p metrics; it
 *    may be safely deallocated after the function returns
 */
ROC_API int roc_sender_encoder_query(roc_sender_encoder* encoder,
                                     roc_sender_metrics* metrics);

/** Write frame to encoder.
 *
 * Encodes samples to into network packets and enqueues them to internal queues of
 * activated interfaces.
 *
 * If \ref ROC_CLOCK_SOURCE_INTERNAL is used, the function blocks until it's time to
 * encode the samples according to the configured sample rate.
 *
 * Until at least one interface is activated, the stream is just dropped.
 *
 * **Parameters**
 *  - \p encoder should point to an opened encoder
 *  - \p frame should point to an initialized frame; it should contain pointer to
 *    a buffer and it's size; the buffer is fully copied into encoder
 *
 * **Returns**
 *  - returns zero if all samples were successfully encoded and enqueued
 *  - returns a negative value if the arguments are invalid
 *  - returns a negative value on resource allocation failure
 *
 * **Ownership**
 *  - doesn't take or share the ownership of \p frame; it may be safely deallocated
 *    after the function returns
 */
ROC_API int roc_sender_encoder_push(roc_sender_encoder* encoder, const roc_frame* frame);

/** Read packet from encoder.
 *
 * Removes encoded packet from interface queue and returns it to the user.
 *
 * Packets are added to the queue from roc_sender_encoder_push(). Each push may produce
 * multiple packets, so the user should iteratively pop packets until error. This should
 * be repeated for all activated interfaces.
 *
 * **Parameters**
 *  - \p encoder should point to an opened encoder
 *  - \p packet should point to an initialized packet; it should contain pointer to
 *    a buffer and it's size; packet bytes are copied to user's buffer and the
 *    size field is updated with the actual packet size
 *
 * **Returns**
 *  - returns zero if a packet was successfully copied from encoder
 *  - returns a negative value if there are no more packets for this interface
 *  - returns a negative value if the interface is not activated
 *  - returns a negative value if the buffer size of the provided packet is too small
 *  - returns a negative value if the arguments are invalid
 *  - returns a negative value on resource allocation failure
 *
 * **Ownership**
 *  - doesn't take or share the ownership of \p packet; it may be safely deallocated
 *    after the function returns
 */
ROC_API int roc_sender_encoder_pop(roc_sender_encoder* encoder,
                                   roc_interface iface,
                                   roc_packet* packet);

/** Close encoder.
 *
 * Deinitializes and deallocates the encoder, and detaches it from the context. The user
 * should ensure that nobody uses the encoder during and after this call. If this
 * function fails, the encoder is kept opened and attached to the context.
 *
 * **Parameters**
 *  - \p encoder should point to an opened encoder
 *
 * **Returns**
 *  - returns zero if the encoder was successfully closed
 *  - returns a negative value if the arguments are invalid
 *
 * **Ownership**
 *  - ends the user ownership of \p encoder; it can't be used anymore after the
 *    function returns
 */
ROC_API int roc_sender_encoder_close(roc_sender_encoder* encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ROC_SENDER_ENCODER_H_ */
