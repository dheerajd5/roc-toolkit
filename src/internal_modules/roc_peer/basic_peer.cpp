/*
 * Copyright (c) 2020 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "roc_peer/basic_peer.h"

namespace roc {
namespace peer {

BasicPeer::BasicPeer(Context& context)
    : context_(context) {
    context_.incref();
}

BasicPeer::~BasicPeer() {
    context_.decref();
}

Context& BasicPeer::context() {
    return context_;
}

} // namespace peer
} // namespace roc
