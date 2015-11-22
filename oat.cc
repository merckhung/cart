/*
- * Copyright (C) 2008 The Android Open Source Project
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "utils.h"
#include "list.h"
#include "oat.h"
#include "elf.h"
#include "cart.h"

OatDexFile_t* OpenOatDexFile(const uint8_t* path) {
  OatDexFile_t* pOatDexFile = NULL;
  int32_t fd;
  off_t fs, ofs;
  void* p = NULL;

  // Open a OatDex file
  fd = open((const char*)path, O_RDONLY);
  if (fd < 0) {
    pdbg("Failed to open OatDex file: %s\n", path);
    return NULL;
  }

  // Obtain the file size
  ofs = fs = lseek(fd, 0, SEEK_END);
  if ((lseek(fd, 0, SEEK_SET) == (off_t)-1) || (fs == (off_t)-1)) {
    pdbg("Failed to obtain the file size\n");
    return NULL;
  }

  // Align the file size to the page boundary
  if (fs % PAGE_SIZE) {
    fs = ((fs / PAGE_SIZE) + 1) * PAGE_SIZE;
  }

  // Allocate a handle
  p = malloc(sizeof(OatDexFile_t));
  if (!p) {
    pdbg("Out of memory, failed to allocate a handle\n");
    close(fd);
    return NULL;
  }
  memset(p, 0, sizeof(OatDexFile_t));
  pOatDexFile = (OatDexFile_t*)p;

  // Mmap the file into virtual memory space
  p = mmap(NULL, fs, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);
  if (p == (void *)-1) {
    pdbg("Failed to mmap file: %s\n", path);
    close(fd);
    return NULL;
  }

  // Initialize the data
  pOatDexFile->fd = fd;
  pOatDexFile->file_name = path;
  pOatDexFile->file_sz = ofs;
  pOatDexFile->mem_ptr = p;
  pOatDexFile->mem_sz = fs;

  // Parse the ELF file
  if (ParseElfFile(pOatDexFile) == false) {
    pdbg("Failed to parse ELF file %s\n", path);
    CloseOatDexFile(pOatDexFile);
    return NULL;
  }

  // Parse the OatDex file
  if (ParseOatDexFile(pOatDexFile) == false) {
    pdbg("Failed to parse OatDex file %s\n", path);
    CloseOatDexFile(pOatDexFile);
    return NULL;
  }

  // Return
  return pOatDexFile;
}

void CloseOatDexFile(OatDexFile_t* pOatDexFile) {
  if (!pOatDexFile) {
    pdbg("pOatDexFile == NULL\n");
    return;
  }
  // Release the linklist
  LlFreeAllObject(pOatDexFile->dex_files);

  // Un-mmap the file
  munmap((void*)pOatDexFile->mem_ptr, pOatDexFile->mem_sz);

  // Close the file descriptor
  close(pOatDexFile->fd);

  // Release the source
  free((void*)pOatDexFile);
}

bool IsValidOatDexFile(OatDexFile_t* pOatDexFile) {
  OatHdr_t* pOatHdr = (OatHdr_t*)pOatDexFile->oat_data;

  // Check out the size of OAT data section
  if (pOatDexFile->oat_data_sz < sizeof(OatHdr_t)) {
    pdbg("OAT data section is too short\n");
    return false;
  }

  // Check out the OAT header magic & version
  if ((pOatHdr->magic_[0] == 'o')
      && (pOatHdr->magic_[1] == 'a')
      && (pOatHdr->magic_[2] == 't')
      && (pOatHdr->magic_[3] == '\n')
      && (pOatHdr->version_[0] == '0')
      && (pOatHdr->version_[1] == '3')
      && (pOatHdr->version_[2] == '9')
      && (pOatHdr->version_[3] == 0)) {
    return true;
  }

  // Debug message, if it's invalid
  pdbg("Invalid OAT header, magic=\"%c%c%c%s\", version=\"%c%c%c%s\"\n",
       pOatHdr->magic_[0],
       pOatHdr->magic_[1],
       pOatHdr->magic_[2],
       (pOatHdr->magic_[3] == '\n') ? "\\n" : "?",
       pOatHdr->version_[0],
       pOatHdr->version_[1],
       pOatHdr->version_[2],
       (pOatHdr->version_[3] == '\0') ? "\\0" : "?");
  return false;
}

bool ParseOatDexFile(OatDexFile_t* pOatDexFile) {
  DexFileData_t* pDexFileData;

  // Check out the
  if (IsValidOatDexFile(pOatDexFile) == false) {
    pdbg("Invalid OAT header format\n");
    return false;
  }

  // Setup OAT header pointer
  pOatDexFile->oat_hdr = (const OatHdr_t*)pOatDexFile->oat_data;
  uint8_t* dex_tbl = ((uint8_t*)pOatDexFile->oat_hdr->key_value_store_) + pOatDexFile->oat_hdr->key_value_store_size_;
  pOatDexFile->nr_dex_files = pOatDexFile->oat_hdr->dex_file_count_;

  // Create a linklist for chaining DEX files
  for (uint32_t i = 0; i < pOatDexFile->nr_dex_files; ++i) {
    // Allocate a new resource and then clean up
    pDexFileData = (DexFileData_t*)malloc(sizeof(DexFileData_t));
    if (!pDexFileData) {
      pdbg("Out of memory\n");
      return false;
    }
    memset((void*)pDexFileData, 0, sizeof(DexFileData_t));

    // Obtain the length of the string of the DEX file location
    pDexFileData->dex_file_name_len = *((uint32_t*)dex_tbl);
#if 0
    pdbg("[%d] Length of DEX location string = %d\n", i, pDexFileData->dex_file_name_len);
#endif

    // Move to the string of the DEX file location
    dex_tbl += sizeof(uint32_t);
    pDexFileData->dex_file_name = dex_tbl;
#if 0
    DumpData((const uint32_t*)pDexFileData->dex_file_name, pDexFileData->dex_file_name_len, 0);
#endif

    // Move to the checksum of the DEX file
    dex_tbl += pDexFileData->dex_file_name_len;
#if 0
    pdbg("[%d] Checksum = 0x%X\n", i, *((uint32_t*)dex_tbl));
#endif

    // Move to the offset field of the DEX file
    dex_tbl += sizeof(uint32_t);
#if 0
    pdbg("[%d] Offset = 0x%X\n", i, *((uint32_t*)dex_tbl));
#endif
    pDexFileData->dex_file_ptr = pOatDexFile->oat_data + *((uint32_t*)dex_tbl);

    // Validate DEX header
    if (IsValidDexHeader(pDexFileData->dex_file_ptr) == false) {
      pdbg("Invalid DEX header\n");
      return false;
    }
    // Assign to DexHdr_t
    DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
#if 0
    DumpData((const uint32_t*)pDexFileData->dex_file_ptr, pDexHdr->file_size_, *((uint32_t*)dex_tbl));
#endif

    // Move to the field of class offset pointers
    dex_tbl += sizeof(uint32_t);
    pDexFileData->oat_class_ptrs = (uint32_t*)dex_tbl;
    pDexFileData->nr_classes = pDexHdr->class_defs_size_;

    // Move to next record of the DEX table
    dex_tbl += (sizeof(*pDexFileData->oat_class_ptrs) * pDexFileData->nr_classes);

    // Insert this DEX file data into linklist
    LlAddObjectToHead(pOatDexFile->dex_files, pDexFileData);
  }

  // Return
  return true;
}

bool IsValidDexHeader(const uint8_t* dex_ptr) {
  DexHdr_t* pDexHdr = (DexHdr_t*)dex_ptr;

  // Check out the DEX header magic
  if ((pDexHdr->magic_[0] == 'd')
      && (pDexHdr->magic_[1] == 'e')
      && (pDexHdr->magic_[2] == 'x')
      && (pDexHdr->magic_[3] == '\n')
      && (pDexHdr->magic_[4] == '0')
      && (pDexHdr->magic_[5] == '3')
      && (pDexHdr->magic_[6] == '5')
      && (pDexHdr->magic_[7] == 0)) {
    return true;
  }
  // Debug message, if it's invalid
  pdbg("Invalid DEX header, magic=\"%c%c%c%s%c%c%c%s\"\n",
       pDexHdr->magic_[0],
       pDexHdr->magic_[1],
       pDexHdr->magic_[2],
       (pDexHdr->magic_[3] == '\n') ? "\\n" : "?",
       pDexHdr->magic_[4],
       pDexHdr->magic_[5],
       pDexHdr->magic_[6],
       (pDexHdr->magic_[7] == '\0') ? "\\0" : "?");
  return false;
}

const uint8_t* GetStringById(DexFileData_t* pDexFileData, uint32_t string_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (string_id >= pDexHdr->string_ids_size_) {
    return NULL;
  }
  // Point to the start of the string
  StringId_t* pStringId = ((StringId_t*)(pDexFileData->dex_file_ptr + pDexHdr->string_ids_off_) + string_id);
  const uint8_t* ptr = pDexFileData->dex_file_ptr + pStringId->string_data_off_ + 1;
  return ptr;
}

const uint8_t* GetTypeStringById(DexFileData_t* pDexFileData, uint32_t type_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (type_id >= pDexHdr->type_ids_size_) {
    return NULL;
  }
  // Get the string of the specified id
  TypeId_t* pTypeId = (((TypeId_t*)(pDexFileData->dex_file_ptr + pDexHdr->type_ids_off_)) + type_id);
  return GetStringById(pDexFileData, pTypeId->descriptor_idx_);
}

const uint8_t* GetProtoShortyStringById(DexFileData_t* pDexFileData, uint32_t proto_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (proto_id >= pDexHdr->proto_ids_size_) {
    return NULL;
  }
  // Get the string of the specified id
  ProtoId_t* pProtoId = (((ProtoId_t*)(pDexFileData->dex_file_ptr + pDexHdr->proto_ids_off_)) + proto_id);
  return GetStringById(pDexFileData, pProtoId->shorty_idx_);
}

const uint8_t* GetProtoReturnTypeStringById(DexFileData_t* pDexFileData, uint32_t proto_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (proto_id >= pDexHdr->proto_ids_size_) {
    return NULL;
  }
  // Get the string of the specified id
  ProtoId_t* pProtoId = (((ProtoId_t*)(pDexFileData->dex_file_ptr + pDexHdr->proto_ids_off_)) + proto_id);
  return GetStringById(pDexFileData, pProtoId->return_type_idx_);
}

const TypeList_t* GetProtoTypeListById(DexFileData_t* pDexFileData, uint32_t proto_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (proto_id >= pDexHdr->proto_ids_size_) {
    return NULL;
  }
  // Get the pointer of the specified type id
  ProtoId_t* pProtoId = (((ProtoId_t*)(pDexFileData->dex_file_ptr + pDexHdr->proto_ids_off_)) + proto_id);
  return (const TypeList_t*)(pDexFileData->dex_file_ptr + pProtoId->parameters_off_);
}

uint32_t GetProtoTypeIdOfTypeListByIdx(DexFileData_t* pDexFileData, uint32_t proto_id, uint32_t type_idx) {
  const TypeList_t* pTypeList = GetProtoTypeListById(pDexFileData, proto_id);
  // Boundary check
  if (type_idx >= pTypeList->size_) {
    return 0;
  }
  // Get the type id
  const TypeItem_t* pTypeItem = pTypeList->list_ + type_idx;
  return (uint32_t)pTypeItem->type_idx_;
}

const uint8_t* GetProtoTypeStringOfTypeListByIdx(DexFileData_t* pDexFileData, uint32_t proto_id, uint32_t type_idx) {
  return GetTypeStringById(pDexFileData, GetProtoTypeIdOfTypeListByIdx(pDexFileData, proto_id, type_idx));
}

uint32_t GetFieldClassIdById(DexFileData_t* pDexFileData, uint32_t field_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (field_id >= pDexHdr->field_ids_size_) {
    return 0;
  }
  // Get the id
  FieldId_t* pFieldId = (((FieldId_t*)(pDexFileData->dex_file_ptr + pDexHdr->field_ids_off_)) + field_id);
  return (uint32_t)pFieldId->class_idx_;
}

const uint8_t* GetFieldClassStringById(DexFileData_t* pDexFileData, uint32_t field_id) {
  // Get the string
  return GetTypeStringById(pDexFileData, GetFieldClassIdById(pDexFileData, field_id));
}

uint32_t GetFieldTypeIdById(DexFileData_t* pDexFileData, uint32_t field_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (field_id >= pDexHdr->field_ids_size_) {
    return 0;
  }
  // Get the id
  FieldId_t* pFieldId = (((FieldId_t*)(pDexFileData->dex_file_ptr + pDexHdr->field_ids_off_)) + field_id);
  return pFieldId->type_idx_;
}

const uint8_t* GetFieldTypeStringById(DexFileData_t* pDexFileData, uint32_t field_id) {
  // Get the string
  return GetTypeStringById(pDexFileData, GetFieldTypeIdById(pDexFileData, field_id));
}

const uint8_t* GetFieldNameStringById(DexFileData_t* pDexFileData, uint32_t field_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (field_id >= pDexHdr->field_ids_size_) {
    return NULL;
  }
  // Get the string of the specified id
  FieldId_t* pFieldId = (((FieldId_t*)(pDexFileData->dex_file_ptr + pDexHdr->field_ids_off_)) + field_id);
  return GetStringById(pDexFileData, pFieldId->name_idx_);
}

uint32_t GetMethodClassIdById(DexFileData_t* pDexFileData, uint32_t method_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (method_id >= pDexHdr->method_ids_size_) {
    return 0;
  }
  // Get the id
  MethodId_t* pMethodId = (((MethodId_t*)(pDexFileData->dex_file_ptr + pDexHdr->method_ids_off_)) + method_id);
  return pMethodId->class_idx_;
}

const uint8_t* GetMethodClassStringById(DexFileData_t* pDexFileData, uint32_t method_id) {
  // Get the string
  return GetTypeStringById(pDexFileData, GetMethodClassIdById(pDexFileData, method_id));
}

uint32_t GetMethodProtoIdById(DexFileData_t* pDexFileData, uint32_t method_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (method_id >= pDexHdr->method_ids_size_) {
    return 0;
  }
  // Get the id
  MethodId_t* pMethodId = (((MethodId_t*)(pDexFileData->dex_file_ptr + pDexHdr->method_ids_off_)) + method_id);
  return pMethodId->proto_idx_;
}

const uint8_t* GetMethodProtoStringById(DexFileData_t* pDexFileData, uint32_t method_id) {
  // Get the string
  return GetTypeStringById(pDexFileData, GetMethodProtoIdById(pDexFileData, method_id));
}

const uint8_t* GetMethodNameStringById(DexFileData_t* pDexFileData, uint32_t method_id) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (method_id >= pDexHdr->method_ids_size_) {
    return NULL;
  }
  // Get the id
  MethodId_t* pMethodId = (((MethodId_t*)(pDexFileData->dex_file_ptr + pDexHdr->method_ids_off_)) + method_id);
  return GetStringById(pDexFileData, pMethodId->name_idx_);
}

const ClassDef_t* GetClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx) {
  DexHdr_t* pDexHdr = (DexHdr_t*)pDexFileData->dex_file_ptr;
  // Boundary check
  if (class_idx >= pDexHdr->class_defs_size_) {
    return NULL;
  }
  // Get the id
  ClassDef_t* pClassDef = (((ClassDef_t*)(pDexFileData->dex_file_ptr + pDexHdr->class_defs_off_)) + class_idx);
  return pClassDef;
}

uint32_t GetClassIdOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx) {
  const ClassDef_t* pClassDef = GetClassDefByIdx(pDexFileData, class_idx);
  if (!pClassDef) {
    return 0;
  }
  return pClassDef->class_idx_;
}

const uint8_t* GetClassStringOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx) {
  return GetTypeStringById(pDexFileData, GetClassIdOfClassDefByIdx(pDexFileData, class_idx));
}

uint32_t GetSuperclassIdOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx) {
  const ClassDef_t* pClassDef = GetClassDefByIdx(pDexFileData, class_idx);
  if (!pClassDef) {
    return 0;
  }
  return pClassDef->superclass_idx_;
}

const uint8_t* GetSuperclassStringOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx) {
  return GetTypeStringById(pDexFileData, GetSuperclassIdOfClassDefByIdx(pDexFileData, class_idx));
}

const uint8_t* GetSourcefileStringOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx) {
  const ClassDef_t* pClassDef = GetClassDefByIdx(pDexFileData, class_idx);
  if (!pClassDef) {
    return 0;
  }
  return GetStringById(pDexFileData, pClassDef->source_file_idx_);
}

const uint8_t* GetClassDataHdrPtrOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx) {
  const ClassDef_t* pClassDef = GetClassDefByIdx(pDexFileData, class_idx);
  if (!pClassDef) {
    return 0;
  }
  return (pDexFileData->dex_file_ptr + pClassDef->class_data_off_);
}

const uint8_t* ComputeClassDataHdr(const uint8_t* hdr, ClassDataHdr_t* pClassDataHdr) {
  pClassDataHdr->static_fields_size_ = DecodeUnsignedLeb128(&hdr);
  pClassDataHdr->instance_fields_size_ = DecodeUnsignedLeb128(&hdr);
  pClassDataHdr->direct_methods_size_ = DecodeUnsignedLeb128(&hdr);
  pClassDataHdr->virtual_methods_size_ = DecodeUnsignedLeb128(&hdr);
  return hdr;
}

const uint8_t* GetStaticFieldOfClassDataByIdx(const ClassDataHdr_t* pClassDataHdr,
                                              const uint8_t* end,
                                              ClassDataField_t* pClassDataField,
                                              uint32_t field_idx) {
  if (!pClassDataHdr->static_fields_size_) {
    return end;
  }
  if (field_idx >= pClassDataHdr->static_fields_size_) {
    field_idx = pClassDataHdr->static_fields_size_ - 1;
  }
  for (uint32_t i = 0; i <= field_idx; ++i) {
    pClassDataField->field_idx_delta_ = DecodeUnsignedLeb128(&end);
    pClassDataField->access_flags_ = DecodeUnsignedLeb128(&end);
  }
  return end;
}

const uint8_t* GetInstanceFieldOfClassDataByIdx(const ClassDataHdr_t* pClassDataHdr,
                                                const uint8_t* end,
                                                ClassDataField_t* pClassDataField,
                                                uint32_t field_idx) {
  if (!pClassDataHdr->instance_fields_size_) {
    return end;
  }
  if (field_idx >= pClassDataHdr->instance_fields_size_) {
    field_idx = pClassDataHdr->instance_fields_size_ - 1;
  }
  for (uint32_t i = 0; i <= field_idx; ++i) {
    pClassDataField->field_idx_delta_ = DecodeUnsignedLeb128(&end);
    pClassDataField->access_flags_ = DecodeUnsignedLeb128(&end);
  }
  return end;
}

const uint8_t* GetDirectMethodOfClassDataByIdx(const ClassDataHdr_t* pClassDataHdr,
                                               const uint8_t* end,
                                               ClassDataMethod_t* pClassDataMethod,
                                               uint32_t method_idx) {
  if (!pClassDataHdr->direct_methods_size_) {
    return end;
  }
  if (method_idx >= pClassDataHdr->direct_methods_size_) {
    method_idx = pClassDataHdr->direct_methods_size_ - 1;
  }
  for (uint32_t i = 0; i <= method_idx; ++i) {
    pClassDataMethod->method_idx_delta_ = DecodeUnsignedLeb128(&end);
    pClassDataMethod->access_flags_ = DecodeUnsignedLeb128(&end);
    pClassDataMethod->code_off_ = DecodeUnsignedLeb128(&end);
  }
  return end;
}

const uint8_t* GetVirtualMethodOfClassDataByIdx(const ClassDataHdr_t* pClassDataHdr,
                                                const uint8_t* end,
                                                ClassDataMethod_t* pClassDataMethod,
                                                uint32_t method_idx) {
  if (!pClassDataHdr->virtual_methods_size_) {
    return end;
  }
  if (method_idx >= pClassDataHdr->virtual_methods_size_) {
    method_idx = pClassDataHdr->virtual_methods_size_ - 1;
  }
  for (uint32_t i = 0; i <= method_idx; ++i) {
    pClassDataMethod->method_idx_delta_ = DecodeUnsignedLeb128(&end);
    pClassDataMethod->access_flags_ = DecodeUnsignedLeb128(&end);
    pClassDataMethod->code_off_ = DecodeUnsignedLeb128(&end);
  }
  return end;
}

const uint8_t* GetMethodNameStringOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM) {
  return GetMethodNameStringById(pDexFileData, pCDM->method_idx_delta_);
}

uint32_t GetProtoIdOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM) {
  return GetMethodProtoIdById(pDexFileData, pCDM->method_idx_delta_);
}

const uint8_t* GetProtoShortyStringOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM) {
  return GetProtoShortyStringById(pDexFileData, GetProtoIdOfCDM(pDexFileData, pCDM));
}

const uint8_t* GetProtoReturnTypeStringOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM) {
  return GetProtoReturnTypeStringById(pDexFileData, GetProtoIdOfCDM(pDexFileData, pCDM));
}

const CodeItem_t* GetDexCodeItemOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM) {
  CodeItem_t* pCodeItem = (CodeItem_t*)(pDexFileData->dex_file_ptr + pCDM->code_off_);
  return pCodeItem;
}

const uint16_t* GetDexCodePtrOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM) {
  const CodeItem_t* pCodeItem = GetDexCodeItemOfCDM(pDexFileData, pCDM);
  return pCodeItem->insns_;
}

uint32_t GetAmountOfMethodsByIdx(DexFileData_t* pDexFileData, uint32_t class_idx) {
  // Count the amount number of methods of the specified class
  const uint8_t* hdr = GetClassDataHdrPtrOfClassDefByIdx(pDexFileData, class_idx);
  if (!hdr) {
    return 0;
  }
  ClassDataHdr_t bufClassDataHdr;
  ComputeClassDataHdr(hdr, &bufClassDataHdr);
  return (bufClassDataHdr.direct_methods_size_ + bufClassDataHdr.virtual_methods_size_);
}

const OatMethodOffsets_t* GetOatMethodOffByIdx(DexFileData_t* pDexFileData,
                                               const uint8_t* oat_base,
                                               uint32_t class_idx,
                                               uint32_t oat_method_idx) {
  // OAT class data
  uint32_t oat_class_offset = pDexFileData->oat_class_ptrs[class_idx];
  const uint8_t* oat_class_data = (const uint8_t*)(oat_base + oat_class_offset);
  const uint8_t* oat_class_status = oat_class_data;
  const uint8_t* oat_class_type = oat_class_status + sizeof(uint16_t);
  const uint8_t* oat_class_after_type = oat_class_type + sizeof(int16_t);
  // Pointers
  uint32_t bitmap_size = 0;
  const uint8_t* bitmap_pointer = NULL;
  const uint8_t* oat_methods_off_ptr = oat_class_after_type;
  // If it contains a bitmap
  int16_t type = *((const int16_t*)(oat_class_type));
  if (type == kOatClassSomeCompiled) {
    bitmap_size = (uint32_t)(*(const uint32_t*)(oat_class_after_type));
    bitmap_pointer = oat_class_after_type + sizeof(bitmap_size);
    oat_methods_off_ptr = bitmap_pointer + bitmap_size;
  }
  // Return the data
  return ((const OatMethodOffsets_t*)oat_methods_off_ptr) + oat_method_idx;
}

const uint8_t* GetOatCodePtrByIdx(DexFileData_t* pDexFileData,
                                  const uint8_t* oat_base,
                                  uint32_t class_idx,
                                  uint32_t oat_method_idx) {
  const OatMethodOffsets_t* pOMO = GetOatMethodOffByIdx(pDexFileData,
                                                        oat_base,
                                                        class_idx,
                                                        oat_method_idx);
  if (!pOMO) {
    return NULL;
  }
  return (oat_base + pOMO->code_offset_);
}

const uint8_t* GetOatGcMapPtrByIdx(DexFileData_t* pDexFileData,
                                   const uint8_t* oat_base,
                                   uint32_t class_idx,
                                   uint32_t oat_method_idx) {
  const OatMethodOffsets_t* pOMO = GetOatMethodOffByIdx(pDexFileData,
                                                        oat_base,
                                                        class_idx,
                                                        oat_method_idx);
  if (!pOMO) {
    return NULL;
  }
  return (oat_base + pOMO->gc_map_offset_);
}

const OatQuickMethodHdr_t* GetOatQuickMethodHdrByIdx(DexFileData_t* pDexFileData,
                                                     const uint8_t* oat_base,
                                                     uint32_t class_idx,
                                                     uint32_t oat_method_idx) {
  const uint8_t* ptr = GetOatCodePtrByIdx(pDexFileData, oat_base, class_idx, oat_method_idx);
  ptr -= sizeof(OatQuickMethodHdr_t);
  return (OatQuickMethodHdr_t*)ptr;
}
