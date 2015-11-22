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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "hash.h"
#include "heap.h"
#include "cart.h"

static inline size_t _ComputeBitmapSize(size_t sz) {
  return ((sz / SZ_SLOT) / BITS_PER_BYTE);
}

static inline size_t _ComputeEntryMemSize(size_t mem_sz) {
  return sizeof(HeapVolume_t) + sizeof(HeapEntry_t) + _ComputeBitmapSize(mem_sz) + mem_sz;
}

static inline size_t _ComputeFundmantalMemSize(size_t mem_sz) {
  return sizeof(HeapVolume_t) + _ComputeEntryMemSize(mem_sz);
}

static inline void _FillHeapEntry(HeapEntry_t* pHeapEnt, HashTable_t* pHashTable, size_t sz) {
  uint8_t* p = (uint8_t*)pHeapEnt;
  pHeapEnt->Next = NULL;
  pHeapEnt->alloced_sz = 0;
  pHeapEnt->avail_sz = sz;
  pHeapEnt->bitmap_sz = _ComputeBitmapSize(sz);
  pHeapEnt->bitmap = p + sizeof(HeapEntry_t);
  pHeapEnt->ptr = pHeapEnt->bitmap + pHeapEnt->bitmap_sz;
  pHeapEnt->records = pHashTable;
}

//
// Memory layout of HeapVolume_t:
//   ----------------------
//   Heap volume header
//   ----------------------
//   Heap entry header of 0 --> *Next: Heap entry of 1
//   ----------------------
//   bitmap start
//   ....
//   ....
//   bitmap end
//   ----------------------
//   memory pool start
//   ....
//   ....
//   memory pool end
//   ----------------------
//
HeapVolume_t* AllocHeapVolume(size_t sz) {
  size_t req_sz;
  HeapVolume_t* pHeapVol;
  uint8_t* p;
  HashTable_t* records;

  // Align to the page boundary
  if (sz % PAGE_SIZE) {
    sz = ((sz / PAGE_SIZE) + 1) * PAGE_SIZE;
    pdbg("Align heap volume size to %u\n", (unsigned int)sz);
  }

  // Compute the requesting memory size, and then clean up
  req_sz = _ComputeFundmantalMemSize(sz);
  p = (uint8_t*)malloc(req_sz);
  if (!p) {
    pdbg("Out of memory\n");
    return NULL;
  }
  memset(p, 0, req_sz);
  pHeapVol = (HeapVolume_t*)p;

  // Allocate hash table
  records = AllocHashTable(NULL, NR_HASH_TBL);
  if (!records) {
    pdbg("Out of memory\n");
    free(p);
    return NULL;
  }

  // Initialize data
  pHeapVol->nr = 1;
  pHeapVol->total_sz = sz;
  pHeapVol->ptr = (HeapEntry_t*)(p + sizeof(HeapVolume_t));
  _FillHeapEntry(pHeapVol->ptr, records, sz);

  // Succeed and return
  return pHeapVol;
}

