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
 * Utils header support
 */

#ifndef CART_UTILS_H_
#define CART_UTILS_H_

bool GetStoreKeyValuePairByIndex(uint32_t* key_value_store_,
                                 uint32_t key_value_store_size_,
                                 size_t index,
                                 const char** key,
                                 const char** value);
uint32_t DecodeUnsignedLeb128(const uint8_t** data);
size_t GenerateOatDexFilename(const uint8_t* file, const uint8_t* isa, uint8_t* out, size_t len);
size_t GenerateOat2DexCmd(const uint8_t* file, const uint8_t* isa, uint8_t* out, size_t len);
bool IsFileExist(const uint8_t* file);

#ifdef CART_DEBUG
void DumpData(const uint32_t* ptr, uint32_t len, uint32_t label);
#else
static inline void DumpData(const uint32_t* ptr, uint32_t len, uint32_t label) {}
#endif

#endif  // CART_UTILS_H_

