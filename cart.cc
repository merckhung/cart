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
#include "jni_env_ext.h"
#include "java_vm_ext.h"

#include "utils.h"
#include "oat.h"
#include "elf.h"
#include "heap.h"
#include "hash.h"
#include "debugger.h"
#include "cart.h"
#include "class.h"

///////////////////////////////////////////////////////////////////////////////
// Java Virtual Machine Runtime                                              //
///////////////////////////////////////////////////////////////////////////////

static pthread_t pth;

jint JNI_GetDefaultJavaVMInitArgs(void* /*vm_args*/) {
  return JNI_ERR;
}

jint JNI_GetCreatedJavaVMs(JavaVM** vms, jsize, jsize* vm_count) {
  *vm_count = 1;
  vms[0] = NULL;
  return JNI_OK;
}

jint JNI_CreateJavaVM(JavaVM** p_vm, JNIEnv** p_env, void* vm_args) {
  const JavaVMInitArgs* args = (JavaVMInitArgs*)(vm_args);
  JavaVMOption* option = NULL;
  uint8_t isa[] = "x86";
  cart::JavaVMExt* java_vm_ = NULL;
  cart::JNIEnvExt* jni_env_ = NULL;

  // Handle input options
  const char* cp = NULL;
  const char* bootclasspath = NULL;
  for (int32_t i = 0; i < args->nOptions; ++i) {
    option = &args->options[i];
    if (!strcmp(option->optionString, "-cp")) {
      option = &args->options[i + 1];
      cp = option->optionString;
    } else if (!strncmp(option->optionString, "-Xbootclasspath:", strlen("-Xbootclasspath:"))) {
      bootclasspath = option->optionString;
      bootclasspath += strlen("-Xbootclasspath:");
    }
  }
  if (!bootclasspath || !cp) {
    fprintf(stderr, "Must specify both -Xbootclasspath and -cp\n");
    return JNI_ERR;
  }

  // Test the DEX JAR file
  if (IsFileExist((const uint8_t*)bootclasspath) == false) {
    pdbg("Failed to open -Xbootclasspath file: %s\n", cp);
    return JNI_ERR;
  }
  if (IsFileExist((const uint8_t*)cp) == false) {
    pdbg("Failed to open -cp file: %s\n", cp);
    return JNI_ERR;
  }

  // Obtain OatDex file paths
  uint8_t bootcp_oatdex[PATH_MAX];
  GenerateOatDexFilename((const uint8_t*)bootclasspath, isa, bootcp_oatdex, sizeof(bootcp_oatdex));
  uint8_t cp_oatdex[PATH_MAX];
  GenerateOatDexFilename((const uint8_t*)cp, isa, cp_oatdex, sizeof(cp_oatdex));

  // Call oat2dex command to convert DEX into OATDEX
  uint8_t cmd[PATH_MAX];
  GenerateOat2DexCmd((const uint8_t*)bootclasspath, isa, cmd, sizeof(cmd));
  system((const char*)cmd);
  GenerateOat2DexCmd((const uint8_t*)cp, isa, cmd, sizeof(cmd));
  system((const char*)cmd);

  // Create Java virtual machine
  java_vm_ = new cart::JavaVMExt();
  jni_env_ = java_vm_->GetJniEnvExt();
  if (java_vm_->IsReady() == false) {
    pdbg("Failed to create a Java VM instance\n");
    return JNI_ERR;
  }

  // Load classes from the OatDex files (bootcp & cp)
  // LoadClassesOfOatDexFile(jni_env_->GetOatDexFiles(), jni_env_->GetClassLinker(), (const uint8_t*)bootcp_oatdex);
  LoadClassesOfOatDexFile(jni_env_->GetOatDexFiles(), jni_env_->GetClassLinker(), (const uint8_t*)cp_oatdex);

  // Heap information
  // DumpHeap(pClassLinker->heap_vol);
  pdbg("HEAP: avail=%u, alloced=%u free=%u\n",
       (unsigned int)HeapAvailableSize(jni_env_->GetClassLinker()->heap_vol),
       (unsigned int)HeapAllocedSize(jni_env_->GetClassLinker()->heap_vol),
       (unsigned int)HeapFreeSize(jni_env_->GetClassLinker()->heap_vol));

#if 0
  // Start debugger
  StartDebuggerThread(&pth);
  for (;;) {}
#endif

  // Prepare for returning
  *p_vm = java_vm_;
  *p_env = jni_env_;

  // Return
  return JNI_OK;
}
