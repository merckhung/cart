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

#ifndef CART_NET_H_
#define CART_NET_H_

#define IP_STR_LEN_MIN 7
#define IP_STR_LEN 15
#define IP_STR_BUF (IP_STR_LEN + 1)
#define PORT_DEF 9600

int32_t isIPv4Format(const int8_t *str);
int32_t initializeSocket(int32_t *fd, int8_t *addr, int32_t port);
int32_t connectSocket(int32_t *fd, int8_t *addr, int32_t port);
void deinitializeSocket(int32_t fd);
int32_t acceptSocket(int32_t fd, int32_t *sd);
int32_t transferSocket(int32_t fd, const void *pktBuf, ssize_t length);
int32_t receiveSocket(int32_t fd, void *pktBuf, ssize_t length);

#endif  // CART_NET_H_

