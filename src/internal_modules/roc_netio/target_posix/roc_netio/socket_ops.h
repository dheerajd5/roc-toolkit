/*
 * Copyright (c) 2020 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_netio/target_posix/roc_netio/socket_ops.h
//! @brief Socket operations.

#ifndef ROC_NETIO_SOCKET_OPS_H_
#define ROC_NETIO_SOCKET_OPS_H_

#include "roc_address/socket_addr.h"
#include "roc_core/stddefs.h"
#include "roc_netio/io_error.h"
#include "roc_netio/socket_options.h"

namespace roc {
namespace netio {

//! Socket type.
enum SocketType {
    SocketType_Tcp, //!< TCP socket.
    SocketType_Udp  //!< UDP socket.
};

//! Platform-specific socket handle.
typedef int SocketHandle;

//! Invalid socket handle.
const SocketHandle SocketInvalid = -1;

//! Create non-blocking socket.
bool socket_create(address::AddrFamily family, SocketType type, SocketHandle& new_sock);

//! Accept incoming connection.
bool socket_accept(SocketHandle sock,
                   SocketHandle& new_sock,
                   address::SocketAddr& remote_address);

//! Set socket options.
bool socket_setup(SocketHandle sock, const SocketOptions& options);

//! Bind socket to local address.
bool socket_bind(SocketHandle sock, address::SocketAddr& local_address);

//! Start listening for incoming connections.
bool socket_listen(SocketHandle sock, size_t backlog);

//! Initiate connecting to remote peer.
//! @returns true if connection was successfully initiated.
//! Sets @p completed_immediately to true if connection was established
//! immediately and there is no need to wait for it.
bool socket_begin_connect(SocketHandle sock,
                          const address::SocketAddr& remote_address,
                          bool& completed_immediately);

//! Finish connecting to remote peer.
//! @returns true if connection was successfully established.
bool socket_end_connect(SocketHandle sock);

//! Try to read bytes from socket without blocking.
//! @returns number of bytes read (>= 0) or IOError (< 0).
ssize_t socket_try_recv(SocketHandle sock, void* buf, size_t bufsz);

//! Try to write bytes to socket without blocking.
//! @returns number of bytes written (>= 0) or IOError (< 0).
ssize_t socket_try_send(SocketHandle sock, const void* buf, size_t bufsz);

//! Try to send datagram via socket to given address, without blocking.
//! @returns number of bytes written (>= 0) or IOError (< 0).
ssize_t socket_try_send_to(SocketHandle sock,
                           const void* buf,
                           size_t bufsz,
                           const address::SocketAddr& remote_address);

//! Gracefully shutdown connection.
bool socket_shutdown(SocketHandle sock);

//! Close socket.
bool socket_close(SocketHandle sock);

//! Close socket and send reset to remote peer.
//! Remote peer will get error when reading from connection.
bool socket_close_with_reset(SocketHandle sock);

} // namespace netio
} // namespace roc

#endif // ROC_NETIO_SOCKET_OPS_H_
