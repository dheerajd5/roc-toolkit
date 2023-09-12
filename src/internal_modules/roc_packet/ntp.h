/*
 * Copyright (c) 2022 Roc Streaming authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! @file roc_packet/ntp.h
//! @brief Utitilies for NTP timestamp.

#ifndef ROC_PACKET_NTP_H_
#define ROC_PACKET_NTP_H_

#include "roc_core/time.h"

namespace roc {
namespace packet {

//! NTP timestamp.
//! @remarks
//!  Highest 32 bits - seconds since NTP epoch, lowest 32 bits - fractions of a second.
//!  NTP epoch starts from January 1, 1900.
typedef uint64_t ntp_timestamp_t;

//! Compares a and b if they close enough.
bool ntp_equal_delta(ntp_timestamp_t a, ntp_timestamp_t b, ntp_timestamp_t delta);

//! Convert Unix absolute time to NTP absolute time.
//! @remarks
//!  @p unix_time defines nanoseconds since Unix epoch (should be positive)
ntp_timestamp_t unix_2_ntp(core::nanoseconds_t unix_time);

//! Convert NTP absolute time to Unix absolute time.
core::nanoseconds_t ntp_2_unix(ntp_timestamp_t ntp_time);

//! Convert nanoseconds delta to NTP delta.
//! @remarks
//!  @p ns_delta defines nanoseconds delta (should be positive)
ntp_timestamp_t nanoseconds_2_ntp(core::nanoseconds_t ns_delta);

//! Converts NTP delta to nanoseconds delta.
core::nanoseconds_t ntp_2_nanoseconds(ntp_timestamp_t ntp_delta);

} // namespace packet
} // namespace roc

#endif // ROC_PACKET_NTP_H_
