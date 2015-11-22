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

#ifndef CART_JNI_ENV_EXT_H_
#define CART_JNI_ENV_EXT_H_

#include "jni.h"
#include "hash.h"
#include "class.h"

namespace cart {

struct JNIEnvExt : public JNIEnv {
  JNIEnvExt();
  ~JNIEnvExt();

  HashTable_t* GetOatDexFiles() const;
  ClassLinker_t* GetClassLinker() const;
  bool IsReady() const;

 private:
  HashTable_t* oatdex_files_;
  ClassLinker_t* class_linker_;
};

}  // namespace cart

#endif  // CART_JNI_ENV_EXT_H_

