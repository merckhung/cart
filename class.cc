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
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "hash.h"
#include "heap.h"
#include "oat.h"
#include "class.h"
#include "cart.h"

ClassLinker_t* AllocateClassLinker(size_t heap_size) {
  // Allocate a classlinker structure
  ClassLinker_t* pClassLinker = (ClassLinker_t*)malloc(sizeof(ClassLinker_t));
  if (!pClassLinker) {
    pdbg("Out of memory\n");
    return NULL;
  }
  // Allocate heap
  pClassLinker->heap_vol = AllocHeapVolume(heap_size);
  if (!pClassLinker->heap_vol) {
    pdbg("Out of memory\n");
    free((void*)pClassLinker);
    return NULL;
  }
  // Allocate a hash table of loaded classes
  pClassLinker->loaded_classes = AllocHashTable(pClassLinker->heap_vol, CLASSES_HASH_NR);
  if (!pClassLinker->loaded_classes) {
    pdbg("Out of memory\n");
    FreeHeapVolume(pClassLinker->heap_vol);
    free((void*)pClassLinker);
    return NULL;
  }
  // Succeed and return
  return pClassLinker;
}

void FreeClassLinker(ClassLinker_t* pClassLinker) {
  FreeHeapVolume(pClassLinker->heap_vol);
  FreeHashTable(pClassLinker->loaded_classes);
  free(pClassLinker);
}

bool RegisterOatDexFile(OatDexFile_t* pOatDexFile, ClassLinker_t* pClassLinker) {
  DexFileData_t* pDexFileData = pOatDexFile->dex_files;
  // Iterate each DexFileData record
  for (; pDexFileData; pDexFileData = pDexFileData->Next) {
    // Parse DEX classes
    if (ParseDexClass(pDexFileData, pClassLinker, (const uint8_t*)pOatDexFile->oat_hdr) == false) {
      pdbg("Failed to parse DEX classes");
      return false;
    }
  }
  return true;
}

bool DeregisterAllClasses(ClassLinker_t* pClassLinker) {
  return true;
}

