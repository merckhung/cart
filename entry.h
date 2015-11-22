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
 * Entry header support
 */

#ifndef CART_ENTRY_H_
#define CART_ENTRY_H_

#include "macros.h"
#include "thread.h"

typedef struct PACKED {
  tls_32bit_sized_values_t  tls32_;
  tls_64bit_sized_values_t  tls64_;
  tls_ptr_sized_values_t    tlsPtr_;
} OatCodeExecEnv_t;

void SetupLdt();

#endif  // CART_ENTRY_H_

