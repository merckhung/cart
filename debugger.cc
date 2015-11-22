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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "macros.h"
#include "utils.h"
#include "net.h"
#include "debugger.h"
#include "cart.h"

static void* DebuggerThread(void* arg) {
  int32_t sts = -1;
  int32_t sfd;
  int32_t sd;
  int8_t packet[PACKET_SZ];

  // Open a socket
  sts = initializeSocket(&sfd, NULL, DEBUGGER_PORT);
  if (sts != 0) {
    pdbg("Debugger cannot open socket\n");
    return NULL;
  }

  while (true) {
    // Wait for new connection
    if (acceptSocket(sfd, &sd) == false) {
      usleep(1000);
      continue;
    }

    // Clean memory and then receive a new coming packet
    memset(packet, 0, sizeof(packet));
    receiveSocket(sd, packet, sizeof(packet));
    DumpData((const uint32_t *)(packet), PACKET_SZ, 0);

    DebuggerPktComm* pDebuggerPktComm = (DebuggerPktComm*)(packet);
    switch (pDebuggerPktComm->op) {
      case OP_SUB_HEAP:
        pdbg("OP_SUB_HEAP OP code\n");
        break;
      case OP_STP_HEAP:
        pdbg("OP_STP_HEAP OP code\n");
        break;
      case OP_GC_HEAP:
        pdbg("OP_GC_HEAP OP code\n");
        break;
      default:
        pdbg("Invalid PACKET OP code\n");
        break;
    }

    DebuggerPktRepHeap_t* pArtdbgPktRepHeap = (DebuggerPktRepHeap_t*)packet;
    while (true) {
      // Update data
      memset(packet, 0, sizeof(packet));
      pArtdbgPktRepHeap->x.rep_heap.op = OP_REP_HEAP;
      pArtdbgPktRepHeap->x.rep_heap.TotalMemory = 100;  // HeapAllocedSize
      pArtdbgPktRepHeap->x.rep_heap.FreeMemory = 90;  // HeapFreeSize
      pArtdbgPktRepHeap->x.rep_heap.MaxMemory = 30;  // HeapAvailableSize
      pArtdbgPktRepHeap->x.rep_heap.FreeMemoryUntilGC = 10;
      pArtdbgPktRepHeap->x.rep_heap.FreeMemoryUntilOOME = 10;
      pArtdbgPktRepHeap->x.rep_heap.BytesAllocated = 11;
      pArtdbgPktRepHeap->x.rep_heap.BytesAllocatedEver = 9;
      pArtdbgPktRepHeap->x.rep_heap.BytesFreedEver = 6;
      // Send it out
      transferSocket(sd, packet, sizeof(packet));
      sleep(2);
    }
  }

  // Close the connection
  deinitializeSocket(sd);
  deinitializeSocket(sfd);
  pdbg("Debugger cleaned up\n");
  return NULL;
}

void StartDebuggerThread(pthread_t* pth) {
  pdbg("StartDebuggerThread entering\n");
  pthread_create(pth, NULL, DebuggerThread, NULL);
  pdbg("StartDebuggerThread exiting\n");
}
