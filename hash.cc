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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "hash.h"
#include "heap.h"

static size_t _ComputeHashMemSize(size_t nr) {
  return (sizeof(HashEntry_t) * nr) + sizeof(HashTable_t);
}

//
// Memory layout of HashTable_t:
//   ----------------------
//   Hash table header of 0 --> *Next: Hash table of 1
//   ----------------------
//   Hash entry 0
//   Hash entry 1
//   ....
//   ....
//   Hash entry n
//   ----------------------
//
HashTable_t* AllocHashTable(void* pHeapVol, size_t nr) {
  uint8_t* p;
  HashTable_t* pHashTbl;
  size_t sz = _ComputeHashMemSize(nr);
  HeapVolume_t* ppHeapVol = (HeapVolume_t*)pHeapVol;

  // Allocate memory and clean up
  if (ppHeapVol) {
    p = (uint8_t*)HeapAlloc(ppHeapVol, sz);
  } else {
    p = (uint8_t*)malloc(sz);
  }
  if (!p) {
    return NULL;
  }
  memset(p, 0, sz);

  // Initialize data
  pHashTbl = (HashTable_t*)p;
  pHashTbl->nr = nr;
  pHashTbl->ptr = (HashEntry_t*)(p + sizeof(HashTable_t));
  pHashTbl->pHeapVol = ppHeapVol;

  // Return
  return pHashTbl;
}

void FreeHashTable(HashTable_t* tbl) {
  HashTable_t* tmp = tbl;
  // Free the list list
  while (tbl) {
    tmp = tbl;
    tbl = tbl->Next;
    if (tmp->pHeapVol) {
      HeapFree((HeapVolume_t*)tmp->pHeapVol, (void*)tmp);
    } else {
      free((void*)tmp);
    }
  }
}

size_t GenHashKey(const uint8_t* string) {
  size_t hash = 0;
  for (; *string != '\0'; ++string) {
    hash = hash * 31 + *string;
  }
  return hash;
}

size_t GenHashKeyLen(const uint8_t* string, size_t len) {
  size_t hash = 0;
  for (;; ++string, --len) {
    if (*string == '\0') {
      break;
    } else if (!len) {
      break;
    }
    hash = hash * 31 + *string;
  }
  return hash;
}

HashEntry_t* IsHashExist(HashTable_t* tbl, size_t hash) {
  HashTable_t* p;
  HashEntry_t* e;
  // Iterate each table
  for (p = tbl; p; p = p->Next) {
    // Iterate each entry
    for (size_t i = 0; i < p->nr; ++i) {
      e = p->ptr + i;
      if (e->key == 0) {
        // If key == 0, means it's not assigned
        continue;
      } else if (e->key == hash) {
        // If the key matches, return the entry
        return e;
      }
    }
  }
  // Doesn't exist, return NULL
  return NULL;
}

HashEntry_t* InsertHashEntry(HashTable_t* tbl, size_t hash, void* ptr) {
  HashTable_t* p;
  HashEntry_t* e = IsHashExist(tbl, hash);
  bool found = false;
  // If it exists, return NULL
  if (e != NULL) {
    return NULL;
  }
 Again:
  // Iterate each table
  for (p = tbl; p; p = p->Next) {
    // Iterate each entry
    for (size_t i = 0; i < p->nr; ++i) {
      e = p->ptr + i;
      if (e->key == 0) {
        // If key == 0, means it's available
        found = true;
        p->cnt++;
        break;
      }
    }
  }
  // Check
  if (found == false) {
    pdbg("Extending hash table\n");
    // Iterate to the last one
    for (p = tbl; p->Next; p = p->Next) {}
    // Allocate a new one and append it
    if (p->pHeapVol) {
      p->Next = AllocHashTable(p->pHeapVol, p->nr);
    } else {
      p->Next = AllocHashTable(NULL, p->nr);
    }
    if (!p->Next) {
      pdbg("Out of memory\n");
      return NULL;
    }
    goto Again;
  }
  // Insert
  e->key = hash;
  e->ptr = ptr;
  // Return
  return e;
}

void* RemoveHashEntry(HashTable_t* tbl, size_t hash) {
  HashTable_t* p;
  HashEntry_t* e;
  // Iterate each table
  for (p = tbl; p; p = p->Next) {
    // Iterate each entry
    for (size_t i = 0; i < p->nr; ++i) {
      e = p->ptr + i;
      if (e->key == 0) {
        // If key == 0, means it's not assigned
        continue;
      } else if (e->key == hash) {
        // If the key matches, erase the entry
        p->cnt--;
        e->key = 0;
        e->ptr = (void*)0;
      }
    }
  }
  // Doesn't exist, return NULL
  return NULL;
}

#ifdef CART_DEBUG
void DumpHashTable(HashTable_t* tbl) {
  HashTable_t* p;
  HashEntry_t* e;
  uint32_t idx = 0;
  uint32_t cel;
  fprintf(stderr, "---------------------------------------------------------------------------\n");
  fprintf(stderr, " HASH table dumping\n");
  fprintf(stderr, "---------------------------------------------------------------------------\n");
  // Iterate each table
  for (p = tbl; p; p = p->Next, ++idx) {
    fprintf(stderr, "Chaining No.          : %d\n", idx);
    fprintf(stderr, "Number of amount cell : %u\n", (unsigned int)p->nr);
    fprintf(stderr, "Number of used   cell : %u\n", (unsigned int)p->cnt);
    // Iterate each entry
    cel = 0;
    for (size_t i = 0; i < p->nr; ++i, ++cel) {
      e = p->ptr + i;
      if (e->key == 0) {
        // If key == 0, means it's not assigned
        continue;
      } else {
        fprintf(stderr, " [%d][%d] Key[0x%8.8X] = %p\n", idx, cel,
                (unsigned int)e->key,
                e->ptr);
      }
    }
  }
  fprintf(stderr, "---------------------------------------------------------------------------\n");
}
#endif
