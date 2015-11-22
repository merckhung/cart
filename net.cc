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

#include "net.h"

int32_t isIPv4Format(const int8_t *str) {
  int32_t i, j, pos[3], len;
  int8_t buf[IP_STR_BUF];
  uint32_t addr[4];
  int32_t valid = true;

  // Length check
  len = strlen(reinterpret_cast<const char *>(str));
  if (len > IP_STR_LEN || len < IP_STR_LEN_MIN) {
    return false;
  }
  strncpy(reinterpret_cast<char *>(buf), reinterpret_cast<const char *>(str), IP_STR_BUF);

  // Look for positions of delimiters
  for (i = 0, j = 0; i < len; i++) {
    if (buf[ i ] == '.') {
      // Exceed the limit
      if (j >= 3) {
        return false;
      }
      // Record & Terminate string
      pos[j] = i;
      buf[i] = 0;
      j++;
    }
  }

  // Must have 3 dots at least or at most
  if (j != 3) {
    return false;
  }

  // Convert strings
  addr[0] = strtol((const char *)&buf[0], NULL, 10);
  addr[1] = strtol((const char *)&buf[pos[0] + 1], NULL, 10);
  addr[2] = strtol((const char *)&buf[pos[1] + 1], NULL, 10);
  addr[3] = strtol((const char *)&buf[pos[2] + 1], NULL, 10);

  // Check range
  for (i = 0; i < 4; i++) {
    if (addr[i] > 255) {
      valid = false;
      break;
    }
  }

  // Not ZERO
  snprintf(reinterpret_cast<char *>(buf), IP_STR_BUF, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
  if (strcmp(reinterpret_cast<const char *>(buf), reinterpret_cast<const char *>(str))) {
    valid = false;
  }

  return valid;
}

int32_t initializeSocket(int32_t *fd, int8_t *addr, int32_t port) {
  int32_t sts = 0;
  struct sockaddr_in servaddr;
  int8_t def_ip[] = "0.0.0.0";

  // Argument check
  if (!addr) {
    addr = def_ip;  // Any
  } else if (isIPv4Format(addr) == false) {
    return -1;
  }

  // Open a socket
  *fd = socket(PF_INET, SOCK_STREAM, 0);
  if (*fd < 0) {
    return -1;
  }

  // Default values of address & port
  if (port <= 0) {
    port = PORT_DEF;
  }

  // Set address
  memset(&servaddr, 0, sizeof(struct sockaddr_in));
  servaddr.sin_family = PF_INET;
  servaddr.sin_port = htons(port);
  if (inet_aton(reinterpret_cast<const char *>(addr), &servaddr.sin_addr) < 0) {
    sts = -1;
    goto ErrExit;
  }

  // Bind network port
  if (bind(*fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0) {
    sts = -1;
    goto ErrExit;
  }

  // Listen network port
  if (listen(*fd, 5) < 0) {
    sts = -1;
    goto ErrExit;
  }

  // Return socket fd
  return sts;

 ErrExit:
  close(*fd);
  return sts;
}

int32_t connectSocket(int32_t *fd, int8_t *addr, int32_t port) {
  int32_t sts = 0;
  int8_t def_ip[] = "0.0.0.0";
  struct sockaddr_in servaddr;

  // Argument check
  if (!addr) {
    addr = def_ip;  // Any
  } else if (isIPv4Format(addr) == false) {
    return -1;
  }

  // Open a socket
  *fd = socket(PF_INET, SOCK_STREAM, 0);
  if (*fd < 0) {
    return -1;
  }

  // Default values of address & port
  if (port <= 0) {
    port = PORT_DEF;
  }

  // Set address
  memset(&servaddr, 0, sizeof(struct sockaddr_in));
  servaddr.sin_family = PF_INET;
  servaddr.sin_port = htons(port);
  if (inet_aton(reinterpret_cast<const char *>(addr), &servaddr.sin_addr) < 0) {
    sts = -1;
    goto ErrExit;
  }

  // Connect to network port
  if (connect(*fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0) {
    sts = -1;
    goto ErrExit;
  }

  // Return socket fd
  return sts;

 ErrExit:
  close(*fd);
  return sts;
}

void deinitializeSocket(int32_t fd) {
  close(fd);
}

int32_t acceptSocket(int32_t fd, int32_t *sd) {
  struct sockaddr_in cliaddr;
  socklen_t clilen;
  // Accept new connection
  clilen = sizeof(struct sockaddr_in);
  *sd = accept(fd, (struct sockaddr *)&cliaddr, &clilen);
  if (*sd < 0) {
    return false;
  }
  return true;
}

int32_t transferSocket(int32_t fd, const void *pktBuf, ssize_t length) {
  if (send(fd, pktBuf, length, 0) != length) {
    return false;
  }
  return true;
}

int32_t receiveSocket(int32_t fd, void *pktBuf, ssize_t length) {
  if (recv(fd, pktBuf, length, MSG_WAITALL) != length) {
    return false;
  }
  return true;
}
