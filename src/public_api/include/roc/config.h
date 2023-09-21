/*
 * Copyright (c) 2017 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * \file roc/config.h
 * \brief Constants and configs.
 */

#ifndef ROC_CONFIG_H_
#define ROC_CONFIG_H_

#include "roc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Network slot.
 *
 * A node (sender or receiver) may have multiple slots, which may be independently
 * bound or connected. You can use multiple slots on sender to connect it to multiple
 * receiver addresses, and you can use multiple slots on receiver to bind it to
 * multiple receiver addresses.
 *
 * Slots are numbered from zero and are created implicitly. Just specify slot index
 * when binding or connecting endpoint, and slot will be automatically created if it
 * was not created yet. Numbers does not need to be continuous and can be arbitrary.
 *
 * In simple cases, when one slot is enough, just use \c ROC_SLOT_DEFAULT.
 *
 * Inside each slot, there can be up to one endpoint for each interface type.
 * See \ref roc_interface for details.
 */
typedef unsigned long long roc_slot;

/** Alias for the slot with index zero.
 * Has no special meaning, exists just for convenience.
 * \see roc_slot
 */
static const roc_slot ROC_SLOT_DEFAULT = 0;

/** Network interface.
 *
 * Interface is a way to access the node (sender or receiver) via network.
 *
 * Each node slot has multiple interfaces, one of each type. The user interconnects
 * nodes by binding one of the first node's interfaces to an URI and then connecting the
 * corresponding second node's interface to that URI.
 *
 * A URI is represented by \ref roc_endpoint object.
 *
 * The interface defines the type of the communication with the remote node and the
 * set of protocols (URI schemes) that can be used with this particular interface.
 *
 * \c ROC_INTERFACE_CONSOLIDATED is an interface for high-level protocols which
 * automatically manage all necessary communication: transport streams, control messages,
 * parameter negotiation, etc. When a consolidated connection is established, nodes may
 * automatically setup lower-level interfaces like \c ROC_INTERFACE_AUDIO_SOURCE, \c
 * ROC_INTERFACE_AUDIO_REPAIR, and \c ROC_INTERFACE_AUDIO_CONTROL.
 *
 * \c ROC_INTERFACE_CONSOLIDATED is mutually exclusive with lower-level interfaces.
 * In most cases, the user needs only \c ROC_INTERFACE_CONSOLIDATED. However, the
 * lower-level interfaces may be useful if an external signaling mechanism is used or for
 * compatibility with third-party software.
 *
 * \c ROC_INTERFACE_AUDIO_SOURCE and \c ROC_INTERFACE_AUDIO_REPAIR are lower-level
 * unidirectional transport-only interfaces. The first is used to transmit audio stream,
 * and the second is used to transmit redundant repair stream, if FEC is enabled.
 *
 * \c ROC_INTERFACE_AUDIO_CONTROL is a lower-level interface for control streams.
 * If you use \c ROC_INTERFACE_AUDIO_SOURCE and \c ROC_INTERFACE_AUDIO_REPAIR, you
 * usually also need to use \c ROC_INTERFACE_AUDIO_CONTROL to enable carrying additional
 * non-transport information.
 */
typedef enum roc_interface {
    /** Interface that consolidates all types of streams (source, repair, control).
     *
     * Allowed operations:
     *  - bind    (sender, receiver)
     *  - connect (sender, receiver)
     *
     * Allowed protocols:
     *  - \ref ROC_PROTO_RTSP
     */
    ROC_INTERFACE_CONSOLIDATED = 1,

    /** Interface for audio stream source data.
     *
     * Allowed operations:
     *  - bind    (receiver)
     *  - connect (sender)
     *
     * Allowed protocols:
     *  - \ref ROC_PROTO_RTP
     *  - \ref ROC_PROTO_RTP_RS8M_SOURCE
     *  - \ref ROC_PROTO_RTP_LDPC_SOURCE
     */
    ROC_INTERFACE_AUDIO_SOURCE = 11,

    /** Interface for audio stream repair data.
     *
     * Allowed operations:
     *  - bind    (receiver)
     *  - connect (sender)
     *
     * Allowed protocols:
     *  - \ref ROC_PROTO_RS8M_REPAIR
     *  - \ref ROC_PROTO_LDPC_REPAIR
     */
    ROC_INTERFACE_AUDIO_REPAIR = 12,

    /** Interface for audio control messages.
     *
     * Allowed operations:
     *  - bind    (sender, receiver)
     *  - connect (sender, receiver)
     *
     * Allowed protocols:
     *  - \ref ROC_PROTO_RTCP
     */
    ROC_INTERFACE_AUDIO_CONTROL = 13
} roc_interface;

