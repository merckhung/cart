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
 * ELF header support
 */

#ifndef CART_ELF_H_
#define CART_ELF_H_

#include "macros.h"
#include "oat.h"

#define EI_NIDENT   16

// Unsigned program address
typedef unsigned long long    Elf64_Addr;
// Unsigned small integer
typedef unsigned short        Elf64_Half;
// Unsigned file offset
typedef unsigned long long    Elf64_Off;
// Signed medium integer
typedef unsigned int          Elf64_Sword;
// Signed large integer
typedef unsigned long long    Elf64_Sxword;
// Unsigned medium integer
typedef unsigned int          Elf64_Word;
// Unsigned large integer
typedef unsigned long long    Elf64_Xword;
// Unsigned tiny inetger
typedef unsigned char         Elf64_Byte;
// Section index (unsigned)
typedef unsigned short        Elf64_Section;

typedef unsigned int          Elf32_Addr;
typedef unsigned short        Elf32_Half;
typedef unsigned int          Elf32_Off;
typedef unsigned int          Elf32_Sword;
typedef unsigned int          Elf32_Word;
typedef unsigned char         Elf32_Byte;

typedef enum {
  FT_unknown         = 0x0001,
  FT_signed_char     = 0x0001,
  FT_unsigned_char,
  FT_signed_short,
  FT_unsigned_short,
  FT_signed_int32,
  FT_unsigned_int32,
  FT_signed_int64,
  FT_unsigned_int64,
  FT_pointer32,
  FT_pointer64,
  FT_float32,
  FT_float64,
  FT_float128,
  FT_complex64,
  FT_complex128,
  FT_complex256,
  FT_void,
  FT_bool32,
  FT_bool64,
  FT_label32,
  FT_label64,

  FT_struct          = 0x0020,
  FT_union,
  FT_enum,
  FT_typedef,
  FT_set,
  FT_range,
  FT_member_ptr,
  FT_virtual_ptr,
  FT_class,
} FundamentalDataType_t;

typedef enum {
  MOD_pointer_to    = 0x01,
  MOD_reference_to  = 0x02,
  MOD_const         = 0x03,
  MOD_volatile      = 0x04,
  MOD_lo_user       = 0x80,
  MOD_function      = 0x80,
  MOD_array_of      = 0x81,
  MOD_hi_user       = 0xFF,
} TypeQualifiers_t;

typedef enum {
  EF_MIPS_NOREORDER     = 0x00000001,
  EF_MIPS_PIC           = 0x00000002,
  EF_MIPS_CPIC          = 0x00000004,
  EF_MIPS_UCODE         = 0x00000010,
  EF_MIPS_ABI2          = 0x00000020,
  EF_MIPS_OPTIONS_FIRST = 0x00000080,
  EF_MIPS_ARCH_ASE      = 0x0F000000,
  EF_MIPS_ARCH_ASE_MDMX = 0x08000000,
  EF_MIPS_ARCH_ASE_M16  = 0x04000000,
  EF_MIPS_ARCH          = 0xF0000000,
} ProcSpecFlag_t;

typedef struct PACKED {
  // Magic string: 0x7F, 'E', 'L', 'F'
  Elf32_Byte  EI_MAG[4];
  // Class of format: ELFCLASS64 = 2
  Elf32_Byte  EI_CLASS;
  // Endianness: ELFDATAMSB = 2
  Elf32_Byte  EI_DATA;
  // Version of format: EV_CURRENT = 1
  Elf32_Byte  EI_VERSION;
  // Reserved, must be zero
  Elf32_Byte  EI_PAD[9];
} ElfMagic_t;

typedef struct PACKED {
  ElfMagic_t  e_ident;
  Elf64_Half  e_type;
  // Machine
  Elf64_Half  e_machine;
  // File format version
  Elf64_Word  e_version;
  // Process entry address
  Elf64_Addr  e_entry;
  // Program header table file offset
  Elf64_Off   e_phoff;
  // Section header table file offset
  Elf64_Off   e_shoff;
  // Flags
  Elf64_Word  e_flags;
  // ELF header size (bytes)
  Elf64_Half  e_ehsize;
  // Program header entry size
  Elf64_Half  e_phentsize;
  // Number of program headers
  Elf64_Half  e_phnum;
  // Section header entry size
  Elf64_Half  e_shentsize;
  // Number of section headers
  Elf64_Half  e_shnum;
  // Section name string table
  // section header index
  Elf64_Half  e_shstrndx;
} Elf64Hdr_t;

