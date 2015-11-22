/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * OCAT header support
 */

#ifndef CART_RUNTIME_COAT_H_
#define CART_RUNTIME_COAT_H_

namespace cart {

struct CoatHeader {
  explicit CoatHeader()
    : compressed_size_(0),
      decompressed_size_(0) {
    SetMagicVersion();
  }
  CoatHeader(uint32_t compressed_size,
             uint32_t decompressed_size)
    : compressed_size_(compressed_size),
      decompressed_size_(decompressed_size) {
    SetMagicVersion();
  }
  ~CoatHeader() {}

  /////////////////////////////////////////////////
  // File format of a Compressed OAT(COAT) method
  // CoatHeader             (Fixed length)
  // Compressed method code (variant length)
  /////////////////////////////////////////////////
  uint32_t GetOffsetCompressedCode() {
    return sizeof(CoatHeader);
  }
  uint32_t GetOffsetToEnd() {
    return GetOffsetCompressedCode() + decompressed_size_;
  }
  static uint32_t ComputeCoatPackageSize(size_t compressed_code_size) {
    return sizeof(CoatHeader) + compressed_code_size;
  }
  uint32_t GetCompressedSize() {
    return compressed_size_;
  }
  uint32_t GetDecompressedSize() {
    return decompressed_size_;
  }
  void SetCompressedSize(uint32_t val) {
    compressed_size_ = val;
  }
  void SetDecompressedSize(uint32_t val) {
    decompressed_size_ = val;
  }
  void SetMagicVersion() {
    magic_[0] = 'C';
    magic_[1] = 'O';
    magic_[2] = 'A';
    magic_[3] = 'T';
    version_[0] = '0';
    version_[1] = '3';
    version_[2] = '9';
    version_[3] = 0x00;
  }
  bool IsValidHeader() {
    if ((magic_[0] != 'C')
        || (magic_[1] != 'O')
        || (magic_[2] != 'A')
        || (magic_[3] != 'T')) {
      return false;
    }
    if ((version_[0] != '0')
        || (version_[1] != '3')
        || (version_[2] != '9')
        || (version_[3] != 0x00)) {
      return false;
    }
    if ((compressed_size_ < 1)
        || (decompressed_size_ < 1)) {
      return false;
    }
    return true;
  }

 private:
  uint8_t magic_[4];
  uint8_t version_[4];
  uint32_t compressed_size_;
  uint32_t decompressed_size_;
};

}  // namespace cart

#endif  // CART_RUNTIME_COAT_H_

