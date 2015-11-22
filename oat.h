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
 * OAT header support
 */

#ifndef CART_OAT_H_
#define CART_OAT_H_

#include "macros.h"
#include "hash.h"

#define OAT_DATA_SECNAME    ".rodata"
#define OAT_TEXT_SECNAME    ".text"
#define kSha1DigestSize     20

enum OatClassType {
  kOatClassAllCompiled = 0,   // OatClass is followed by an OatMethodOffsets for each method.
  kOatClassSomeCompiled = 1,  // A bitmap of which OatMethodOffsets are present follows the OatClass.
  kOatClassNoneCompiled = 2,  // All methods are interpretted so no OatMethodOffsets are necessary.
  kOatClassMax = 3,
};

typedef struct PACKED {
  uint32_t code_offset_;
  uint32_t gc_map_offset_;
} OatMethodOffsets_t;

typedef struct PACKED {
  uint32_t frame_size_in_bytes_;
  uint32_t core_spill_mask_;
  uint32_t fp_spill_mask_;
} OatQuickMethodFrameInfo_t;

typedef struct PACKED {
  // The offset in bytes from the start of the mapping table to the end of the header.
  uint32_t mapping_table_offset_;
  // The offset in bytes from the start of the vmap table to the end of the header.
  uint32_t vmap_table_offset_;
  // The stack frame information.
  OatQuickMethodFrameInfo_t frame_info_;
  // The code size in bytes.
  uint32_t code_size_;
} OatQuickMethodHdr_t;

typedef struct PACKED {
  uint8_t   magic_[4];
  uint8_t   version_[4];
  uint32_t  adler32_checksum_;
  uint32_t  instruction_set_;
  uint32_t  instruction_set_features_;
  uint32_t  dex_file_count_;
  uint32_t  executable_offset_;
  uint32_t  interpreter_to_interpreter_bridge_offset_;
  uint32_t  interpreter_to_compiled_code_bridge_offset_;
  uint32_t  jni_dlsym_lookup_offset_;
  uint32_t  portable_imt_conflict_trampoline_offset_;
  uint32_t  portable_resolution_trampoline_offset_;
  uint32_t  portable_to_interpreter_bridge_offset_;
  uint32_t  quick_generic_jni_trampoline_offset_;
  uint32_t  quick_imt_conflict_trampoline_offset_;
  uint32_t  quick_resolution_trampoline_offset_;
  uint32_t  quick_to_interpreter_bridge_offset_;
  uint32_t  image_patch_delta_;
  uint32_t  image_file_location_oat_checksum_;
  uint32_t  image_file_location_oat_data_begin_;
  uint32_t  key_value_store_size_;
  uint32_t  key_value_store_[0];
} OatHdr_t;

typedef struct PACKED {
  uint8_t magic_[4];
  uint8_t version_[4];
  // Required base address for mapping the image.
  uint32_t image_begin_;
  // Image size, not page aligned.
  uint32_t image_size_;
  // Image bitmap offset in the file.
  uint32_t image_bitmap_offset_;
  // Size of the image bitmap.
  uint32_t image_bitmap_size_;
  // Checksum of the oat file we link to for load time sanity check.
  uint32_t oat_checksum_;
  // Start address for oat file. Will be before oat_data_begin_ for .so files.
  uint32_t oat_file_begin_;
  // Required oat address expected by image Method::GetCode() pointers.
  uint32_t oat_data_begin_;
  // End of oat data address range for this image file.
  uint32_t oat_data_end_;
  // End of oat file address range. will be after oat_data_end_ for
  // .so files. Used for positioning a following alloc spaces.
  uint32_t oat_file_end_;
  // The total delta that this image has been patched.
  int32_t patch_delta_;
  // Absolute address of an Object[] of objects needed to reinitialize from an image.
  uint32_t image_roots_;
} ArtHdr_t;

typedef struct PACKED {
  uint8_t  magic_[8];
  uint32_t checksum_;                   // See also location_checksum_
  uint8_t  signature_[kSha1DigestSize];
  uint32_t file_size_;                  // size of entire file
  uint32_t header_size_;                // offset to start of next section
  uint32_t endian_tag_;
  uint32_t link_size_;                  // unused
  uint32_t link_off_;                   // unused
  uint32_t map_off_;                    // unused
  uint32_t string_ids_size_;            // number of StringIds
  uint32_t string_ids_off_;             // file offset of StringIds array
  uint32_t type_ids_size_;              // number of TypeIds, we don't support more than 65535
  uint32_t type_ids_off_;               // file offset of TypeIds array
  uint32_t proto_ids_size_;             // number of ProtoIds, we don't support more than 65535
  uint32_t proto_ids_off_;              // file offset of ProtoIds array
  uint32_t field_ids_size_;             // number of FieldIds
  uint32_t field_ids_off_;              // file offset of FieldIds array
  uint32_t method_ids_size_;            // number of MethodIds
  uint32_t method_ids_off_;             // file offset of MethodIds array
  uint32_t class_defs_size_;            // number of ClassDefs
  uint32_t class_defs_off_;             // file offset of ClassDef array
  uint32_t data_size_;                  // unused
  uint32_t data_off_;                   // unused
} DexHdr_t;