/** Network protocol.
 *
 * Defines URI scheme of \ref roc_endpoint.
 */
typedef enum roc_protocol {
    /** RTSP 1.0 (RFC 2326) or RTSP 2.0 (RFC 7826).
     *
     * Interfaces:
     *  - \ref ROC_INTERFACE_CONSOLIDATED
     *
     * Transports:
     *   - for signaling: TCP
     *   - for media: RTP and RTCP over UDP or TCP
     */
    ROC_PROTO_RTSP = 10,

    /** RTP over UDP (RFC 3550).
     *
     * Interfaces:
     *  - \ref ROC_INTERFACE_AUDIO_SOURCE
     *
     * Transports:
     *  - UDP
     *
     * Audio encodings:
     *   - \ref ROC_PACKET_ENCODING_AVP_L16_MONO
     *   - \ref ROC_PACKET_ENCODING_AVP_L16_STEREO
     *   - encodings registered using roc_context_register_encoding()
     *
     * FEC encodings:
     *   - none
     */
    ROC_PROTO_RTP = 20,

    /** RTP source packet (RFC 3550) + FECFRAME Reed-Solomon footer (RFC 6865) with m=8.
     *
     * Interfaces:
     *  - \ref ROC_INTERFACE_AUDIO_SOURCE
     *
     * Transports:
     *  - UDP
     *
     * Audio encodings:
     *  - similar to \ref ROC_PROTO_RTP
     *
     * FEC encodings:
     *  - \ref ROC_FEC_ENCODING_RS8M
     */
    ROC_PROTO_RTP_RS8M_SOURCE = 30,

    /** FEC repair packet + FECFRAME Reed-Solomon header (RFC 6865) with m=8.
     *
     * Interfaces:
     *  - \ref ROC_INTERFACE_AUDIO_REPAIR
     *
     * Transports:
     *  - UDP
     *
     * FEC encodings:
     *  - \ref ROC_FEC_ENCODING_RS8M
     */
    ROC_PROTO_RS8M_REPAIR = 31,

    /** RTP source packet (RFC 3550) + FECFRAME LDPC-Staircase footer (RFC 6816).
     *
     * Interfaces:
     *  - \ref ROC_INTERFACE_AUDIO_SOURCE
     *
     * Transports:
     *  - UDP
     *
     * Audio encodings:
     *  - similar to \ref ROC_PROTO_RTP
     *
     * FEC encodings:
     *  - \ref ROC_FEC_ENCODING_LDPC_STAIRCASE
     */
    ROC_PROTO_RTP_LDPC_SOURCE = 32,

    /** FEC repair packet + FECFRAME LDPC-Staircase header (RFC 6816).
     *
     * Interfaces:
     *  - \ref ROC_INTERFACE_AUDIO_REPAIR
     *
     * Transports:
     *  - UDP
     *
     * FEC encodings:
     *  - \ref ROC_FEC_ENCODING_LDPC_STAIRCASE
     */
    ROC_PROTO_LDPC_REPAIR = 33,

    /** RTCP over UDP (RFC 3550).
     *
     * Interfaces:
     *  - \ref ROC_INTERFACE_AUDIO_CONTROL
     *
     * Transports:
     *  - UDP
     */
    ROC_PROTO_RTCP = 70
} roc_protocol;

/** Packet encoding.
 * Each packet encoding defines sample format, channel layout, and rate.
 * Each packet encoding is caompatible with specific protocols.
 */
