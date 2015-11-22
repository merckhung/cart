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
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "elf.h"

static inline bool IsElf64Format(OatDexFile_t* pOatDexFile) {
  ElfHdr_t* pElfHdr = (ElfHdr_t*)pOatDexFile->mem_ptr;
  if (pElfHdr->pElf64Hdr.e_ident.EI_CLASS == 2) {
    return true;
  }
  return false;
}

bool IsValidElfFile(OatDexFile_t* pOatDexFile) {
  ElfHdr_t* pElfHdr = (ElfHdr_t*)pOatDexFile->mem_ptr;
  // Check the file length
  if (pOatDexFile->file_sz < sizeof(ElfHdr_t)) {
    pdbg("File size is too short\n");
    return false;
  }
  // ELF32 format check
  if ((pElfHdr->pElf32Hdr.e_ident.EI_MAG[0] == 0x7F)
      && (pElfHdr->pElf32Hdr.e_ident.EI_MAG[1] == 'E')
      && (pElfHdr->pElf32Hdr.e_ident.EI_MAG[2] == 'L')
      && (pElfHdr->pElf32Hdr.e_ident.EI_MAG[3] == 'F')
      && (pElfHdr->pElf32Hdr.e_ident.EI_CLASS == 1)
      && (pElfHdr->pElf32Hdr.e_ident.EI_DATA == 1)  // Little endian only
      && (pElfHdr->pElf32Hdr.e_ident.EI_VERSION == 1)) {
    return true;
  }
  // ELF64 format check
  if ((pElfHdr->pElf64Hdr.e_ident.EI_MAG[0] == 0x7F)
      && (pElfHdr->pElf64Hdr.e_ident.EI_MAG[1] == 'E')
      && (pElfHdr->pElf64Hdr.e_ident.EI_MAG[2] == 'L')
      && (pElfHdr->pElf64Hdr.e_ident.EI_MAG[3] == 'F')
      && (pElfHdr->pElf64Hdr.e_ident.EI_CLASS == 2)
      && (pElfHdr->pElf64Hdr.e_ident.EI_DATA == 1)  // Little endian only
      && (pElfHdr->pElf64Hdr.e_ident.EI_VERSION == 1)) {
    return true;
  }
  // Invalid format
  pdbg("Is neither ELF64 nor ELF32\n");
  return false;
}

static inline const uint8_t* RetrieveElfStr(
    const uint8_t* base,
    ElfSecHdr_t* pStrSecHdr,
    uint32_t offset,
    bool isElf64) {
  if (isElf64 == true) {
    return (const uint8_t*)(base + pStrSecHdr->pElf64SecHdr.sh_offset + offset);
  }
  return (const uint8_t*)(base + pStrSecHdr->pElf32SecHdr.sh_offset + offset);
}

