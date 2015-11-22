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
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

#include "macros.h"

#ifdef CART_DEBUG
static int8_t ConvertDWordToByte(const uint32_t* Data, uint32_t Offset) {
  uint32_t tmp, off, bs;
  off = Offset / 4;
  bs = Offset % 4;
  if (bs) {
    tmp = *(Data + off);
    tmp = ((tmp >> (bs * 8)) & 0xFF);
    return tmp;
  }
  tmp = ((*(Data + off)) & 0xFF);
  return tmp;
}

void DumpData(const uint32_t* ptr, uint32_t len, uint32_t label) {
  uint32_t i, j;
  uint32_t c;

  fprintf(stderr, "---------------------------------------------------------------------------\n");
  fprintf(stderr, " Address | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F|   ASCII DATA   \n");
  fprintf(stderr, "---------------------------------------------------------------------------\n");

  for (i = 0; i <= len; i++) {
    if (!(i % 16)) {
      if (i > 15) {
        for (j = i - 16; j < i; j++) {
          c = ConvertDWordToByte(ptr, j);
          if ((c >= '!') && (c <= '~')) {
            fprintf(stderr, "%c", c);
          } else {
            fprintf(stderr, ".");
          }
        }
      }
      if (i) {
        fprintf(stderr, "\n");
      }
      if (i == len) {
        break;
      }
      fprintf(stderr, "%8.8X : ", i + label);
    }
    fprintf(stderr, "%2.2X ", ConvertDWordToByte(ptr, i) & 0xFF);
  }

  if ((i % 16)) {
    for (j = i; j % 16; j++) {
      fprintf(stderr, "   ");
    }
    for (j = (i - (i % 16)); j < (i + 16); j++) {
      if (j < i) {
        c = ConvertDWordToByte(ptr, j);
        if ((c >= '!') && (c <= '~')) {
          fprintf(stderr, "%c", c);
        } else {
          fprintf(stderr, ".");
        }
      } else {
        fprintf(stderr, " ");
      }
    }
    fprintf(stderr, "\n");
  }

  fprintf(stderr, "---------------------------------------------------------------------------\n");
}
#endif

static const char* ParseString(const char* start, const char* end) {
  while (start < end && *start != 0) {
    start++;
  }
  return start;
}

bool GetStoreKeyValuePairByIndex(uint32_t* key_value_store_,
                                 uint32_t key_value_store_size_,
                                 size_t index,
                                 const char** key,
                                 const char** value) {
  const char* ptr = reinterpret_cast<const char*>(key_value_store_);
  const char* end = ptr + key_value_store_size_;
  ssize_t counter = static_cast<ssize_t>(index);

  while (ptr < end && counter >= 0) {
    // Scan for a closing zero.
    const char* str_end = ParseString(ptr, end);
    if (str_end < end) {
      const char* maybe_key = ptr;
      ptr = ParseString(str_end + 1, end) + 1;
      if (ptr <= end) {
        if (counter == 0) {
          *key = maybe_key;
          *value = str_end + 1;
          return true;
        } else {
          counter--;
        }
      } else {
        return false;
      }
    } else {
      break;
    }
  }
  // Not found.
  return false;
}

uint32_t DecodeUnsignedLeb128(const uint8_t** data) {
  const uint8_t* ptr = *data;
  int result = *(ptr++);
  if (result > 0x7f) {
    int cur = *(ptr++);
    result = (result & 0x7f) | ((cur & 0x7f) << 7);
    if (cur > 0x7f) {
      cur = *(ptr++);
      result |= (cur & 0x7f) << 14;
      if (cur > 0x7f) {
        cur = *(ptr++);
        result |= (cur & 0x7f) << 21;
        if (cur > 0x7f) {
          // Note: We don't check to see if cur is out of range here,
          // meaning we tolerate garbage in the four high-order bits.
          cur = *(ptr++);
          result |= cur << 28;
        }
      }
    }
  }
  *data = ptr;
  return static_cast<uint32_t>(result);
}

size_t GenerateOatDexFilename(const uint8_t* file, const uint8_t* isa, uint8_t* out, size_t len) {
  char* fullname = realpath((const char*)file, NULL);
  char dalvik_cache[] = "dalvik-cache";
  char* android_data = getenv("ANDROID_DATA");
  char* ptr = fullname + 1;
  size_t slen = strlen(ptr);
  // Replace '/' with '@' character
  for (size_t i = 0; i < slen; ++i) {
    if (*(ptr + i) == '/') {
      *(ptr + i) = '@';
    }
  }
  // Format the string
  snprintf((char*)out, len, "%s/%s/%s/%s@classes.dex", android_data, dalvik_cache, isa, ptr);
  // Free the buffer
  free(fullname);
  // Return the length of output
  return strlen((char*)out);
}

size_t GenerateOat2DexCmd(const uint8_t* file, const uint8_t* isa, uint8_t* out, size_t len) {
  uint8_t oat_file[PATH_MAX];
  GenerateOatDexFilename(file, isa, oat_file, PATH_MAX);
  char* android_root = getenv("ANDROID_ROOT");
#ifdef CART_DEBUG
  char dex2oat[] = "bin/dex2oatd";
#else
  char dex2oat[] = "bin/dex2oat";
#endif
  // Format the string
  snprintf((char*)out, len, "%s/%s --instruction-set=%s --boot-image=%s/framework/core.art --dex-file=%s --oat-file=%s --android-root=%s",
           android_root, dex2oat, isa, android_root, file, oat_file, android_root);
  // Return the length of output
  return strlen((char*)out);
}

bool IsFileExist(const uint8_t* file) {
  int32_t fd;
  fd = open((const char*)file, O_RDONLY);
  if (fd >= 0) {
    close(fd);
    return true;
  }
  return false;
}
