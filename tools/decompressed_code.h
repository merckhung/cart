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
 * Decompress code support
 */

#ifndef CART_RUNTIME_DECOMPRESSED_CODE_H_
#define CART_RUNTIME_DECOMPRESSED_CODE_H_

#include <string>
#include <map>

#if 0
#include "base/macros.h"
#include "globals.h"
#include "os.h"
#include "mem_map.h"
#endif

#define MB  (1024 * 1024)
#define kPageSize 4096

namespace cart {

static constexpr size_t ChunkSize = 1 * MB;
static constexpr size_t MaxNrChunk = 10 * MB;
static constexpr size_t SlotSize = 64;
static constexpr uint32_t BitsPerByte = 8;

struct DcRecord {
  explicit DcRecord()
    : hash_(0),
      index_(0),
      nr_(0) {
  }
  DcRecord(size_t hash, uint32_t index, uint16_t nr)
    : hash_(hash),
      index_(index),
      nr_(nr) {
  }
  ~DcRecord() {}

  size_t GetHash() {
    return hash_;
  }
  uint32_t GetIndex() {
    return index_;
  }
  uint32_t GetNr() {
    return nr_;
  }

 private:
  size_t hash_;
  uint32_t index_;
  uint32_t nr_;
};

struct DcHeader {
  explicit DcHeader()
    : chunk_size_(0),
      nr_chunk_(0),
      nr_record_(0),
      sz_bitmap_(0) {
    SetMagicVersion();
  }
  DcHeader(uint32_t chunk_size,
           uint32_t nr_chunk,
           uint32_t nr_record,
           uint32_t  sz_bitmap)
    : chunk_size_(chunk_size),
      nr_chunk_(nr_chunk),
      nr_record_(nr_record),
      sz_bitmap_(sz_bitmap) {
    SetMagicVersion();
  }
  ~DcHeader() {}

  ////////////////////////////////////////////
  // File format of .coat_cache
  // DcHeader          (Fixed length)
  // Bitmap data       (variant length)
  // Decompressed code (ChunkSize * NrChunk)
  // Record data       (variant length)
  ////////////////////////////////////////////
  uint32_t GetOffsetToBitmapData() {
    return sizeof(DcHeader);
  }
  uint32_t GetOffsetToDecompressedCode() {
    return sizeof(DcHeader) + sz_bitmap_;
  }
  uint32_t GetOffsetToRecordData() {
    return sizeof(DcHeader) + sz_bitmap_ + (chunk_size_ * nr_chunk_);
  }
  uint32_t GetOffsetToEnd() {
    return GetOffsetToRecordData() + (sizeof(DcRecord) * nr_record_);
  }
  uint32_t GetChunkSize() {
    return chunk_size_;
  }
  uint32_t GetNrChunk() {
    return nr_chunk_;
  }
  uint32_t GetNrRecord() {
    return nr_record_;
  }
  uint32_t GetSzBitmap() {
    return sz_bitmap_;
  }
  void SetChunkSize(uint32_t val) {
    chunk_size_ = val;
  }
  void SetNrChunk(uint32_t val) {
    nr_chunk_ = val;
  }
  void SetNrRecord(uint32_t val) {
    nr_record_ = val;
  }
  void SetSzBitmap(uint32_t val) {
    sz_bitmap_ = val;
  }
  size_t GetMemoryMapSize() {
    return sizeof(DcHeader) + sz_bitmap_ + (chunk_size_ * nr_chunk_);
  }
  void SetMagicVersion() {
    magic_[0] = 'C';
    magic_[1] = 'C';
    magic_[2] = 'H';
    magic_[3] = 'E';
    version_[0] = '0';
    version_[1] = '3';
    version_[2] = '9';
    version_[3] = 0x00;
  }
  bool IsValid() {
    if ((magic_[0] != 'C')
        || (magic_[1] != 'C')
        || (magic_[2] != 'H')
        || (magic_[3] != 'E')) {
      return false;
    }
    if ((version_[0] != '0')
        || (version_[1] != '3')
        || (version_[2] != '9')
        || (version_[3] != 0x00)) {
      return false;
    }
    if ((chunk_size_ < 1)
        || (nr_chunk_ < 1)
        || (sz_bitmap_ < 1)) {
      return false;
    }
    return true;
  }

 private:
  uint8_t magic_[4];
  uint8_t version_[4];
  uint32_t chunk_size_;
  uint32_t nr_chunk_;
  uint32_t nr_record_;
  uint32_t sz_bitmap_;
};

class DecompressedCode {
 public:
  static DecompressedCode* Create(const std::string& name);
  static DecompressedCode* LoadFile(const std::string& name);

  ~DecompressedCode();

  void* Alloc(const size_t hash, const size_t size);
  void Dealloc(const size_t hash);
  void* GetPtr(const size_t hash);
  void DumpBitmap();
  void DumpRecord();
  bool IsReady();
  std::string GetName() const {
    return name_;
  }

 private:
  explicit DecompressedCode(const int32_t fd,
                            const std::string& name,
                            void* mem,
                            const size_t mem_sz);
  bool SaveRecordsToFile();
  bool LoadRecordsFromFile();

  const std::string& name_;
  int32_t fd_;
  // std::unique_ptr<File> file_;
  void* Mem_;
  size_t MemSz_;
  // MemMap* mem_map_;
  DcHeader* header_;
  uint8_t* code_;
  uint8_t* bitmap_;
  uint32_t sz_bitmap_;
  std::map<size_t, struct DcRecord*> records_;

  // DISALLOW_COPY_AND_ASSIGN(DecompressedCode);
};

}  // namespace cart

#endif  // CART_RUNTIME_DECOMPRESSED_CODE_H_

