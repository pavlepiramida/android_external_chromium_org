// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/gpu/media/h264_bit_reader.h"

namespace content {

H264BitReader::H264BitReader() {
}

H264BitReader::~H264BitReader() {}

void H264BitReader::UpdateCurrByte() {
  DCHECK_EQ(num_remaining_bits_in_curr_byte_, 0);

  if (bytes_left_ >= 1) {
    // Emulation prevention three-byte detection.
    // If a sequence of 0x000003 is found, skip (ignore) the last byte (0x03).
    if (*data_ == 0x03 && Tell() >= 16 && data_[-1] == 0 && data_[-2] == 0) {
      // Detected 0x000003, skip last byte.
      ++data_;
      --bytes_left_;
      position_ += 8;
    }
  }

  if (bytes_left_ >= 1) {
    // Load a new byte and advance pointers.
    curr_byte_ = *data_;
    ++data_;
    --bytes_left_;
    num_remaining_bits_in_curr_byte_ = 8;
  }

  // Check if this is the end of RBSP data.
  if (bytes_left_ == 0) {
    while (num_remaining_bits_in_curr_byte_ != 0 && !(curr_byte_ & 0x1)) {
      --num_remaining_bits_in_curr_byte_;
      curr_byte_ >>= 1;
    }
  }
}

bool H264BitReader::HasMoreRBSPData() {
  // Make sure we have more bits, if we are at 0 bits in current byte
  // and updating current byte fails, we don't have more data anyway.
  if (num_remaining_bits_in_curr_byte_ == 0) {
    UpdateCurrByte();
    if (num_remaining_bits_in_curr_byte_ == 0)
      return false;
  }

  // Not on last byte?
  if (bytes_left_)
    return true;

  // Last byte, look for stop bit;
  // We have more RBSP data if the last non-zero bit we find is not the
  // first available bit.
  return (curr_byte_ &
          ((1 << (num_remaining_bits_in_curr_byte_ - 1)) - 1)) != 0;
}

}  // namespace content
