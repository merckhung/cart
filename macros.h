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

#ifndef CART_MACROS_H_
#define CART_MACROS_H_

#define PACKED __attribute__((packed))

#ifdef CART_DEBUG
#define pdbg(fmt, ...) fprintf(stderr, \
                               "%s:%d:%s(): " fmt, \
                               __FILE__, \
                               __LINE__, \
                               __func__, \
                               ##__VA_ARGS__)
#else
#define pdbg(fmt, ...)
#endif

#endif  // CART_MACROS_H_

