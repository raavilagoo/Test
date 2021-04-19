/*
 * COBS.h
 *
 *  Created on: May 17, 2020
 *      Author: Ethan Li
 *
 *  Import of the COBS algorithm from Christopher Baker's PacketSerial project.
 *
 *  Copyright (c) 2011 Christopher Baker <https://christopherbaker.net>
 *  Copyright (c) 2011 Jacques Fortier
 * <https://github.com/jacquesf/COBS-Consistent-Overhead-Byte-Stuffing> SPDX-License-Identifier: MIT
 *
 *  MIT License
 *  Copyright (c) 2017 Christopher Baker https://christopherbaker.net
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions: The above copyright notice and this
 * permission notice shall be included in all copies or substantial portions of the Software. THE
 * SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "Pufferfish/Util/Vector.h"
namespace Pufferfish::Util {

/// \brief A Consistent Overhead Byte Stuffing (COBS) Encoder.
///
/// Consistent Overhead Byte Stuffing (COBS) is an encoding that removes all 0
/// bytes from arbitrary binary data. The encoded data consists only of bytes
/// with values from 0x01 to 0xFF. This is useful for preparing data for
/// transmission over a serial link (RS-232 or RS-485 for example), as the 0
/// byte can be used to unambiguously indicate packet boundaries. COBS also has
/// the advantage of adding very little overhead (at least 1 byte, plus up to an
/// additional byte per 254 bytes of data). For messages smaller than 254 bytes,
/// the overhead is constant.
///
/// \sa http://conferences.sigcomm.org/sigcomm/1997/papers/p062.pdf
/// \sa http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
/// \sa https://github.com/jacquesf/COBS-Consistent-Overhead-Byte-Stuffing
/// \sa http://www.jacquesf.com/2011/03/consistent-overhead-byte-stuffing

/// \brief Encode a byte buffer with the COBS encoder.
/// \param buffer A ByteVector to the unencoded buffer to encode.
/// \param encodedBuffer The ByteVector for the encoded bytes.
/// The encoded buffer is resized to fit the encoded data, returns out_of_bounds otherwise
/// \returns IndexStatus as ok/out_of_bounds
template <size_t input_size, size_t output_size>
IndexStatus encode_cobs(
    const Util::ByteVector<input_size> &buffer, Util::ByteVector<output_size> &encoded_buffer);

/// \brief Decode a COBS-encoded buffer.
/// \param encodedBuffer A ByteVector to the \p encodedBuffer to decode.
/// \param decodedBuffer The target ByteVector for the decoded bytes.
/// The decoded buffer is resized to fit the decoded data, returns out_of_bounds otherwise
/// \returns IndexStatus as ok/out_of_bounds
template <size_t input_size, size_t output_size>
IndexStatus decode_cobs(
    const Util::ByteVector<input_size> &encoded_buffer,
    Util::ByteVector<output_size> &decoded_buffer);

/// \brief Get the maximum encoded buffer size for an unencoded buffer size.
/// \param unencodedBufferSize The size of the buffer to be encoded.
/// \returns the maximum size of the required encoded buffer.
constexpr size_t get_encoded_cobs_buffer_size(size_t unencoded_buffer_size);

}  // namespace Pufferfish::Util

#include "COBS.tpp"