typedef struct PACKED {
  ElfMagic_t  e_ident;
  Elf32_Half  e_type;
  Elf32_Half  e_machine;
  Elf32_Word  e_version;
  Elf32_Addr  e_entry;
  Elf32_Off   e_phoff;
  Elf32_Off   e_shoff;
  Elf32_Word  e_flags;
  Elf32_Half  e_ehsize;
  Elf32_Half  e_phentsize;
  Elf32_Half  e_phnum;
  Elf32_Half  e_shentsize;
  Elf32_Half  e_shnum;
  Elf32_Half  e_shstrndx;
} Elf32Hdr_t;

typedef struct PACKED {
  union {
    Elf32Hdr_t pElf32Hdr;
    Elf64Hdr_t pElf64Hdr;
  };
} ElfHdr_t;

typedef enum {
  // Inactive section
  SHT_NULL        = 0,
  // Information defined by the program
  SHT_PROGBIT,
  // Symbol table
  SHT_SYMTAB,
  // String table
  SHT_STRTAB,
  // Relocation with explicit addends
  SHT_RELA,
  // Symbol hash table
  SHT_HASH,
  // Dynamic linking information
  SHT_DYNAMIC,
  // Vendor-specific file information
  SHT_NOTE,
  // Section contains no bits in object file
  SHT_NOBITS,
  // Relocation without explicit addends
  SHT_REL,
  // Reserved - Non-conforming
  SHT_SHLIB,
  // Dynamic linking symbol table
  SHT_DYNSYM,
  // First processor-specific type
  SHT_LOPROC      = 0x70000000,
  // Last processor-specific type
  SHT_HIPROC      = 0x7FFFFFFF,
  // First application-specific type
  SHT_LOUSER      = 0x80000000,
  // Last application-specific type
  SHT_HIUSER      = 0x8FFFFFFF,

  // ARM AARCH64
  SHT_AARCH64     = 0x70000003,
} SectionType_t;

typedef enum {
  // Section writable during execution
  SHF_WRITE       = 0x1,
  // Section occupies memory
  SHF_ALLOC       = 0x2,
  // Section contains executable instructions
  SHF_EXECINSTR   = 0x4,
  // Reserved for processor-specific flags
  // SHF_MASKPROC    = 0xF00000000,
} SectionAttributeFlag_t;

typedef struct PACKED {
  Elf64_Word    sh_name;
  Elf64_Word    sh_type;
  Elf64_Xword   sh_flags;
  Elf64_Addr    sh_addr;
  Elf64_Off     sh_offset;
  Elf64_Xword   sh_size;
  Elf64_Word    sh_link;
  Elf64_Word    sh_info;
  Elf64_Xword   sh_addralign;
  Elf64_Xword   sh_entsize;
} Elf64SecHdr_t;

typedef struct PACKED {
  Elf32_Word    sh_name;
  Elf32_Word    sh_type;
  Elf32_Word    sh_flags;
  Elf32_Addr    sh_addr;
  Elf32_Off     sh_offset;
  Elf32_Word    sh_size;
  Elf32_Word    sh_link;
  Elf32_Word    sh_info;
  Elf32_Word    sh_addralign;
  Elf32_Word    sh_entsize;
} Elf32SecHdr_t;

typedef struct PACKED {
  union {
    Elf32SecHdr_t pElf32SecHdr;
    Elf64SecHdr_t pElf64SecHdr;
  };
} ElfSecHdr_t;

typedef enum {
  PT_NULL     = 0,
  PT_LOAD,
  PT_DYNAMIC,
  PT_INTERP,
  PT_NOTE,
  PT_SHLIB,
  PT_PHDR,
  // PT_LOPROC   = 0x700000000,
  // PT_HIPROC   = 0x7FFFFFFFF,
} SegmentType_t;

typedef struct PACKED {
  Elf64_Word    p_type;
  Elf64_Off     p_offset;
  Elf64_Addr    p_vaddr;
  Elf64_Addr    p_paddr;
  Elf64_Xword   p_filesz;
  Elf64_Xword   p_memsz;
  Elf64_Word    p_flags;
  Elf64_Xword   p_align;
} Elf64ProgHdr_t;

typedef struct PACKED {
  Elf32_Word    p_type;
  Elf32_Off     p_offset;
  Elf32_Addr    p_vaddr;
  Elf32_Addr    p_paddr;
  Elf32_Word    p_filesz;
  Elf32_Word    p_memsz;
  Elf32_Word    p_flags;
  Elf32_Word    p_align;
} Elf32ProgHdr_t;

bool IsValidElfFile(OatDexFile_t* pOatDexFile);
bool ParseElfFile(OatDexFile_t* pOatDexFile);

#endif  // CART_ELF_H_