typedef enum roc_packet_encoding {
    /** PCM signed 16-bit, 1 channel, 44100 rate.
     *
     * Represents 1-channel L16 stereo encoding from RTP A/V Profile (RFC 3551).
     * Uses uncompressed samples coded as interleaved 16-bit signed big-endian
     * integers in two's complement notation.
     *
     * Supported by protocols:
     *  - \ref ROC_PROTO_RTP
     *  - \ref ROC_PROTO_RTP_RS8M_SOURCE
     *  - \ref ROC_PROTO_RTP_LDPC_SOURCE
     */
    ROC_PACKET_ENCODING_AVP_L16_MONO = 11,

    /** PCM signed 16-bit, 2 channels, 44100 rate.
     *
     * Represents 2-channel L16 stereo encoding from RTP A/V Profile (RFC 3551).
     * Uses uncompressed samples coded as interleaved 16-bit signed big-endian
     * integers in two's complement notation.
     *
     * Supported by protocols:
     *  - \ref ROC_PROTO_RTP
     *  - \ref ROC_PROTO_RTP_RS8M_SOURCE
     *  - \ref ROC_PROTO_RTP_LDPC_SOURCE
     */
    ROC_PACKET_ENCODING_AVP_L16_STEREO = 10,
} roc_packet_encoding;

/** Forward Error Correction encoding.
 * Each FEC encoding is caompatible with specific protocols.
 */
typedef enum roc_fec_encoding {
    /** No FEC encoding.
     *
     * Compatible with \ref ROC_PROTO_RTP protocol.
     *
     * Pros:
     *  - compatible with third-party software that does not support FECFRAME
     *
     * Cons:
     *  - no packet recovery
     */
    ROC_FEC_ENCODING_DISABLE = -1,

    /** Default FEC encoding.
     * Current default is \ref ROC_FEC_ENCODING_RS8M.
     */
    ROC_FEC_ENCODING_DEFAULT = 0,

    /** Reed-Solomon FEC encoding (RFC 6865) with m=8.
     *
     * Good for small block sizes (below 256 packets).
     * Compatible with \ref ROC_PROTO_RTP_RS8M_SOURCE and \ref ROC_PROTO_RS8M_REPAIR
     * protocols for source and repair endpoints.
     *
     * Pros:
     *  - good repair capabilities even on small block sizes
     *
     * Cons:
     *  - high CPU usage on large block sizes
     */
    ROC_FEC_ENCODING_RS8M = 1,

    /** LDPC-Staircase FEC encoding (RFC 6816).
     *
     * Good for large block sizes (above 1024 packets).
     * Compatible with \ref ROC_PROTO_RTP_LDPC_SOURCE and \ref ROC_PROTO_LDPC_REPAIR
     * protocols for source and repair endpoints.
     *
     * Pros:
     *  - low CPU usage even on large block sizes
     *
     * Cons:
     *  - low repair capabilities on small block sizes
     */
    ROC_FEC_ENCODING_LDPC_STAIRCASE = 2
} roc_fec_encoding;

/** Sample format.
 * Defines how each sample is represented.
 * Does not define channels layout and sample rate.
 */
typedef enum roc_format {
    /** PCM floats.
     * Uncompressed samples coded as 32-bit native-endian floats in range [-1; 1].
     * Channels are interleaved, e.g. two channels are encoded as "L R L R ...".
     */
    ROC_FORMAT_PCM_FLOAT32 = 1
} roc_format;

/** Channel layout.
 * Defines number of channels and meaning of each channel.
 */
typedef enum roc_channel_layout {
    /** Multi-track audio.
     *
     * In multitrack layout, stream contains multiple channels which represent
     * independent "tracks" without any special meaning (unlike stereo or surround)
     * and hence without any special processing or mapping.
     *
     * The number of channels is arbitrary and is defined by \c tracks field of
     * \ref roc_media_encoding struct.
     */
    ROC_CHANNEL_LAYOUT_MULTITRACK = 1,

    /** Mono.
     * One channel with monophonic sound.
     */
    ROC_CHANNEL_LAYOUT_MONO = 2,

    /** Stereo.
     * Two channels: left, right.
     */
    ROC_CHANNEL_LAYOUT_STEREO = 3,
} roc_channel_layout;

/** Media encoding.
 * Defines format and parameters of samples encoded in frames or packets.
 */