bool ParseElfFile(OatDexFile_t* pOatDexFile) {
  ElfHdr_t* pElfHdr = (ElfHdr_t*)pOatDexFile->mem_ptr;
  ElfSecHdr_t* pElfSecHdr = NULL;
  ElfSecHdr_t* pStrSecHdr = NULL;
  uint32_t sh_num;
  uint32_t sh_size;
  uint32_t str_idx;
  uint32_t i;
  bool is_elf64;
  const uint8_t* str;
  uint32_t sh_name;

  // Check out the header data
  if (IsValidElfFile(pOatDexFile) == false) {
    pdbg("Invalid ELF format\n");
    return false;
  }

  // Initialize data
  is_elf64 = IsElf64Format(pOatDexFile);
  if (is_elf64 == true) {
    pOatDexFile->elf_file_hdr = (const uint8_t*)pOatDexFile->mem_ptr;
    pOatDexFile->elf_prog_hdr = pOatDexFile->elf_file_hdr + pElfHdr->pElf64Hdr.e_phoff;
    pOatDexFile->elf_sec_hdr = pOatDexFile->elf_file_hdr + pElfHdr->pElf64Hdr.e_shoff;
    sh_num = pElfHdr->pElf64Hdr.e_shnum;
    sh_size = pElfHdr->pElf64Hdr.e_shentsize;
    str_idx = pElfHdr->pElf64Hdr.e_shstrndx;
  } else {
    pOatDexFile->elf_file_hdr = (const uint8_t*)pOatDexFile->mem_ptr;
    pOatDexFile->elf_prog_hdr = pOatDexFile->elf_file_hdr + pElfHdr->pElf32Hdr.e_phoff;
    pOatDexFile->elf_sec_hdr = pOatDexFile->elf_file_hdr + pElfHdr->pElf32Hdr.e_shoff;
    sh_num = pElfHdr->pElf32Hdr.e_shnum;
    sh_size = pElfHdr->pElf32Hdr.e_shentsize;
    str_idx = pElfHdr->pElf32Hdr.e_shstrndx;
  }

  // Lookup .rodata & .text sections
  pOatDexFile->elf_sec_str = pOatDexFile->elf_sec_hdr + (sh_size * str_idx);
  pStrSecHdr = (ElfSecHdr_t*)pOatDexFile->elf_sec_str;
  for (i = 0; i < sh_num; ++i) {
    // Obtain the section header
    pElfSecHdr = (ElfSecHdr_t*)(pOatDexFile->elf_sec_hdr + (sh_size * i));
    if (is_elf64 == true) {
      sh_name = pElfSecHdr->pElf64SecHdr.sh_name;
    } else {
      sh_name = pElfSecHdr->pElf32SecHdr.sh_name;
    }
    // Retrieve the string of the section name
    str = RetrieveElfStr(pOatDexFile->elf_file_hdr,
                         pStrSecHdr,
                         sh_name,
                         is_elf64);
    // Compare the string, look for .rodata and .text sections
    if (!strcmp((const char*)str, OAT_DATA_SECNAME)) {
      pOatDexFile->elf_sec_rodata = pOatDexFile->elf_sec_hdr + (sh_size * i);
    } else if (!strcmp((const char*)str, OAT_TEXT_SECNAME)) {
      pOatDexFile->elf_sec_text = pOatDexFile->elf_sec_hdr + (sh_size * i);
    }
  }

  // Sanity check
  if (!pOatDexFile->elf_sec_rodata || !pOatDexFile->elf_sec_text) {
    pdbg("There's no .rodata and .text sections in the ELF file\n");
    return false;
  }
#if 0
  pdbg("elf_sec_rodata = %p, elf_sec_text = %p\n", pOatDexFile->elf_sec_rodata, pOatDexFile->elf_sec_text);
#endif

  // Setup OAT header and code pointers
  pElfSecHdr = (ElfSecHdr_t*)pOatDexFile->elf_sec_rodata;
  if (is_elf64 == true) {
    pOatDexFile->oat_data = pOatDexFile->elf_file_hdr + pElfSecHdr->pElf64SecHdr.sh_offset;
    pOatDexFile->oat_data_sz = pElfSecHdr->pElf64SecHdr.sh_size;

    pElfSecHdr = (ElfSecHdr_t*)pOatDexFile->elf_sec_text;
    pOatDexFile->oat_code = pOatDexFile->elf_file_hdr + pElfSecHdr->pElf64SecHdr.sh_offset;
    pOatDexFile->oat_code_sz = pElfSecHdr->pElf64SecHdr.sh_size;
  } else {
    pOatDexFile->oat_data = pOatDexFile->elf_file_hdr + pElfSecHdr->pElf32SecHdr.sh_offset;
    pOatDexFile->oat_data_sz = pElfSecHdr->pElf32SecHdr.sh_size;

    pElfSecHdr = (ElfSecHdr_t*)pOatDexFile->elf_sec_text;
    pOatDexFile->oat_code = pOatDexFile->elf_file_hdr + pElfSecHdr->pElf32SecHdr.sh_offset;
    pOatDexFile->oat_code_sz = pElfSecHdr->pElf32SecHdr.sh_size;
  }
#if 0
  pdbg("oat_data(0x%x)=%p, oat_code(0x%x)=%p\n",
       pOatDexFile->oat_data_sz,
       pOatDexFile->oat_data,
       pOatDexFile->oat_code_sz,
       pOatDexFile->oat_code);
#endif

  // Return
  return true;
}
