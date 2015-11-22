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
 * Class header support
 */

#ifndef CART_CLASS_H_
#define CART_CLASS_H_

#include "hash.h"
#include "heap.h"
#include "oat.h"

typedef struct PACKED {
  uint32_t        field_id;
  const uint8_t*  field_str;
  uint32_t        access_flags;
} Field_t;

typedef struct PACKED {
  uint32_t                    method_id;
  const uint8_t*              method_name_str;
  uint32_t                    method_type_id;
  const uint8_t*              method_type_str;
  uint32_t                    method_proto_id;
  const uint8_t*              method_proto_shorty_str;
  const uint8_t*              method_proto_return_type_str;
  const uint16_t*             method_dex_code;
  const uint8_t*              method_oat_code;
  const OatQuickMethodHdr_t*  method_oat_code_hdr;
} Method_t;

typedef struct PACKED {
  uint32_t        class_id;
  uint32_t        superclass_id;
  const uint8_t*  superclass_str;
  const uint8_t*  class_name_str;

  HashTable_t*    static_fields;
  HashTable_t*    instance_fields;
  HashTable_t*    direct_methods;
  HashTable_t*    virtual_methods;
} Class_t;

typedef struct PACKED {
  HashTable_t* loaded_classes;
  HeapVolume_t* heap_vol;
} ClassLinker_t;

ClassLinker_t* AllocateClassLinker(size_t heap_size);
void FreeClassLinker(ClassLinker_t* pClassLinker);

bool RegisterOatDexFile(OatDexFile_t* pOatDexFile, ClassLinker_t* pClassLinker);
bool DeregisterAllClasses(ClassLinker_t* pClassLinker);
bool ParseDexClass(DexFileData_t* pDexFileData, ClassLinker_t* pClassLinker, const uint8_t* oat_base);
bool LoadClassesOfOatDexFile(HashTable_t* pOatDexFiles, ClassLinker_t* pClassLinker, const uint8_t* path);

Class_t* ClFindClass(ClassLinker_t* pCL, const uint8_t* name);
Method_t* ClFindMethod(Class_t* pClass, const uint8_t* name);
void ExecuteOatCode(const void* code);

#endif  // CART_CLASS_H_

