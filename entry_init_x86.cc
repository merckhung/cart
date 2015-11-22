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
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "utils.h"
#include "macros.h"
#include "cart.h"
#include "thread.h"
#include "entry.h"

#define MODIFY_LDT_CONTENTS_DATA 0

static OatCodeExecEnv_t bOatCodeExecEnv;

#if defined(__APPLE__)
#include <architecture/i386/table.h>
#include <i386/user_ldt.h>
struct descriptor_table_entry_t {
  uint16_t limit0;
  uint16_t base0;
  unsigned base1: 8, type: 4, s: 1, dpl: 2, p: 1;
  unsigned limit: 4, avl: 1, l: 1, d: 1, g: 1, base2: 8;
} __attribute__((packed));
#define MODIFY_LDT_CONTENTS_DATA 0
#else
#include <asm/ldt.h>
#endif

void TestSuspend() {
  pdbg("TestSuspend is called\n");
}

void SetupLdt() {
  memset(&bOatCodeExecEnv, 0, sizeof(OatCodeExecEnv_t));
  bOatCodeExecEnv.tlsPtr_.quick_entrypoints.TestSuspend = TestSuspend;
  pdbg("Address of quick_entrypoints.TestSuspend = %p\n", &bOatCodeExecEnv.tlsPtr_.quick_entrypoints.TestSuspend);
  pdbg("Address of TestSuspend() = %p\n", TestSuspend);

  const uintptr_t base = reinterpret_cast<uintptr_t>(&bOatCodeExecEnv);
  const size_t limit = PAGE_SIZE;

  const int contents = MODIFY_LDT_CONTENTS_DATA;
  const int seg_32bit = 1;
  const int read_exec_only = 0;
  const int limit_in_pages = 0;
  const int seg_not_present = 0;
  const int useable = 1;

  int entry_number = -1;

#if defined(__APPLE__)
  descriptor_table_entry_t entry;
  memset(&entry, 0, sizeof(entry));
  entry.limit0 = (limit & 0x0ffff);
  entry.limit  = (limit & 0xf0000) >> 16;
  entry.base0 = (base & 0x0000ffff);
  entry.base1 = (base & 0x00ff0000) >> 16;
  entry.base2 = (base & 0xff000000) >> 24;
  entry.type = ((read_exec_only ^ 1) << 1) | (contents << 2);
  entry.s = 1;
  entry.dpl = 0x3;
  entry.p = seg_not_present ^ 1;
  entry.avl = useable;
  entry.l = 0;
  entry.d = seg_32bit;
  entry.g = limit_in_pages;

  entry_number = i386_set_ldt(LDT_AUTO_ALLOC, reinterpret_cast<ldt_entry*>(&entry), 1);
  if (entry_number == -1) {
    pdbg("i386_set_ldt failed");
  }
#else
  // Read current LDT entries.
  uint64_t ldt[LDT_ENTRIES];
  size_t ldt_size = sizeof(uint64_t) * LDT_ENTRIES;
  memset(ldt, 0, ldt_size);

  // TODO: why doesn't this return LDT_ENTRY_SIZE * LDT_ENTRIES for the main thread?
  syscall(__NR_modify_ldt, 0, ldt, ldt_size);

  // Find the first empty slot.
  for (entry_number = 0; entry_number < LDT_ENTRIES && ldt[entry_number] != 0; ++entry_number) {
  }
  if (entry_number >= LDT_ENTRIES) {
    pdbg("Failed to find a free LDT slot");
  }

  // Update LDT entry.
  user_desc ldt_entry;
  memset(&ldt_entry, 0, sizeof(ldt_entry));
  ldt_entry.entry_number = entry_number;
  ldt_entry.base_addr = base;
  ldt_entry.limit = limit;
  ldt_entry.seg_32bit = seg_32bit;
  ldt_entry.contents = contents;
  ldt_entry.read_exec_only = read_exec_only;
  ldt_entry.limit_in_pages = limit_in_pages;
  ldt_entry.seg_not_present = seg_not_present;
  ldt_entry.useable = useable;
  syscall(__NR_modify_ldt, 1, &ldt_entry, sizeof(ldt_entry));
  entry_number = ldt_entry.entry_number;
#endif

  // Change %fs to be new LDT entry.
  uint16_t table_indicator = 1 << 2;  // LDT
  uint16_t rpl = 3;  // Requested privilege level
  uint16_t selector = (entry_number << 3) | table_indicator | rpl;
  __asm__ __volatile__("movw %w0, %%fs"
      :    // output
      : "q"(selector)  // input
      :);  // clobber
}

