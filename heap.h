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
 * Heap header support
 */

#ifndef CART_HEAP_H_
#define CART_HEAP_H_

#include "hash.h"

#define HEAP_RECORD_BITS   20
#define BITS_PER_BYTE      8

typedef struct PACKED _HeapEntry {
  _HeapEntry*   Next;
  uint8_t*      bitmap;
  size_t        bitmap_sz;
  size_t        avail_sz;
  size_t        alloced_sz;
  void*         ptr;
  HashTable_t*  records;
} HeapEntry_t;

typedef struct PACKED {
  size_t       nr;
  size_t       total_sz;
  HeapEntry_t* ptr;
} HeapVolume_t;

HeapVolume_t* AllocHeapVolume(size_t sz);
void FreeHeapVolume(HeapVolume_t* pHeapVol);
void* HeapAlloc(HeapVolume_t* pHeapVol, size_t sz);
void HeapFree(HeapVolume_t* pHeapVol, void* ptr);
size_t HeapAvailableSize(HeapVolume_t* pHeapVol);
size_t HeapAllocedSize(HeapVolume_t* pHeapVol);
size_t HeapFreeSize(HeapVolume_t* pHeapVol);

HashTable_t* AllocHashTableFromHeap(HeapVolume_t* pHeapVol, size_t nr);

#ifdef CART_DEBUG
void DumpHeap(HeapVolume_t* pHeapVol);
#else
static inline void DumpHeap(HeapVolume_t* pHeapVol) {}
#endif

#endif  // CART_HEAP_H_

