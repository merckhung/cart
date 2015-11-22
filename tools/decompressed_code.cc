/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "decompressed_code.h"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <memory>
#include <string>
#include <map>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#if 0
#include "base/logging.h"
#include "base/stl_util.h"
#include "base/unix_file/fd_file.h"
#include "os.h"
#include "runtime.h"
#include "thread.h"
#include "utils.h"
#endif

namespace cart {

static int8_t ConvertDWordToByte(const uint32_t* Data, uint32_t Offset) {
  uint32_t tmp, off, bs;
  off = Offset / 4;
  bs = Offset % 4;
  if (bs) {
    tmp = *(Data + off);
    tmp = ((tmp >> (bs * 8)) & 0xFF);
    return tmp;
  }
  tmp = ((*(Data + off)) & 0xFF);
  return tmp;
}

static void DumpData(const uint32_t* Data, uint32_t Length, uint32_t BaseAddr) {
  uint32_t i, j;
  uint32_t c;

  fprintf(stderr, "---------------------------------------------------------------------------\n");
  fprintf(stderr, " Address | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F|   ASCII DATA   \n");
  fprintf(stderr, "---------------------------------------------------------------------------\n");

  for (i = 0; i <= Length; i++) {
    if (!(i % 16)) {
      if (i > 15) {
        for (j = i - 16; j < i; j++) {
          c = ConvertDWordToByte(Data, j);
          if ((c >= '!') && (c <= '~')) {
            fprintf(stderr, "%c", c);
          } else {
            fprintf(stderr, ".");
          }
        }
      }
      if (i) {
        fprintf(stderr, "\n");
      }
      if (i == Length) {
        break;
      }
      fprintf(stderr, "%8.8X : ", i + BaseAddr);
    }
    fprintf(stderr, "%2.2X ", ConvertDWordToByte(Data, i) & 0xFF);
  }

  if ((i % 16)) {
    for (j = i; j % 16; j++) {
      printf("   ");
    }
    for (j = (i - (i % 16)); j < (i + 16); j++) {
      if (j < i) {
        c = ConvertDWordToByte(Data, j);
        if ((c >= '!') && (c <= '~')) {
          fprintf(stderr, "%c", c);
        } else {
          fprintf(stderr, ".");
        }
      } else {
        fprintf(stderr, " ");
      }
    }
    fprintf(stderr, "\n");
  }

  fprintf(stderr, "---------------------------------------------------------------------------\n");
}

static inline size_t GetFundamentalBitmapSize() {
  return ((ChunkSize / SlotSize) / BitsPerByte);
}

static inline size_t GetFundamentalFileSize() {
  // Header size + bitmap size + chunk(code) size
  return (sizeof(DcHeader) + GetFundamentalBitmapSize() + ChunkSize);
}

DecompressedCode* DecompressedCode::Create(const std::string& name) {
  // Create a new file
  int fd = open(name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0640);
  if (fd < 0) {
    fprintf(stderr, "Cannot create file: %s\n", name.c_str());
    return nullptr;
  }
  // Compute the fundamental file size
  size_t memsz = GetFundamentalFileSize();
  // Set file length
  int32_t rst = ftruncate(fd, memsz);
  if (rst == -1) {
    close(fd);
    return nullptr;
  }
  // Align the file size for memory mapping
  if (memsz % kPageSize) {
    memsz = ((memsz / kPageSize) + 1) * kPageSize;
  }
  // Map the file into memory space
  void* p = mmap(nullptr, memsz, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (p == MAP_FAILED) {
    close(fd);
    return nullptr;
  }
  // Generate a new header
  std::unique_ptr<DcHeader> pDcHdr(new DcHeader(ChunkSize, 1, 0, GetFundamentalBitmapSize()));
  memcpy(p, pDcHdr.get(), sizeof(DcHeader));

  // Manufacture a new instance and return it
  return new DecompressedCode(fd, name, p, memsz);
}

DecompressedCode* DecompressedCode::LoadFile(const std::string& name) {
  // Open the specified file
  int32_t fd = open(name.c_str(), O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Cannot open file: %s\n", name.c_str());
    return nullptr;
  }
  // Load header data and perform a sanity check
  DcHeader bufDcHdr;
  int32_t rbyte = read(fd, &bufDcHdr, sizeof(DcHeader));
  if (rbyte != sizeof(DcHeader)) {
    fprintf(stderr, "Cannot load file: %s\n", name.c_str());
    close(fd);
    return nullptr;
  }
  // Validate the header
  if (bufDcHdr.IsValid() == false) {
    fprintf(stderr, "Invalid header data\n");
    close(fd);
    return nullptr;
  }
  // Compute the fundamental file size
  size_t memsz = bufDcHdr.GetMemoryMapSize();
  // Align the file size for memory mapping
  if (memsz % kPageSize) {
    memsz = ((memsz / kPageSize) + 1) * kPageSize;
  }
  // Map the file into memory space
  void* p = mmap(nullptr, memsz, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (p == MAP_FAILED) {
    close(fd);
    return nullptr;
  }
  // Manufacture a new instance and return it
  return new DecompressedCode(fd, name, p, memsz);
}

bool DecompressedCode::SaveRecordsToFile() {
  // Move the file pointer
  uint32_t sz = lseek(fd_, header_->GetOffsetToRecordData(), SEEK_SET);
  if (sz != header_->GetOffsetToRecordData()) {
    return false;
  }
  // Write out each record to the file
  uint32_t cnt = 0;
  for (auto it : records_) {
    DcRecord* record = it.second;
    sz = write(fd_, reinterpret_cast<void*>(record), sizeof(DcRecord));
    if (sz != sizeof(DcRecord)) {
      return false;
    }
    cnt++;
  }
  // Update the header
  header_->SetNrRecord(cnt);
  // Succeed
  return true;
}

bool DecompressedCode::LoadRecordsFromFile() {
  // Move the file pointer
  uint32_t sz = lseek(fd_, header_->GetOffsetToRecordData(), SEEK_SET);
  if (sz != header_->GetOffsetToRecordData()) {
    return false;
  }
  // Read every record and load it
  uint32_t cnt = header_->GetNrRecord();
  DcRecord bufDcRecord;
  // Clear the map
  records_.clear();
  // Read all records from the file
  for (uint32_t i = 0; i < cnt; ++i) {
    // Read a record from the file
    sz = read(fd_, reinterpret_cast<void*>(&bufDcRecord), sizeof(DcRecord));
    if (sz != sizeof(DcRecord)) {
      return false;
    }
    // Allocate a record
    DcRecord* pDcRecord = new DcRecord(bufDcRecord.GetHash(),
                                       bufDcRecord.GetIndex(),
                                       bufDcRecord.GetNr());
    // Insert it into the map
    records_.insert(std::pair<size_t, DcRecord*>(bufDcRecord.GetHash(), pDcRecord));
  }
  return true;
}

DecompressedCode::DecompressedCode(const int32_t fd,
                                   const std::string& name,
                                   void* mem,
                                   const size_t mem_sz)
  : name_(name),
    fd_(fd),
    Mem_(mem),
    MemSz_(mem_sz) {
  // Get the header
  header_ = reinterpret_cast<DcHeader*>(mem);
  // Load values
  uint8_t* p = reinterpret_cast<uint8_t*>(mem);
  code_ = p + header_->GetOffsetToDecompressedCode();
  bitmap_ = p + header_->GetOffsetToBitmapData();
  sz_bitmap_ = header_->GetSzBitmap();
  // Load saved records from file, if any
  if (header_->GetNrRecord() > 0) {
    LoadRecordsFromFile();
  }
}

DecompressedCode::~DecompressedCode() {
  // Save all records to the file
  SaveRecordsToFile();
  // Unmap the file
  munmap(Mem_, MemSz_);
  // Close the file
  close(fd_);
}

static inline uint8_t NextBit(uint8_t byte) {
  switch (byte) {
    case 0x01:
      return 1;
    case 0x03:
      return 2;
    case 0x07:
      return 3;
    case 0x0F:
      return 4;
    case 0x1F:
      return 5;
    case 0x3F:
      return 6;
    case 0x7F:
      return 7;
    default:
      break;
  }
  return 0x00;
}

void* DecompressedCode::Alloc(const size_t hash, const size_t size) {
  size_t has_size = 0;
  uint32_t start_byte = 0;
  uint32_t start_byte_cnt = 0;
  size_t start_bit = 0;
  size_t start_bit_cnt = 0;
  bool found = false;

  // Search in the bitmap for the needed size of memory
  for (uint32_t i = 0; i < sz_bitmap_; ++i) {
    // Read from memory to a temporary variable
    uint8_t tmp = *(bitmap_ + i);
    if (tmp == 0xFF) {
      continue;
    }
    uint8_t j = NextBit(tmp);
    // Iterate each bit
    for (; j < BitsPerByte; ++j) {
      // If there's already a gotten size suffices the request, just break it
      if (has_size >= size) {
        goto DoneScan;
      }
      // Iterate each bit
      if (tmp & (1 << j)) {
        // If this bit is in use, try next one
        continue;
      }
      // If this bit is not in use, record it
      if (found == false) {
        start_byte = i;
        start_bit = j;
        found = true;
      }
      start_bit_cnt++;
      has_size += SlotSize;
    }
    if (found == true) {
      start_byte_cnt++;
    }
  }
 DoneScan:
  // If (has_size < size), raise Out Of Memory
  if (has_size < size) {
    return nullptr;
  }
  // Mark bits in the bitmap
  uint8_t k = start_bit;
  uint8_t cnt = 0;
  for (uint32_t i = start_byte; i <= (start_byte + start_byte_cnt); ++i, k = 0) {
    // Read from memory to a temporary variable
    uint8_t tmp = *(bitmap_ + i);
    for (; k < BitsPerByte; ++k) {
      // If there's already all marked
      if (cnt >= start_bit_cnt) {
        *(bitmap_ + i) = tmp;
        goto DoneMark;
      }
      // Update the value & counter
      tmp |= (1 << k);
      cnt++;
    }
    // Sync the tmp variable to memory
    *(bitmap_ + i) = tmp;
  }
 DoneMark:
  // Calculate the pointer
  uint8_t* ptr = reinterpret_cast<uint8_t*>(code_) + (((start_byte * BitsPerByte) + start_bit) * SlotSize);
  // Create a record for it
  DcRecord* rec = new DcRecord(hash, (start_byte * BitsPerByte) + start_bit, start_bit_cnt);
  records_.insert(std::pair<size_t, DcRecord*>(hash, rec));
  // Return the address of the pointer
  return reinterpret_cast<void*>(ptr);
}

void DecompressedCode::Dealloc(const size_t hash) {
  auto it = records_.find(hash);
  if (it == records_.end()) {
    return;
  }
  // Retrieve the control parameters from the record
  uint32_t start_byte = it->second->GetIndex() / BitsPerByte;
  uint32_t start_byte_cnt = (it->second->GetNr() % BitsPerByte)
                              ? ((it->second->GetNr() / BitsPerByte) + 1)
                              : (it->second->GetNr() / BitsPerByte);
  size_t start_bit = it->second->GetIndex() % BitsPerByte;
  size_t start_bit_cnt = it->second->GetNr();
  // Unmark bits in the bitmap
  uint8_t k = start_bit;
  uint8_t cnt = 0;
  for (uint32_t i = start_byte; i <= (start_byte + start_byte_cnt); ++i, k = 0) {
    // Read from memory to a temporary variable
    uint8_t tmp = *(bitmap_ + i);
    for (; k < BitsPerByte; ++k) {
      // If there's already all marked
      if (cnt >= start_bit_cnt) {
        *(bitmap_ + i) = tmp;
        goto DoneUnmark;
      }
      // Update the value & counter
      tmp &= ~(1 << k);
      cnt++;
    }
    // Sync the tmp variable to memory
    *(bitmap_ + i) = tmp;
  }
 DoneUnmark:
  // Remove this record from the map
  records_.erase(it);
}

void* DecompressedCode::GetPtr(const size_t hash) {
  auto it = records_.find(hash);
  if (it == records_.end()) {
    return nullptr;
  }
  uint8_t* ptr = reinterpret_cast<uint8_t*>(code_) + (it->second->GetIndex() * SlotSize);
  return reinterpret_cast<void*>(ptr);
}

void DecompressedCode::DumpBitmap() {
  if (bitmap_ != nullptr) {
    DumpData(reinterpret_cast<const uint32_t*>(bitmap_), sz_bitmap_, 0);
  }
}

void DecompressedCode::DumpRecord() {
  for (auto it : records_) {
    DcRecord* record = it.second;
    fprintf(stderr, "HASH = 0x%X, BITMAP_INDEX = %d, NR_BITS = %d\n",
            record->GetHash(),
            record->GetIndex(),
            record->GetNr());
  }
}

bool DecompressedCode::IsReady() {
  if ((fd_ >= 0) && (Mem_ != MAP_FAILED) && (bitmap_ != nullptr)) {
    return true;
  }
  return false;
}

}  // namespace cart
