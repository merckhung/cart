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
 * Hash header support
 */

#ifndef CART_HASH_H_
#define CART_HASH_H_

#include "macros.h"

typedef struct PACKED {
  size_t key;
  void* ptr;
} HashEntry_t;

typedef struct PACKED _HashTable {
  struct _HashTable *Next;
  size_t            nr;
  size_t            cnt;
  void*             pHeapVol;
  HashEntry_t*      ptr;
} HashTable_t;

HashTable_t* AllocHashTable(void* pHeapVol, size_t nr);
void FreeHashTable(HashTable_t* tbl);
size_t GenHashKey(const uint8_t* string);
size_t GenHashKeyLen(const uint8_t* string, size_t len);
HashEntry_t* IsHashExist(HashTable_t* tbl, size_t hash);
HashEntry_t* InsertHashEntry(HashTable_t* tbl, size_t hash, void* ptr);
void* RemoveHashEntry(HashTable_t* tbl, size_t hash);

#ifdef CART_DEBUG
void DumpHashTable(HashTable_t* tbl);
#else
static inline void DumpHashTable(HashTable_t* tbl) {}
#endif

#endif  // CART_HASH_H_

