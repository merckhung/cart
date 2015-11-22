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

#ifndef CART_DEBUGGER_H_
#define CART_DEBUGGER_H_

#include <pthread.h>

#define PACKET_SZ 512

enum {
  OP_SUB_HEAP,
  OP_REP_HEAP,
  OP_STP_HEAP,
  OP_GC_HEAP,
  OP_EXIT,
};

typedef struct {
  uint32_t op;
  uint32_t data[0];
} DebuggerPktComm;

typedef struct {
  union {
    DebuggerPktComm comm;
    struct {
      uint32_t op;
      uint32_t TotalMemory;
      uint32_t FreeMemory;
      uint32_t MaxMemory;
      uint32_t FreeMemoryUntilGC;
      uint32_t FreeMemoryUntilOOME;
      uint32_t BytesAllocated;
      uint64_t BytesAllocatedEver;
      uint64_t BytesFreedEver;
    } rep_heap;
  } x;
} DebuggerPktRepHeap_t;

void StartDebuggerThread(pthread_t* pth);

#endif  // CART_DEBUGGER_H_