typedef struct roc_media_encoding {
    /** Sample frequency.
     * Defines number of samples per channel per second (e.g. 44100).
     */
    unsigned int rate;

    /** Sample format.
     * Defines sample precision and encoding.
     */
    roc_format format;

    /** Channel layout.
     * Defines number of channels and meaning of each channel.
     */
    roc_channel_layout channels;

    /** Multi-track channel count.
     *
     * If \c channels is \ref ROC_CHANNEL_LAYOUT_MULTITRACK, defines
     * number of channels (which represent independent "tracks").
     * For other channel layouts should be zero.
     *
     * Should be in range [1; 1024].
     */
    unsigned int tracks;
} roc_media_encoding;

/** Clock source for sender or receiver.
 * Defines wo is responsible to invoke read or write in proper time.
 */
typedef enum roc_clock_source {
    /** Sender or receiver is clocked by external user-defined clock.
     *
     * Write and read operations are non-blocking. The user is responsible
     * to call them in time, according to the external clock.
     *
     * Use when samples source (from where you read them to pass to receiver)
     * or destination (to where you write them after obtaining from sender)
     * is active and has its own clock, e.g. it is a sound card.
     */
    ROC_CLOCK_SOURCE_EXTERNAL = 0,

    /** Sender or receiver is clocked by an internal pipeline clock.
     *
     * Write and read operations are blocking. They automatically wait until it's time
     * to process the next bunch of samples according to the configured sample rate,
     * based on a CPU timer.
     *
     * Use when samples source (from where you read them to pass to receiver)
     * or destination (to where you write them after obtaining from sender)
     * is passive and does now have clock, e.g. it is a file on disk.
     */
    ROC_CLOCK_SOURCE_INTERNAL = 1
} roc_clock_source;

/** Clock synchronization algorithm.
 * Defines how sender and receiver clocks are synchronized.
 */
typedef enum roc_clock_sync_backend {
    /** Disable clock synchronization.
     *
     * In this mode, sender and receiver clocks are not synchronized. This mode is
     * generally not recommended, since clock drift will lead to periodic playback
     * disruptions caused by underruns and overruns.
     */
    ROC_CLOCK_SYNC_BACKEND_DISABLE = -1,

    /** Default backend.
     * Current default is \c ROC_CLOCK_SYNC_BACKEND_NIQ.
     */
    ROC_CLOCK_SYNC_BACKEND_DEFAULT = 0,

    /** Clock synchronization based on network incoming queue size.
     *
     * In this mode, receiver monitors incoming queue size and adjusts playback clock
     * speed to match the estimated capture clock speed.
     *
     * Pros:
     *  - works with any protocol (does not require RTCP or NTP)
     *
     * Cons:
     *  - synchronizes only clock speed, but not position; different receivers will
     *    have different (constant) delays
     *  - affected by network jitter; spikes in packet delivery will cause slow
     *    oscillations in clock speed
     */
    ROC_CLOCK_SYNC_BACKEND_NIQ = 2
} roc_clock_sync_backend;

/** Clock synchronization profile.
 * Defines what latency and jitter are tolerated by clock synchronization algorithm.
 */
typedef enum roc_clock_sync_profile {
    /** Default profile.
     *
     * When \ref ROC_CLOCK_SYNC_BACKEND_NIQ is used, selects \ref
     * ROC_CLOCK_SYNC_PROFILE_RESPONSIVE if target latency is low, and \ref
     * ROC_CLOCK_SYNC_PROFILE_GRADUAL if target latency is high.
     */
    ROC_CLOCK_SYNC_PROFILE_DEFAULT = 0,

    /** Responsive clock adjustment.
     *
     * Clock speed is adjusted quickly and accurately.
     *
     * Requires high precision clock adjustment, hence recommended for use with
     * \ref ROC_RESAMPLER_BACKEND_BUILTIN.
     *
     * Pros:
     *  - allows very low latency or synchronization error
     *
     * Cons:
     *  - does not work well with some resampler backends
     *  - does not work well with \ref ROC_CLOCK_SYNC_BACKEND_NIQ
     *    if network jitter is high
     */
    ROC_CLOCK_SYNC_PROFILE_RESPONSIVE = 1,

    /** Gradual clock adjustment.
     *
     * Clock speed is adjusted slowly and smoothly.
     *
     * Pros:
     *  - works well even with high network jitter
     *  - works well with any resampler backend
     *
     * Cons:
     *  - does not allow very low latency or synchronization error
     */
    ROC_CLOCK_SYNC_PROFILE_GRADUAL = 2
} roc_clock_sync_profile;