bool ParseDexClass(DexFileData_t* pDexFileData, ClassLinker_t* pClassLinker, const uint8_t* oat_base) {
  // Class level iteration
  for (uint32_t class_idx = 0; class_idx < pDexFileData->nr_classes; ++class_idx) {
    // Allocate a class
    Class_t* pClass = (Class_t*)HeapAlloc(pClassLinker->heap_vol, sizeof(Class_t));
    if (!pClass) {
      pdbg("Out of memory\n");
      return false;
    }
    memset((void*)pClass, 0, sizeof(Class_t));

    // Assign values
    pClass->class_id = GetClassIdOfClassDefByIdx(pDexFileData, class_idx);
    pClass->superclass_id = GetSuperclassIdOfClassDefByIdx(pDexFileData, class_idx);
    pClass->superclass_str = GetSuperclassStringOfClassDefByIdx(pDexFileData, class_idx);
    pClass->class_name_str = GetClassStringOfClassDefByIdx(pDexFileData, class_idx);

    // Debug messages
    pdbg("Class ID         : %d\n", pClass->class_id);
    pdbg("Superclass ID    : %d\n", pClass->superclass_id);
    pdbg("Class name       : \"%s\"\n", pClass->class_name_str);
    pdbg("Superclass name  : \"%s\"\n", pClass->superclass_str);

    // Retrieve the class data header
    const uint8_t* ptr = GetClassDataHdrPtrOfClassDefByIdx(pDexFileData, class_idx);
    if (!ptr) {
      pdbg("Invalid DEX format\n");
      return false;
    }

    // Decode the leb128 values
    ClassDataHdr_t bCDH;
    ptr = ComputeClassDataHdr(ptr, &bCDH);

#if 0
    // Debug messages
    pdbg("static   = 0x%X\n", bCDH.static_fields_size_);
    pdbg("instance = 0x%X\n", bCDH.instance_fields_size_);
    pdbg("direct   = 0x%X\n", bCDH.direct_methods_size_);
    pdbg("virtual  = 0x%X\n", bCDH.virtual_methods_size_);
#endif

    // Static fields
    if (bCDH.static_fields_size_) {
      // Allocate static field hash table
      pClass->static_fields = AllocHashTable(pClassLinker->heap_vol, bCDH.static_fields_size_);
      if (!pClass->static_fields) {
        pdbg("Out of memory\n");
        return false;
      }

      // Iterate static fields
      ClassDataField_t bCDF;
      const uint8_t *end = ptr;
      for (uint32_t field_idx = 0; field_idx < bCDH.static_fields_size_; ++field_idx) {
        // Decode a method
        end = GetStaticFieldOfClassDataByIdx(&bCDH, ptr, &bCDF, field_idx);

        // Allocate a method
        Field_t* pField = (Field_t*)HeapAlloc(pClassLinker->heap_vol, sizeof(Field_t));
        if (!pField) {
          pdbg("Out of memory\n");
          return false;
        }
        memset((void*)pField, 0, sizeof(Field_t));

        // Assign values
        pField->field_id = bCDF.field_idx_delta_;
        pField->field_str = GetFieldNameStringById(pDexFileData, bCDF.field_idx_delta_);
        pField->access_flags = bCDF.access_flags_;

#if 1
        // Debug messages
        pdbg("  (S)Field ID                : %u\n", pField->field_id);
        pdbg("  (S)Field name              : \"%s\"\n", pField->field_str);
        pdbg("  (S)Field access_flags      : 0x%X\n", pField->access_flags);
#endif

        // Insert this method
        // InsertHashEntry(pClass->static_fields, GenHashKey(pField->field_str), (void*)pField);
        InsertHashEntry(pClass->static_fields, pField->field_id, (void*)pField);
      }
      ptr = end;
    }  // if (bCDH.static_fields_size_)

    // Instance fields
    if (bCDH.instance_fields_size_) {
      // Allocate static field hash table
      pClass->instance_fields = AllocHashTable(pClassLinker->heap_vol, bCDH.instance_fields_size_);
      if (!pClass->instance_fields) {
        pdbg("Out of memory\n");
        return false;
      }

      // Iterate static fields
      ClassDataField_t bCDF;
      const uint8_t *end = ptr;
      for (uint32_t field_idx = 0; field_idx < bCDH.instance_fields_size_; ++field_idx) {
        // Decode a method
        end = GetInstanceFieldOfClassDataByIdx(&bCDH, ptr, &bCDF, field_idx);

        // Allocate a method
        Field_t* pField = (Field_t*)HeapAlloc(pClassLinker->heap_vol, sizeof(Field_t));
        if (!pField) {
          pdbg("Out of memory\n");
          return false;
        }
        memset((void*)pField, 0, sizeof(Field_t));

        // Assign values
        pField->field_id = bCDF.field_idx_delta_;
        pField->field_str = GetFieldNameStringById(pDexFileData, bCDF.field_idx_delta_);
        pField->access_flags = bCDF.access_flags_;

#if 1
        // Debug messages
        pdbg("  (I)Field ID                : %u\n", pField->field_id);
        pdbg("  (I)Field name              : \"%s\"\n", pField->field_str);
        pdbg("  (I)Field access_flags      : 0x%X\n", pField->access_flags);
#endif

        // Insert this method
        // InsertHashEntry(pClass->instance_fields, GenHashKey(pField->field_str), (void*)pField);
        InsertHashEntry(pClass->instance_fields, pField->field_id, (void*)pField);
      }
      ptr = end;
    }  // if (bCDH.instance_fields_size_)

    // Count the index for computing OAT methods
    uint32_t oat_method_idx = 0;

    // Direct methods
    if (bCDH.direct_methods_size_) {
      // Allocate direct method hash table
      pClass->direct_methods = AllocHashTable(pClassLinker->heap_vol, bCDH.direct_methods_size_);
      if (!pClass->direct_methods) {
        pdbg("Out of memory\n");
        return false;
      }

      // Iterate direct methods
      ClassDataMethod_t bCDM;
      const uint8_t *end = ptr;
      for (uint32_t method_idx = 0; method_idx < bCDH.direct_methods_size_; ++method_idx, ++oat_method_idx) {
        // Decode a method
        end = GetDirectMethodOfClassDataByIdx(&bCDH, ptr, &bCDM, method_idx);

        // Allocate a method
        Method_t* pMethod = (Method_t*)HeapAlloc(pClassLinker->heap_vol, sizeof(Method_t));
        if (!pMethod) {
          pdbg("Out of memory\n");
          return false;
        }
        memset((void*)pMethod, 0, sizeof(Method_t));

        // Assign values
        pMethod->method_id = bCDM.method_idx_delta_;
        pMethod->method_name_str = GetMethodNameStringOfCDM(pDexFileData, &bCDM);
        pMethod->method_proto_id = GetProtoIdOfCDM(pDexFileData, &bCDM);
        pMethod->method_proto_shorty_str = GetProtoShortyStringOfCDM(pDexFileData, &bCDM);
        pMethod->method_proto_return_type_str = GetProtoReturnTypeStringOfCDM(pDexFileData, &bCDM);
        pMethod->method_dex_code = GetDexCodePtrOfCDM(pDexFileData, &bCDM);
        pMethod->method_oat_code = GetOatCodePtrByIdx(pDexFileData, oat_base, class_idx, oat_method_idx);
        pMethod->method_oat_code_hdr = GetOatQuickMethodHdrByIdx(pDexFileData, oat_base, class_idx, oat_method_idx);

#if 1
        // Debug messages
        pdbg("  (D)Method ID               : %u\n", pMethod->method_id);
        pdbg("  (D)Method name             : \"%s\"\n", pMethod->method_name_str);
        pdbg("  (D)Method proto id         : %u\n", pMethod->method_proto_id);
        pdbg("  (D)Method proto shorty     : \"%s\"\n", pMethod->method_proto_shorty_str);
        pdbg("  (D)Method proto return type: \"%s\"\n", pMethod->method_proto_return_type_str);
        pdbg("  (D)Method DEX code ptr     : %p\n", pMethod->method_dex_code);
        pdbg("  (D)Method OAT code ptr     : %p\n", pMethod->method_oat_code);
        pdbg("  (D)Method OAT code size    : %d\n", pMethod->method_oat_code_hdr->code_size_);
        // DumpData((const uint32_t*)pMethod->method_oat_code, pMethod->method_oat_code_hdr->code_size_, 0);
#endif

        // Insert this method
        if (pMethod->method_name_str) {
          InsertHashEntry(pClass->direct_methods, GenHashKey(pMethod->method_name_str), (void*)pMethod);
        } else {
          InsertHashEntry(pClass->direct_methods, pMethod->method_id, (void*)pMethod);
        }
      }
      ptr = end;
    }  // if (bCDH.direct_methods_size_)

    // Virtual methods
    if (bCDH.virtual_methods_size_) {
      // Allocate virtual method hash table
      pClass->virtual_methods = AllocHashTable(pClassLinker->heap_vol, bCDH.virtual_methods_size_);
      if (!pClass->virtual_methods) {
        pdbg("Out of memory\n");
        return false;
      }

      // Iterate virtual methods
      ClassDataMethod_t bCDM;
      const uint8_t *end = ptr;
      for (uint32_t method_idx = 0; method_idx < bCDH.virtual_methods_size_; ++method_idx, ++oat_method_idx) {
        // Decode a method
        end = GetVirtualMethodOfClassDataByIdx(&bCDH, ptr, &bCDM, method_idx);

        // Allocate a method
        Method_t* pMethod = (Method_t*)HeapAlloc(pClassLinker->heap_vol, sizeof(Method_t));
        if (!pMethod) {
          pdbg("Out of memory\n");
          return false;
        }
        memset((void*)pMethod, 0, sizeof(Method_t));

        // Assign values
        pMethod->method_id = bCDM.method_idx_delta_;
        pMethod->method_name_str = GetMethodNameStringOfCDM(pDexFileData, &bCDM);
        pMethod->method_proto_id = GetProtoIdOfCDM(pDexFileData, &bCDM);
        pMethod->method_proto_shorty_str = GetProtoShortyStringOfCDM(pDexFileData, &bCDM);
        pMethod->method_proto_return_type_str = GetProtoReturnTypeStringOfCDM(pDexFileData, &bCDM);
        pMethod->method_dex_code = GetDexCodePtrOfCDM(pDexFileData, &bCDM);
        pMethod->method_oat_code = GetOatCodePtrByIdx(pDexFileData, oat_base, class_idx, oat_method_idx);
        pMethod->method_oat_code_hdr = GetOatQuickMethodHdrByIdx(pDexFileData, oat_base, class_idx, oat_method_idx);

#if 1
        // Debug messages
        pdbg("  (V)Method ID               : %u\n", pMethod->method_id);
        pdbg("  (V)Method name             : \"%s\"\n", pMethod->method_name_str);
        pdbg("  (V)Method proto id         : %u\n", pMethod->method_proto_id);
        pdbg("  (V)Method proto shorty     : \"%s\"\n", pMethod->method_proto_shorty_str);
        pdbg("  (V)Method proto return type: \"%s\"\n", pMethod->method_proto_return_type_str);
        pdbg("  (V)Method DEX code ptr     : %p\n", pMethod->method_dex_code);
        pdbg("  (V)Method OAT code ptr     : %p\n", pMethod->method_oat_code);
        pdbg("  (V)Method OAT code hdr ptr : %p\n", pMethod->method_oat_code_hdr);
        pdbg("  (V)Method OAT code size    : %d\n", pMethod->method_oat_code_hdr->code_size_);
        // DumpData((const uint32_t*)pMethod->method_oat_code, pMethod->method_oat_code_hdr->code_size_, 0);
#endif

        // Insert this method
        // InsertHashEntry(pClass->virtual_methods, GenHashKey(pMethod->method_name_str), (void*)pMethod);
        InsertHashEntry(pClass->virtual_methods, pMethod->method_id, (void*)pMethod);
      }
      ptr = end;
    }  // if (bCDH.virtual_methods_size_)

    // Insert this class
    InsertHashEntry(pClassLinker->loaded_classes,
                    GenHashKeyLen(pClass->class_name_str + 1,
                                  strlen((const char*)pClass->class_name_str + 1) - 1),
                    (void*)pClass);
    // InsertHashEntry(pClassLinker->loaded_classes, pClass->class_id, (void*)pClass);
  }
  // Succeed and return
  return true;
}