typedef struct PACKED _DexFileData {
  // Linklist
  struct _DexFileData* Next;
  // DEX file
  const uint8_t*       dex_file_ptr;
  uint32_t             dex_file_size;
  // File path
  uint8_t*             dex_file_name;
  uint32_t             dex_file_name_len;
  // OAT methods
  uint32_t*            oat_class_ptrs;
  uint32_t             nr_classes;
} DexFileData_t;

typedef struct PACKED {
  // File handler
  const uint8_t*     file_name;
  size_t             file_sz;
  const void*        mem_ptr;
  size_t             mem_sz;
  int32_t            fd;
  // ELF handler
  const uint8_t*     elf_file_hdr;
  const uint8_t*     elf_prog_hdr;
  const uint8_t*     elf_sec_hdr;
  const uint8_t*     elf_sec_str;
  const uint8_t*     elf_sec_text;
  const uint8_t*     elf_sec_rodata;
  const uint8_t*     oat_data;
  uint32_t           oat_data_sz;
  const uint8_t*     oat_code;
  uint32_t           oat_code_sz;
  // OatDex handler
  const OatHdr_t*    oat_hdr;
  DexFileData_t*     dex_files;
  uint32_t           nr_dex_files;
} OatDexFile_t;

typedef struct PACKED {
  uint32_t string_data_off_;  // offset in bytes from the base address
} StringId_t;

typedef struct PACKED {
  uint32_t descriptor_idx_;   // index into string_ids
} TypeId_t;

typedef struct PACKED {
  uint32_t shorty_idx_;       // index into string_ids array for shorty descriptor
  uint16_t return_type_idx_;  // index into type_ids array for return type
  uint16_t pad_;              // padding = 0
  uint32_t parameters_off_;   // file offset to type_list for parameter types
} ProtoId_t;

typedef struct PACKED {
  uint16_t type_idx_;         // index into type_ids section
} TypeItem_t;

typedef struct PACKED {
  uint32_t    size_;          // size of the list, in entries
  TypeItem_t  list_[1];
} TypeList_t;

typedef struct PACKED {
  uint16_t class_idx_;        // index into type_ids_ array for defining class
  uint16_t type_idx_;         // index into type_ids_ array for field type
  uint32_t name_idx_;         // index into string_ids_ array for field name
} FieldId_t;

typedef struct PACKED {
  uint16_t class_idx_;        // index into type_ids_ array for defining class
  uint16_t proto_idx_;        // index into proto_ids_ array for method prototype
  uint32_t name_idx_;         // index into string_ids_ array for method name
} MethodId_t;

typedef struct PACKED {
  uint16_t class_idx_;          // index into type_ids_ array for this class
  uint16_t pad1_;               // padding = 0
  uint32_t access_flags_;
  uint16_t superclass_idx_;     // index into type_ids_ array for superclass
  uint16_t pad2_;               // padding = 0
  uint32_t interfaces_off_;     // file offset to TypeList
  uint32_t source_file_idx_;    // index into string_ids_ for source file name
  uint32_t annotations_off_;    // file offset to annotations_directory_item
  uint32_t class_data_off_;     // file offset to class_data_item
  uint32_t static_values_off_;  // file offset to EncodedArray
} ClassDef_t;

typedef struct PACKED {
  uint32_t static_fields_size_;    // the number of static fields
  uint32_t instance_fields_size_;  // the number of instance fields
  uint32_t direct_methods_size_;   // the number of direct methods
  uint32_t virtual_methods_size_;  // the number of virtual methods
} ClassDataHdr_t;

typedef struct PACKED {
  uint32_t field_idx_delta_;   // delta of index into the field_ids array for FieldId
  uint32_t access_flags_;      // access flags for the field
} ClassDataField_t;

typedef struct PACKED {
  uint32_t method_idx_delta_;  // delta of index into the method_ids array for MethodId
  uint32_t access_flags_;
  uint32_t code_off_;
} ClassDataMethod_t;

typedef struct {
  uint16_t registers_size_;
  uint16_t ins_size_;
  uint16_t outs_size_;
  uint16_t tries_size_;
  uint32_t debug_info_off_;            // file offset to debug info stream
  uint32_t insns_size_in_code_units_;  // size of the insns array, in 2 byte code units
  uint16_t insns_[1];
} CodeItem_t;

OatDexFile_t* OpenOatDexFile(const uint8_t* path);
void CloseOatDexFile(OatDexFile_t* pOatDexFile);
bool IsValidOatDexFile(OatDexFile_t* pOatDexFile);
bool ParseOatDexFile(OatDexFile_t* pOatDexFile);
bool IsValidDexHeader(const uint8_t* dex_ptr);