/** Resampler backend.
 * Affects CPU usage, quality, and clock synchronization precision.
 * Some backends may be disabled at build time.
 */
typedef enum roc_resampler_backend {
    /** Default backend.
     *
     * Selects \ref ROC_RESAMPLER_BACKEND_BUILTIN when using \ref
     * ROC_CLOCK_SYNC_PROFILE_RESPONSIVE, or when SpeexDSP is disabled.
     *
     * Otherwise, selects \ref ROC_RESAMPLER_BACKEND_SPEEX.
     */
    ROC_RESAMPLER_BACKEND_DEFAULT = 0,

    /** CPU-intensive good-quality high-precision built-in resampler.
     *
     * This backend controls clock speed with high precision, and hence is useful when
     * latency or synchronization error should be very low.
     *
     * This backend has high CPU usage, especially on high resampling quality and on
     * CPUs with small caches.
     *
     * This backend is always available.
     *
     * Recommended for \ref ROC_CLOCK_SYNC_PROFILE_RESPONSIVE and on good CPUs.
     */
    ROC_RESAMPLER_BACKEND_BUILTIN = 1,

    /** Very fast good-quality low-precision resampler based on SpeexDSP.
     *
     * This backend has low CPU usage even on high resampler quality and cheap CPUs.
     *
     * This backend controls clock speed with lower precision, and is not so good when
     * latency or synchronization error should be very low.
     *
     * This backend is available if SpeexDSP was enabled at build time.
     *
     * Recommended for \ref ROC_CLOCK_SYNC_PROFILE_GRADUAL and on cheap CPUs.
     */
    ROC_RESAMPLER_BACKEND_SPEEX = 2
} roc_resampler_backend;

/** Resampler profile.
 * Affects CPU usage and quality.
 * Each resampler backend treats profile in its own way.
 */
typedef enum roc_resampler_profile {
    /** Default profile.
     * Current default is \c ROC_RESAMPLER_PROFILE_MEDIUM.
     */
    ROC_RESAMPLER_PROFILE_DEFAULT = 0,

    /** High quality, higher CPU usage. */
    ROC_RESAMPLER_PROFILE_HIGH = 1,

    /** Medium quality, medium CPU usage. */
    ROC_RESAMPLER_PROFILE_MEDIUM = 2,

    /** Low quality, lower CPU usage. */
    ROC_RESAMPLER_PROFILE_LOW = 3
} roc_resampler_profile;

/** Context configuration.
 *
 * It is safe to memset() this struct with zeros to get a default config. It is also
 * safe to memcpy() this struct to get a copy of config.
 *
 * \see roc_context
 */
typedef struct roc_context_config {
    /** Maximum size in bytes of a network packet.
     * Defines the amount of bytes allocated per network packet.
     * Sender and receiver won't handle packets larger than this.
     * If zero, default value is used.
     */
    unsigned int max_packet_size;

    /** Maximum size in bytes of an audio frame.
     * Defines the amount of bytes allocated per intermediate internal frame in the
     * pipeline. Does not limit the size of the frames provided by user.
     * If zero, default value is used.
     */
    unsigned int max_frame_size;
} roc_context_config;

/** Sender configuration.
 *
 * For most fields, zero value means "use default", and you can memset() this struct
 * with zeros and then set only a few fields that don't have defaults. It is safe to
 * use memcpy() to get a copy of config, the struct is flat.
 *
 * \see roc_sender
 */