void FreeHeapVolume(HeapVolume_t* pHeapVol) {
  HeapEntry_t* pHeapEnt;
  HeapEntry_t* tmpHeapEnt;

  // Free each subsequent HeapEntry_t, but not the first one
  for (pHeapEnt = pHeapVol->ptr->Next; pHeapEnt;) {
    tmpHeapEnt = pHeapEnt;
    pHeapEnt = pHeapEnt->Next;
    // Free the hash table of records
    FreeHashTable(tmpHeapEnt->records);
    // Free the entry
    free((void*)tmpHeapEnt);
  }
  // Free the first HeapEntry_t and the HeapVolume header
  FreeHashTable(pHeapVol->ptr->records);
  free((void*)pHeapVol);
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

static inline void* HeapRecordEncode(uint32_t start_bits, uint32_t nr_bits) {
#ifdef CART_DEBUG
  uint32_t test = start_bits << HEAP_RECORD_BITS;
  test >>= HEAP_RECORD_BITS;
  if (test != start_bits) {
    pdbg("!!!Overflow of start_bits!!!\n");
  }
  test = 0xFFFFFFFF;
  test >>= HEAP_RECORD_BITS;
  test <<= HEAP_RECORD_BITS;
  if ((nr_bits & ~test) != nr_bits) {
    pdbg("!!!Overflow of nr_bits!!!\n");
  }
#endif
  return (void*)(size_t)((start_bits << HEAP_RECORD_BITS) | nr_bits);
}

static inline uint32_t HeapRecordDecodeStartBits(void* ptr) {
  uint32_t value = (uint32_t)(size_t)ptr;
  value >>= HEAP_RECORD_BITS;
  return value;
}

static inline uint32_t HeapRecordDecodeNrBits(void* ptr) {
  uint32_t value = (uint32_t)(size_t)ptr;
  uint32_t mask = 0xFFFFFFFF;
  mask >>= HEAP_RECORD_BITS;
  mask <<= HEAP_RECORD_BITS;
  value &= ~mask;
  return value;
}

static inline void* _HeapAlloc(HeapEntry_t* pHeapEnt, size_t sz) {
  size_t has_size = 0;
  uint32_t start_byte = 0;
  uint32_t start_byte_cnt = 0;
  size_t start_bit = 0;
  size_t start_bit_cnt = 0;
  bool found = false;

  // Check if available for the allocation
  if ((pHeapEnt->avail_sz - pHeapEnt->alloced_sz) < sz) {
    return NULL;
  }

  // Iterate each entry
  for (; pHeapEnt; pHeapEnt = pHeapEnt->Next) {
    // Search in the bitmap for the needed size of memory
    for (uint32_t i = 0; i < pHeapEnt->bitmap_sz; ++i) {
      // Read from memory to a temporary variable
      uint8_t tmp = *(pHeapEnt->bitmap + i);
      if (tmp == 0xFF) {
        continue;
      }
      uint8_t j = NextBit(tmp);
      // Iterate each bit
      for (; j < BITS_PER_BYTE; ++j) {
        // If there's already a gotten size suffices the request, just break it
        if (has_size >= sz) {
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
        has_size += SZ_SLOT;
      }
      if (found == true) {
        start_byte_cnt++;
      }
    }
 DoneScan:
    // If (has_size < sz), raise Out Of Memory
    if (has_size < sz) {
      // FIXME:
      return NULL;
    }
    // Mark bits in the bitmap
    uint8_t k = start_bit;
    uint8_t cnt = 0;
    for (uint32_t i = start_byte; i <= (start_byte + start_byte_cnt); ++i, k = 0) {
      // Read from memory to a temporary variable
      uint8_t tmp = *(pHeapEnt->bitmap + i);
      for (; k < BITS_PER_BYTE; ++k) {
        // If there's already all marked
        if (cnt >= start_bit_cnt) {
          *(pHeapEnt->bitmap + i) = tmp;
          goto DoneMark;
        }
        // Update the value & counter
        tmp |= (1 << k);
        cnt++;
      }
      // Sync the tmp variable to memory
      *(pHeapEnt->bitmap + i) = tmp;
    }
  }
 DoneMark:
  // Deduct the size from allocated size
  pHeapEnt->alloced_sz += has_size;
  // Calculate the pointer
  uint8_t* ptr = ((uint8_t*)pHeapEnt->ptr) + (((start_byte * BITS_PER_BYTE) + start_bit) * SZ_SLOT);
  // Record this allocation
  InsertHashEntry(pHeapEnt->records,
                  (size_t)ptr,
                  HeapRecordEncode((start_byte * BITS_PER_BYTE) + start_bit, start_bit_cnt));
  // Return the address of the pointer
  return (void*)ptr;
}

void* HeapAlloc(HeapVolume_t* pHeapVol, size_t sz) {
  HeapEntry_t* e = pHeapVol->ptr;
  size_t mem_size;
  void* p;
 Again:
  for (; e; e = e->Next) {
    p = _HeapAlloc(e, sz);
    if (p) {
      // Succeed, return the pointer
      return p;
    }
  }
  pdbg("Growing the heap by appending a new entry\n");
  mem_size = pHeapVol->ptr->avail_sz;
  e = (HeapEntry_t*)malloc(_ComputeEntryMemSize(mem_size));
  if (!e) {
    pdbg("Out of memory\n");
    return NULL;
  }
  // Allocate hash table
  e->records = AllocHashTable(NULL, NR_HASH_TBL);
  if (!e->records) {
    pdbg("Out of memory\n");
    free((void*)e);
    return NULL;
  }
  // Initialize data
  pHeapVol->nr++;
  pHeapVol->total_sz += mem_size;
  _FillHeapEntry(e, e->records, mem_size);
  // Chain this entry
  HeapEntry_t* tmp = pHeapVol->ptr;
  for (; tmp->Next; tmp = tmp->Next) {}
  tmp->Next = e;
  // Resume the heap allocation
  goto Again;
  // Non-reachable
  return NULL;
}

static inline void* _HeapFree(HeapEntry_t* pHeapEnt, void* ptr) {
  HashEntry_t* pHashEntry = IsHashExist(pHeapEnt->records, (size_t)ptr);
  if (!pHashEntry) {
    pdbg("Cannot find pointer(%p) in the heap\n", ptr);
    return NULL;
  }
  // Retrieve the control parameters from the record
  size_t index = (size_t)HeapRecordDecodeStartBits(pHashEntry->ptr);
  size_t nr = (size_t)HeapRecordDecodeNrBits(pHashEntry->ptr);
  uint32_t start_byte = index / BITS_PER_BYTE;
  uint32_t start_byte_cnt = nr / BITS_PER_BYTE;
  if (nr % BITS_PER_BYTE) {
    start_byte_cnt++;
  }
  size_t start_bit = index % BITS_PER_BYTE;
  size_t start_bit_cnt = nr;
  // Unmark bits in the bitmap
  uint8_t k = start_bit;
  uint8_t cnt = 0;
  for (uint32_t i = start_byte; i <= (start_byte + start_byte_cnt); ++i, k = 0) {
    // Read from memory to a temporary variable
    uint8_t tmp = *(pHeapEnt->bitmap + i);
    for (; k < BITS_PER_BYTE; ++k) {
      // If there's already all marked
      if (cnt >= start_bit_cnt) {
        *(pHeapEnt->bitmap + i) = tmp;
        goto DoneUnmark;
      }
      // Update the value & counter
      tmp &= ~(1 << k);
      cnt++;
    }
    // Sync the tmp variable to memory
    *(pHeapEnt->bitmap + i) = tmp;
  }
 DoneUnmark:
  // Deduct the size from allocated size
  pHeapEnt->alloced_sz -= (start_bit_cnt * SZ_SLOT);
  // Remove this record from the map
  return RemoveHashEntry(pHeapEnt->records, (size_t)ptr);
}

void HeapFree(HeapVolume_t* pHeapVol, void* ptr) {
  HeapEntry_t* e = pHeapVol->ptr;
  void* p;
  for (; e; e = e->Next) {
    p = _HeapFree(e, ptr);
    if (p) {
      // Succeed
      return;
    }
  }
}

size_t HeapAvailableSize(HeapVolume_t* pHeapVol) {
  return pHeapVol->total_sz;
}

size_t HeapAllocedSize(HeapVolume_t* pHeapVol) {
  size_t sum = 0;
  HeapEntry_t* e = pHeapVol->ptr;
  for (; e; e = e->Next) {
    sum += e->alloced_sz;
  }
  return sum;
}

size_t HeapFreeSize(HeapVolume_t* pHeapVol) {
  size_t alloced = HeapAllocedSize(pHeapVol);
  return pHeapVol->total_sz - alloced;
}

#ifdef CART_DEBUG
void DumpHeap(HeapVolume_t* pHeapVol) {
  HeapEntry_t* e;
  uint32_t idx = 0;

  fprintf(stderr, "---------------------------------------------------------------------------\n");
  fprintf(stderr, " HEAP table dumping\n");
  fprintf(stderr, "---------------------------------------------------------------------------\n");
  fprintf(stderr, "Number of heap entries: %u\n", (unsigned int)pHeapVol->nr);
  fprintf(stderr, "Total memory capacity : %u\n", (unsigned int)pHeapVol->total_sz);
  for (e = pHeapVol->ptr; e; e = e->Next, ++idx) {
    fprintf(stderr, "  [%d] Entry No.         : %d\n", idx, idx);
    fprintf(stderr, "  [%d] Bitmap size       : %u\n", idx, (unsigned int)e->bitmap_sz);
    fprintf(stderr, "  [%d] Allocated size    : %u\n", idx, (unsigned int)e->alloced_sz);
    fprintf(stderr, "  [%d] Available size    : %u\n", idx, (unsigned int)e->avail_sz);
    fprintf(stderr, "  [%d] Memory start addr : %p\n", idx, e->ptr);
    fprintf(stderr, "  [%d] Dump bitmap START\n", idx);
    DumpData((const uint32_t*)e->bitmap, e->bitmap_sz, 0);
    fprintf(stderr, "  [%d] Dump bitmap END\n", idx);
    fprintf(stderr, "  [%d] Dump record START\n", idx);
    DumpHashTable(e->records);
    fprintf(stderr, "  [%d] Dump record END\n", idx);
  }

  fprintf(stderr, "---------------------------------------------------------------------------\n");
}
#endif