// String
const uint8_t* GetStringById(DexFileData_t* pDexFileData, uint32_t string_id);
// Type
const uint8_t* GetTypeStringById(DexFileData_t* pDexFileData, uint32_t type_id);
// Proto
const uint8_t* GetProtoShortyStringById(DexFileData_t* pDexFileData, uint32_t proto_id);
const uint8_t* GetProtoReturnTypeStringById(DexFileData_t* pDexFileData, uint32_t proto_id);
const TypeList_t* GetProtoTypeListById(DexFileData_t* pDexFileData, uint32_t proto_id);
uint32_t GetProtoTypeIdOfTypeListByIdx(DexFileData_t* pDexFileData, uint32_t proto_id, uint32_t type_idx);
const uint8_t* GetProtoTypeStringOfTypeListByIdx(DexFileData_t* pDexFileData, uint32_t proto_id, uint32_t type_idx);
// Field
uint32_t GetFieldClassIdById(DexFileData_t* pDexFileData, uint32_t field_id);
const uint8_t* GetFieldClassStringById(DexFileData_t* pDexFileData, uint32_t field_id);
uint32_t GetFieldTypeIdById(DexFileData_t* pDexFileData, uint32_t field_id);
const uint8_t* GetFieldTypeStringById(DexFileData_t* pDexFileData, uint32_t field_id);
const uint8_t* GetFieldNameStringById(DexFileData_t* pDexFileData, uint32_t field_id);
// Method
uint32_t GetMethodClassIdById(DexFileData_t* pDexFileData, uint32_t method_id);
const uint8_t* GetMethodClassStringById(DexFileData_t* pDexFileData, uint32_t method_id);
uint32_t GetMethodProtoIdById(DexFileData_t* pDexFileData, uint32_t method_id);
const uint8_t* GetMethodProtoStringById(DexFileData_t* pDexFileData, uint32_t method_id);
const uint8_t* GetMethodNameStringById(DexFileData_t* pDexFileData, uint32_t method_id);
// Class def
const ClassDef_t* GetClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx);
uint32_t GetClassIdOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx);
const uint8_t* GetClassStringOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx);
uint32_t GetSuperclassIdOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx);
const uint8_t* GetSuperclassStringOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx);
const uint8_t* GetSourcefileStringOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx);
const uint8_t* GetClassDataHdrPtrOfClassDefByIdx(DexFileData_t* pDexFileData, uint32_t class_idx);
const uint8_t* ComputeClassDataHdr(const uint8_t* hdr, ClassDataHdr_t* pClassDataHdr);
// Class data (methods and fields)
const uint8_t* GetStaticFieldOfClassDataByIdx(const ClassDataHdr_t* pClassDataHdr,
                                              const uint8_t* end,
                                              ClassDataField_t* pClassDataField,
                                              uint32_t field_idx);
const uint8_t* GetInstanceFieldOfClassDataByIdx(const ClassDataHdr_t* pClassDataHdr,
                                                const uint8_t* end,
                                                ClassDataField_t* pClassDataField,
                                                uint32_t field_idx);
const uint8_t* GetDirectMethodOfClassDataByIdx(const ClassDataHdr_t* pClassDataHdr,
                                               const uint8_t* end,
                                               ClassDataMethod_t* pClassDataMethod,
                                               uint32_t method_idx);
const uint8_t* GetVirtualMethodOfClassDataByIdx(const ClassDataHdr_t* pClassDataHdr,
                                                const uint8_t* end,
                                                ClassDataMethod_t* pClassDataMethod,
                                                uint32_t method_idx);
const uint8_t* GetMethodNameStringOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM);
uint32_t GetProtoIdOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM);
const uint8_t* GetProtoShortyStringOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM);
const uint8_t* GetProtoReturnTypeStringOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM);
const CodeItem_t* GetDexCodeItemOfCDM(const ClassDataMethod_t* pCDM);
const uint16_t* GetDexCodePtrOfCDM(DexFileData_t* pDexFileData, const ClassDataMethod_t* pCDM);
uint32_t GetAmountOfMethodsByIdx(DexFileData_t* pDexFileData, uint32_t class_idx);
// OAT method
const OatMethodOffsets_t* GetOatMethodOffByIdx(DexFileData_t* pDexFileData,
                                               const uint8_t* oat_base,
                                               uint32_t oat_method_idx);
const uint8_t* GetOatCodePtrByIdx(DexFileData_t* pDexFileData,
                                  const uint8_t* oat_base,
                                  uint32_t class_idx,
                                  uint32_t oat_method_idx);
const uint8_t* GetOatGcMapPtrByIdx(DexFileData_t* pDexFileData,
                                   const uint8_t* oat_base,
                                   uint32_t class_idx,
                                   uint32_t oat_method_idx);
const OatQuickMethodHdr_t* GetOatQuickMethodHdrByIdx(DexFileData_t* pDexFileData,
                                                     const uint8_t* oat_base,
                                                     uint32_t class_idx,
                                                     uint32_t oat_method_idx);

#endif  // CART_OAT_H_