typedef struct roc_sender_config {
    /** The encoding used in frames passed to sender.
     * Frame encoding defines sample format, channel layout, and sample rate in local
     * frames created by user and passed to sender.
     * Should be set (zero value is invalid).
     */
    roc_media_encoding frame_encoding;

    /** The encoding used for packets produced by sender.
     *
     * Packet encoding defines sample format, channel layout, and sample rate in network
     * packets. If packet encoding differs from frame encoding, conversion is performed
     * automatically.
     *
     * If zero, sender selects packet encoding automatically based on \c frame_encoding.
     * This automatic selection matches only encodings that have exact same sample rate
     * and channel layout, and hence don't require conversions. If you need conversions,
     * you should set packet encoding explicitly.
     *
     * If you want to force specific packet encoding, and built-in set of encodings is
     * not enough, you can use \ref roc_context_register_encoding() to register custom
     * encoding, set \c packet_encoding to registered identifier. If you use signaling
     * protocol like RTSP, it's enough to register in just on sender; otherwise, you
     * need to do the same on receiver as well.
     */
    roc_packet_encoding packet_encoding;

    /** The length of the packets produced by sender, in nanoseconds.
     * Number of nanoseconds encoded per packet.
     * The samples written to the sender are buffered until the full packet is
     * accumulated or the sender is flushed or closed. Larger number reduces
     * packet overhead but also increases latency.
     * If zero, default value is used.
     */
    unsigned long long packet_length;

    /** Enable packet interleaving.
     * If non-zero, the sender shuffles packets before sending them. This
     * may increase robustness but also increases latency.
     */
    unsigned int packet_interleaving;

    /** FEC encoding to use.
     * If non-zero, the sender employs a FEC encoding to generate redundant packets
     * which may be used on receiver to restore lost packets. This requires both
     * sender and receiver to use two separate source and repair endpoints.
     */
    roc_fec_encoding fec_encoding;

    /** Number of source packets per FEC block.
     * Used if some FEC encoding is selected.
     *
     * Sender divides stream into blocks of N source (media) packets, and adds M repair
     * (redundancy) packets to each block, where N is \c fec_block_source_packets and M
     * is \c fec_block_repair_packets.
     *
     * Larger number of source packets in block increases robustness (repair ratio), but
     * also increases latency.
     *
     * If zero, default value is used.
     */
    unsigned int fec_block_source_packets;

    /** Number of repair packets per FEC block.
     * Used if some FEC encoding is selected.
     *
     * Sender divides stream into blocks of N source (media) packets, and adds M repair
     * (redundancy) packets to each block, where N is \c fec_block_source_packets and M
     * is \c fec_block_repair_packets.
     *
     * Larger number of repair packets in block increases robustness (repair ratio), but
     * also increases traffic. Number of repair packets usually should be 1/2 or 2/3 of
     * the number of source packets.
     *
     * If zero, default value is used.
     */
    unsigned int fec_block_repair_packets;

    /** Clock source to use.
     * Defines whether write operation will be blocking or non-blocking.
     * If zero, default value is used (\c ROC_CLOCK_SOURCE_EXTERNAL).
     */
    roc_clock_source clock_source;

    /** Resampler backend to use.
     * If zero, default value is used.
     */
    roc_resampler_backend resampler_backend;

    /** Resampler profile to use.
     * If non-zero, the sender employs resampler if the frame sample rate differs
     * from the packet sample rate.
     */
    roc_resampler_profile resampler_profile;
} roc_sender_config;

/** Receiver configuration.
 *
 * For most fields, zero value means "use default", and you can memset() this struct
 * with zeros and then set only a few fields that don't have defaults. It is safe to
 * use memcpy() to get a copy of config, the struct is flat.
 *
 * \see roc_receiver
 */
