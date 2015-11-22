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
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "jni.h"
#include "../jni_env_ext.h"
#include "../java_vm_ext.h"

#include "../utils.h"
#include "../oat.h"
#include "../elf.h"
#include "../heap.h"
#include "../hash.h"
#include "../cart.h"
#include "../class.h"
#include "../entry.h"
#include "../thread.h"

static HeapVolume_t* pHeapVolume = NULL;
static HashTable_t* pHashTable = NULL;
static cart::JavaVMExt* java_vm_ = NULL;
static cart::JNIEnvExt* jni_env_ = NULL;

bool CreateJavaVM() {
  uint8_t isa[] = "x86";
  const uint8_t cp[] = "../../Loop.jar";

  putenv((char*)"ANDROID_DATA=/tmp/android-data");
  putenv((char*)"ANDROID_ROOT=/Volumes/DataHD/Projects/android-5.0.2_r1/out/host/darwin-x86");

  if (IsFileExist((const uint8_t*)cp) == false) {
    pdbg("Failed to open -cp file: %s\n", cp);
    return false;
  }

  // Obtain OatDex file path
  uint8_t cp_oatdex[PATH_MAX];
  GenerateOatDexFilename((const uint8_t*)cp, isa, cp_oatdex, sizeof(cp_oatdex));

  // Call oat2dex command to convert DEX into OATDEX
  uint8_t cmd[PATH_MAX];
  GenerateOat2DexCmd((const uint8_t*)cp, isa, cmd, sizeof(cmd));
  system((const char*)cmd);

  // Create Java virtual machine
  java_vm_ = new cart::JavaVMExt();
  jni_env_ = java_vm_->GetJniEnvExt();
  if (java_vm_->IsReady() == false) {
    pdbg("Failed to create a Java VM instance\n");
    return false;
  }

  // Load classes from the OatDex file
  LoadClassesOfOatDexFile(jni_env_->GetOatDexFiles(), jni_env_->GetClassLinker(), (const uint8_t*)cp_oatdex);

  // Heap information
  // DumpHeap(pClassLinker->heap_vol);
  pdbg("HEAP: avail=%u, alloced=%u free=%u\n",
       (unsigned int)HeapAvailableSize(jni_env_->GetClassLinker()->heap_vol),
       (unsigned int)HeapAllocedSize(jni_env_->GetClassLinker()->heap_vol),
       (unsigned int)HeapFreeSize(jni_env_->GetClassLinker()->heap_vol));

  // Return
  return true;
}

int main(int argc, char** argv) {
  uint8_t* p1;
  uint8_t* p2;
  uint8_t* p3;
  uint8_t* p4;

  // Allocate hash table
  pHashTable = AllocHashTable(NULL, 5);
  if (pHashTable) {
    fprintf(stderr, "Succeed to allocate a hash table\n");
  } else {
    fprintf(stderr, "Out of memory for hash table initialization\n");
    return -1;
  }

  // Allocate heap table
  pHeapVolume = AllocHeapVolume(512);
  if (pHeapVolume) {
    fprintf(stderr, "Succeed to allocate a heap, first entry = %p, first loc = %p\n",
            pHeapVolume->ptr,
            pHeapVolume->ptr->ptr);
  } else {
    fprintf(stderr, "Out of memory for heap initialization\n");
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Test HASH
  /////////////////////////////////////////////////////////////////////////////
  InsertHashEntry(pHashTable, 123, (void*)0x12345678);
  InsertHashEntry(pHashTable, 456, (void*)0x12340000);
  InsertHashEntry(pHashTable, 789, (void*)0x00005678);
  InsertHashEntry(pHashTable, 677, (void*)0xEEEEEEEE);
  InsertHashEntry(pHashTable, 984, (void*)0xEEEE0000);

  InsertHashEntry(pHashTable, 156, (void*)0x0000EEEE);
  InsertHashEntry(pHashTable, 186, (void*)0xFFFFFFFF);
  InsertHashEntry(pHashTable, 539, (void*)0x00001234);
  InsertHashEntry(pHashTable, 539, (void*)0x0000567A);
  InsertHashEntry(pHashTable, 539, (void*)0x00123456);
  InsertHashEntry(pHashTable, 539, (void*)0x01234567);

  InsertHashEntry(pHashTable, 982, (void*)0x00123544);
  InsertHashEntry(pHashTable, 456, (void*)0x05345344);
  InsertHashEntry(pHashTable, 354, (void*)0x06546234);
  InsertHashEntry(pHashTable, 897, (void*)0x0654AAA4);
  InsertHashEntry(pHashTable, 211, (void*)0x0654EEEE);
  if (IsHashExist(pHashTable, 211) != NULL) {
    fprintf(stderr, "IsHashExist1: passed\n");
  } else {
    fprintf(stderr, "IsHashExist1: failed\n");
    return -1;
  }
  if (IsHashExist(pHashTable, 42342342) == NULL) {
    fprintf(stderr, "IsHashExist2: passed\n");
  } else {
    fprintf(stderr, "IsHashExist2: failed\n");
    return -1;
  }
  DumpHashTable(pHashTable);

  RemoveHashEntry(pHashTable, 156);
  if (IsHashExist(pHashTable, 156) == NULL) {
    fprintf(stderr, "IsHashExist3: passed\n");
  } else {
    fprintf(stderr, "IsHashExist3: failed\n");
    return -1;
  }
  RemoveHashEntry(pHashTable, 897);
  if (IsHashExist(pHashTable, 897) == NULL) {
    fprintf(stderr, "IsHashExist4: passed\n");
  } else {
    fprintf(stderr, "IsHashExist4: failed\n");
    return -1;
  }
  DumpHashTable(pHashTable);
  FreeHashTable(pHashTable);
  pHashTable = 0;

  /////////////////////////////////////////////////////////////////////////////
  // Test HEAP
  /////////////////////////////////////////////////////////////////////////////
  p1 = (uint8_t*)HeapAlloc(pHeapVolume, 123);
  p2 = (uint8_t*)HeapAlloc(pHeapVolume, 2800);
  p3 = (uint8_t*)HeapAlloc(pHeapVolume, 1335);
  p4 = (uint8_t*)HeapAlloc(pHeapVolume, 512);
  fprintf(stderr, "p1 = %p, size = %u\n", p1, (unsigned int)(p2 - p1));
  fprintf(stderr, "p2 = %p, size = %u\n", p2, (unsigned int)(p3 - p2));
  fprintf(stderr, "p3 = %p, size = %u\n", p3, (unsigned int)(p4 - p3));
  DumpHeap(pHeapVolume);
  fprintf(stderr, "Free p2\n");
  HeapFree(pHeapVolume, p2);
  DumpHeap(pHeapVolume);
  fprintf(stderr, "Free p1\n");
  HeapFree(pHeapVolume, p1);
  DumpHeap(pHeapVolume);
  fprintf(stderr, "Free p3\n");
  HeapFree(pHeapVolume, p3);
  DumpHeap(pHeapVolume);
  fprintf(stderr, "Free p4\n");
  HeapFree(pHeapVolume, p4);
  DumpHeap(pHeapVolume);
  FreeHeapVolume(pHeapVolume);
  pHeapVolume = 0;

  /////////////////////////////////////////////////////////////////////////////
  // Test CART
  /////////////////////////////////////////////////////////////////////////////
  CreateJavaVM();

  jclass klass = jni_env_->FindClass("Loop");
  jmethodID method = jni_env_->GetStaticMethodID(klass, "main", "([Ljava/lang/String;)V");
  jni_env_->CallStaticVoidMethod(klass, method, NULL);

  return 0;
}

