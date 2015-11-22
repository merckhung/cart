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

#include "jni.h"
#include "java_vm_ext.h"
#include "jni_env_ext.h"

#include "thread.h"
#include "entry.h"

namespace cart {

class JII {
 public:
  static jint DestroyJavaVM(JavaVM* vm) {
    printf("%s\n", __func__);
    JavaVMExt* pJvmE = reinterpret_cast<JavaVMExt*>(vm);
    delete pJvmE;
    return JNI_OK;
  }

  static jint AttachCurrentThread(JavaVM* vm, JNIEnv** p_env, void* thr_args) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jint AttachCurrentThreadAsDaemon(JavaVM* vm, JNIEnv** p_env, void* thr_args) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jint DetachCurrentThread(JavaVM* vm) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jint GetEnv(JavaVM* vm, void** env, jint version) {
    printf("%s\n", __func__);
    JavaVMExt* pJvmE = reinterpret_cast<JavaVMExt*>(vm);
    *env = pJvmE->GetJniEnvExt();
    return JNI_OK;
  }
};

const JNIInvokeInterface gJniInvokeInterface = {
  0,  // reserved0
  0,  // reserved1
  0,  // reserved2
  JII::DestroyJavaVM,
  JII::AttachCurrentThread,
  JII::DetachCurrentThread,
  JII::GetEnv,
  JII::AttachCurrentThreadAsDaemon
};

JavaVMExt::JavaVMExt()
  : jni_env_(NULL) {
  // Setup LDT for OAT code execution environment
  SetupLdt();
  // Create an instance of JNIEnvExt
  jni_env_ = new JNIEnvExt();
  if (jni_env_ == NULL) {
    return;
  }
  // Assign the function pointers
  functions = &gJniInvokeInterface;
}

JavaVMExt::~JavaVMExt() {
  if (jni_env_ != NULL) {
    delete jni_env_;
    jni_env_ = NULL;
  }
}

JNIEnvExt* JavaVMExt::GetJniEnvExt() const {
  return jni_env_;
}

bool JavaVMExt::IsReady() const {
  if ((jni_env_ != NULL) && (jni_env_->IsReady() == true)) {
    return true;
  }
  return false;
}

};  // namespace cart