typedef struct roc_receiver_config {
    /** The encoding used in frames returned by receiver.
     * Frame encoding defines sample format, channel layout, and sample rate in local
     * frames returned by receiver to user.
     * Should be set (zero value is invalid).
     */
    roc_media_encoding frame_encoding;

    /** Clock source.
     * Defines whether read operation will be blocking or non-blocking.
     * If zero, \ref ROC_CLOCK_SOURCE_EXTERNAL is used.
     */
    roc_clock_source clock_source;

    /** Clock synchronization backend.
     * Defines how sender and receiver clocks are synchronized.
     * If zero, default value is used.
     */
    roc_clock_sync_backend clock_sync_backend;

    /**  Clock synchronization profile.
     * Defines what latency and network jitter are tolerated.
     * If zero, default value is used.
     */
    roc_clock_sync_profile clock_sync_profile;

    /** Resampler backend.
     * Affects CPU usage, quality, and clock synchronization precision.
     * If zero, default value is used.
     */
    roc_resampler_backend resampler_backend;

    /** Resampler profile.
     * Affects CPU usage and quality.
     * If zero, default value is used.
     */
    roc_resampler_profile resampler_profile;

    /** Target latency, in nanoseconds.
     * The session will not start playing until it accumulates the requested latency.
     * Then, if clock synchronization is enabled, the session will adjust its clock to
     * keep actual latency as close as possible to the target latency.
     * If zero, default value is used.
     */
    unsigned long long target_latency;

    /** Maximum allowed delta between current and target latency, in nanoseconds.
     * If session latency differs from the target latency by more than given value, the
     * session is terminated (it can then automatically restart). Receiver itself is
     * not terminated; if there are no sessions, it will produce zeros.
     * If zero, default value is used.
     */
    unsigned long long latency_tolerance;

    /** Timeout for the lack of playback, in nanoseconds.
     * If there is no playback during this period, the session is terminated (it can
     * then automatically restart). Receiver itself is not terminated; if there are
     * no sessions, it will produce zeros.
     * This mechanism allows to detect dead, hanging, or incompatible clients that
     * generate unparseable packets.
     * If zero, default value is used. If negative, the timeout is disabled.
     */
    long long no_playback_timeout;

    /** Timeout for choppy playback, in nanoseconds.
     * If there is constant stuttering during this period, the session is terminated (it
     * can then automatically restart). Receiver itself is not terminated; if there are
     * no sessions, it will produce zeros.
     * This mechanism allows to detect situations when playback continues but there
     * are frequent glitches, for example because there is a high ratio of late packets.
     * If zero, default value is used. If negative, the timeout is disabled.
     */
    long long choppy_playback_timeout;
} roc_receiver_config;

/** Interface configuration.
 *
 * Sender and receiver can have multiple slots (\ref roc_slot), and each slot can
 * be bound or connected to multiple interfaces (\ref roc_interface).
 *
 * Each such interface has its own configuration, defined by this struct.
 *
 * For all fields, zero value means "use default". If you want to set all options
 * to default values, you can memset() this struct with zeros.
 *
 * \see roc_sender_configure(), roc_receiver_configure().
 */
typedef struct roc_interface_config {
    /** Outgoing IP address.
     *
     * If non-empty, explicitly identifies the OS network interface, by its IP address,
     * from which to send outgoing packets. If NULL, the network interface is selected
     * automatically by the OS, depending on the address of remote endpoint.
     *
     * For example, if eth0 has IP address "192.168.0.1", then setting outgoing address
     * to "192.168.0.1" will force usage of eth0 interface.
     *
     * Setting it to `0.0.0.0` (for IPv4) or to `::` (for IPv6) gives the same effect
     * as if it was NULL.
     *
     * By default, empty.
     */
    char outgoing_address[48];

    /** Multicast group IP address.
     *
     * Multicast group should be set only when binding interface to an endpoint with
     * multicast IP address. If present, it defines an IP address of the OS network
     * interface on which to join the multicast group. If not present, no multicast
     * group is joined.
     *
     * It's possible to receive multicast traffic from only those OS network interfaces,
     * on which the process has joined the multicast group. When using multicast, the
     * user should either set this field, or join multicast group manually using
     * OS-specific API.
     *
     * It is allowed to set multicast group to `0.0.0.0` (for IPv4) or to `::` (for IPv6),
     * to be able to receive multicast traffic from all available interfaces. However,
     * this may not be desirable for security reasons.
     *
     * By default, empty.
     */
    char multicast_group[48];

    /** Socket address reuse flag.
     *
     * When true (non-zero), SO_REUSEADDR is enabled for socket, regardless of socket
     * type, unless binding to ephemeral port (when port is set to zero).
     *
     * When false (zero), SO_REUSEADDR is enabled for socket only if it has multicast
     * type, unless binding to ephemeral port (when port is set to zero).
     *
     * For TCP-based protocols, SO_REUSEADDR allows immediate reuse of recently closed
     * socket in TIME_WAIT state, which may be useful you want to be able to restart
     * server quickly.
     *
     * For UDP-based protocols, SO_REUSEADDR allows multiple processes to bind to the
     * same address, which may be useful if you're using socket activation mechanism.
     *
     * By default, false.
     */
    int reuse_address;
} roc_interface_config;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // ROC_CONFIG_H_
