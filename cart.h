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

#ifndef CART_CART_H_
#define CART_CART_H_

// System
#define PAGE_SIZE         4096
#define HEAP_START_SIZE   (16 * 1024)

// Heap
#define SZ_SLOT           16
#define NR_HASH_TBL       64

// Class
#define CLASSES_HASH_NR   64

// Debugger
#define DEBUGGER_PORT     9500

#endif  // CART_CART_H_