bool LoadClassesOfOatDexFile(HashTable_t* pOatDexFiles, ClassLinker_t* pClassLinker, const uint8_t* path) {
  OatDexFile_t* pOatDexFile = NULL;

  // Open the OatDex file
  pOatDexFile = OpenOatDexFile(path);
  if (!pOatDexFile) {
    pdbg("Failed to open OatDex file: %s\n", path);
    return false;
  }
  // Register the OatDex file
  if (RegisterOatDexFile(pOatDexFile, pClassLinker) == false) {
    pdbg("Failed to register OatDex file %s\n", path);
    return false;
  }
  // Insert this OatDex file
  InsertHashEntry(pOatDexFiles, GenHashKey(path), (void*)pOatDexFile);
  return true;
}

Class_t* ClFindClass(ClassLinker_t* pCL, const uint8_t* name) {
  HashEntry_t* e = IsHashExist(pCL->loaded_classes, GenHashKey(name));
  Class_t* pClass = (Class_t*)e->ptr;
  return pClass;
}

Method_t* ClFindMethod(Class_t* pClass, const uint8_t* name) {
  HashEntry_t* e = IsHashExist(pClass->direct_methods, GenHashKey(name));
  Method_t* pMethod = (Method_t*)e->ptr;
  return pMethod;
}

void ExecuteOatCode(const void* code) {
  void (*func)() = (void (*)())code;
  func();
}
