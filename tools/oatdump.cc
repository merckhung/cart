#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "../oat.h"
#include "oat_addendum.h"
#include "../elf.h"
#include "coat.h"

#define PAGE_SIZE       4096

static void help(void) {
	fprintf(stderr, "Author: Hung, Merck <merckhung@gmail.com>\n");
	fprintf(stderr, "rawoatdump, A raw OAT dumper\n");
	fprintf(stderr, "USAGE: \n   rawoardump <OAT_FILE>\n\n");
}

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

static inline uint32_t DecodeUnsignedLeb128(const uint8_t** data) {
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

static void DumpData(const uint32_t* Data, uint32_t Length, uint32_t BaseAddr) {
  uint32_t i, j;
  uint32_t c;

  fprintf(stderr, "---------------------------------------------------------------------------\n");
  fprintf(stderr, " Address | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F|   ASCII DATA   \n");
  fprintf(stderr, "---------------------------------------------------------------------------\n");

  for (i = 0; i <= Length; i++) {
    if (!(i % 16)) {
      if (i > 15) {
        for (j = i - 16; j < i; j++) {
          c = ConvertDWordToByte(Data, j);
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
      if (i == Length) {
        break;
      }
      fprintf(stderr, "%8.8X : ", i + BaseAddr);
    }
    fprintf(stderr, "%2.2X ", ConvertDWordToByte(Data, i) & 0xFF);
  }

  if ((i % 16)) {
    for (j = i; j % 16; j++) {
      printf("   ");
    }
    for (j = (i - (i % 16)); j < (i + 16); j++) {
      if (j < i) {
        c = ConvertDWordToByte(Data, j);
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

OatMethodOffsets_t* GetOATClassMethodPtr(uint8_t* p, uint32_t* method_off_ptr, uint32_t idx) {

  printf("[%d]oat_file_addr      = %p\n", idx, p);

  uint32_t oat_class_offset = method_off_ptr[idx];
  printf("[%d]oat_class_offset   = 0x%X\n", idx, oat_class_offset);

  uint8_t* oat_class_pointer = reinterpret_cast<uint8_t*>(p + oat_class_offset);
  printf("[%d]oat_class_pointer  = %p\n", idx, oat_class_pointer);

  uint8_t* status_pointer = oat_class_pointer;
  printf("[%d]status_pointer     = %p\n", idx, status_pointer);

  uint8_t* type_pointer = status_pointer + sizeof(uint16_t);
  printf("[%d]type_pointer       = %p\n", idx, type_pointer);

  uint8_t* after_type_pointer = type_pointer + sizeof(int16_t);

  uint32_t bitmap_size;
  uint8_t* bitmap_pointer;
  uint8_t* methods_pointer = NULL;

  int16_t type = *reinterpret_cast<const int16_t*>(type_pointer);
  printf("[%d]type               = 0x%X\n", idx, type);

  if (type == kOatClassSomeCompiled) {
    bitmap_size = static_cast<uint32_t>(*reinterpret_cast<const uint32_t*>(after_type_pointer));
    printf("[%d]bitmap_size           = 0x%X\n", idx, bitmap_size);
    bitmap_pointer = after_type_pointer + sizeof(bitmap_size);
    printf("[%d]bitmap_pointer        = %p\n", idx, bitmap_pointer);
    methods_pointer = bitmap_pointer + bitmap_size;
  } else {
    methods_pointer = after_type_pointer;
  }
  printf("[%d]methods_pointer    = %p\n\n", idx, methods_pointer);

  return (OatMethodOffsets_t*)methods_pointer;
}

void DumpOATHeader(uint8_t* p, int32_t sz) {
  OatHdr_t* pOatHdr = (OatHdr_t*)p;
  uint8_t* xp;
  uint32_t i;

  if (sz <= 0) {
		return;
  }

  printf("\n=======================================================\n");
  printf("Dumping OAT header\n\n");
  printf("magic                                       = \"%c%c%c%s\"\n",
         pOatHdr->magic_[0],
         pOatHdr->magic_[1],
         pOatHdr->magic_[2],
         (pOatHdr->magic_[3] == '\n') ? "\\n" : "?????");
  printf("version                                     = \"%c%c%c%c\"\n",
         pOatHdr->version_[0],
         pOatHdr->version_[1],
         pOatHdr->version_[2],
         pOatHdr->version_[3]);
  printf("adler32_checksum                            = 0x%X\n",
         pOatHdr->adler32_checksum_);
  printf("instruction_set_                            = 0x%X\n",
         pOatHdr->instruction_set_);
  printf("instruction_set_features_                   = 0x%X\n",
         pOatHdr->instruction_set_features_);
  printf("dex_file_count_                             = 0x%X\n",
         pOatHdr->dex_file_count_);
  printf("executable_offset_                          = 0x%X\n",
         pOatHdr->executable_offset_);
  printf("interpreter_to_interpreter_bridge_offset_   = 0x%X\n",
         pOatHdr->interpreter_to_interpreter_bridge_offset_);
  printf("interpreter_to_compiled_code_bridge_offset_ = 0x%X\n",
         pOatHdr->interpreter_to_compiled_code_bridge_offset_);
  printf("jni_dlsym_lookup_offset_                    = 0x%X\n",
         pOatHdr->jni_dlsym_lookup_offset_);
  printf("portable_imt_conflict_trampoline_offset_    = 0x%X\n",
         pOatHdr->portable_imt_conflict_trampoline_offset_);
  printf("portable_resolution_trampoline_offset_      = 0x%X\n",
         pOatHdr->portable_resolution_trampoline_offset_);
  printf("portable_to_interpreter_bridge_offset_      = 0x%X\n",
         pOatHdr->portable_to_interpreter_bridge_offset_);
  printf("quick_generic_jni_trampoline_offset_        = 0x%X\n",
         pOatHdr->quick_generic_jni_trampoline_offset_);
  printf("quick_imt_conflict_trampoline_offset_       = 0x%X\n",
         pOatHdr->quick_imt_conflict_trampoline_offset_);
  printf("quick_resolution_trampoline_offset_         = 0x%X\n",
         pOatHdr->quick_resolution_trampoline_offset_);
  printf("quick_to_interpreter_bridge_offset_         = 0x%X\n",
         pOatHdr->quick_to_interpreter_bridge_offset_);
  printf("image_patch_delta_                          = 0x%X\n",
         pOatHdr->image_patch_delta_);
  printf("image_file_location_oat_checksum_           = 0x%X\n",
         pOatHdr->image_file_location_oat_checksum_);
  printf("image_file_location_oat_data_begin_         = 0x%X\n",
         pOatHdr->image_file_location_oat_data_begin_);
  printf("key_value_store_size_                       = 0x%X\n",
         pOatHdr->key_value_store_size_);
  printf("key_value_store_                            <DUMPING>\n");

  printf("\n=======================================================\n");
  int32_t kidx = 0;
  bool ret;
  const char* key;
  const char* value;
  do {
    ret = GetStoreKeyValuePairByIndex(pOatHdr->key_value_store_,
                                      pOatHdr->key_value_store_size_,
                                      (size_t)kidx,
                                      &key,
                                      &value);
    if (ret == true) {
      printf("[%d] \"%s\" = \"%s\"\n", kidx, key, value);
    }
    kidx++;
  } while(ret == true);
  printf("=======================================================\n");

  printf("Dump DEX files (%d)......\n", pOatHdr->dex_file_count_);
  xp = reinterpret_cast<uint8_t *>(&pOatHdr->key_value_store_);
  xp += pOatHdr->key_value_store_size_;

  uint32_t dex_loc_sz;
  uint32_t dex_checksum;
  uint32_t dex_file_off;
  uint8_t* dex_file_ptr;
  uint32_t* method_off_ptr;
  DexHdr_t*pDexHdr;

  for (i = 0; i < pOatHdr->dex_file_count_; i++) {
    // Dex file location size
    dex_loc_sz = *reinterpret_cast<const uint32_t*>(xp);
    printf("[%d] dex file location size                  = %d\n", i, dex_loc_sz);

    // Move to dex file location starting point, and then dump the path string
    xp += sizeof(uint32_t);
    DumpData(reinterpret_cast<uint32_t*>(xp), dex_loc_sz, 0);

    // Move to dex file checksum
    xp += dex_loc_sz;
    dex_checksum = *reinterpret_cast<const uint32_t*>(xp);
    printf("[%d] dex checksum                            = 0x%X\n", i, dex_checksum);

    // Move to dex file offset field
    xp += sizeof(uint32_t);
    dex_file_off = *reinterpret_cast<const uint32_t*>(xp);
    printf("[%d] dex offset                              = 0x%X\n", i, dex_file_off);

    // Dex file offset field
    // Dex method pointer
    // ...
    // ...
    // The content of the DEX file

    // Set the Dex file pointer
    dex_file_ptr = (uint8_t*)(p + dex_file_off);
    pDexHdr = reinterpret_cast<DexHdr_t*>(dex_file_ptr);

    // Move to method offsets pointer field
    xp += sizeof(uint32_t);
    method_off_ptr = reinterpret_cast<uint32_t*>(xp);

    printf("\n");
    printf("[%d] dex file ptr                            = %p\n\n", i, dex_file_ptr);
    printf("[%d] method off ptr                          = %p\n", i, method_off_ptr);
    printf("[%d] method offset start                     = 0x%X\n\n", i, (uint32_t)(((uint8_t *)method_off_ptr) - p));

    int32_t total_methods_sz = (sizeof(*method_off_ptr) * pDexHdr->class_defs_size_);
    printf("[%d] Total size of method pointers %d bytes\n", i, total_methods_sz);

    printf("[%d] Dump DEX files (%d) START\n", i, pDexHdr->file_size_);
    printf("[%d] magic_                                  = \"%c%c%c%c%c%c%c%c\"\n",
           i,
           pDexHdr->magic_[0],
           pDexHdr->magic_[1],
           pDexHdr->magic_[2],
           pDexHdr->magic_[3],
           pDexHdr->magic_[4],
           pDexHdr->magic_[5],
           pDexHdr->magic_[6],
           pDexHdr->magic_[7]);
    printf("[%d] checksum_                               = 0x%X\n",
           i, pDexHdr->checksum_);
    //printf("[%d] signature_                             = 0x%X\n",
    //       i, pDexHdr->signature_[0]);
    printf("[%d] file_size_                              = 0x%X\n",
           i, pDexHdr->file_size_);
    printf("[%d] header_size_                            = 0x%X\n",
           i, pDexHdr->header_size_);
    printf("[%d] endian_tag_                             = 0x%X\n",
           i, pDexHdr->endian_tag_);
    printf("[%d] link_size_                              = 0x%X\n",
           i, pDexHdr->link_size_);
    printf("[%d] link_off_                               = 0x%X\n",
           i, pDexHdr->link_off_);
    printf("[%d] map_off_                                = 0x%X\n",
           i, pDexHdr->map_off_);
    printf("[%d] string_ids_size_                        = 0x%X\n",
           i, pDexHdr->string_ids_size_);
    printf("[%d] string_ids_off_                         = 0x%X\n",
           i, pDexHdr->string_ids_off_);
    printf("[%d] type_ids_size_                          = 0x%X\n",
           i, pDexHdr->type_ids_size_);
    printf("[%d] type_ids_off_                           = 0x%X\n",
           i, pDexHdr->type_ids_off_);
    printf("[%d] proto_ids_size_                         = 0x%X\n",
           i, pDexHdr->proto_ids_size_);
    printf("[%d] proto_ids_off_                          = 0x%X\n",
           i, pDexHdr->proto_ids_off_);
    printf("[%d] field_ids_size_                         = 0x%X\n",
           i, pDexHdr->field_ids_size_);
    printf("[%d] field_ids_off_                          = 0x%X\n",
           i, pDexHdr->field_ids_off_);
    printf("[%d] method_ids_size_                        = 0x%X\n",
           i, pDexHdr->method_ids_size_);
    printf("[%d] method_ids_off_                         = 0x%X\n",
           i, pDexHdr->method_ids_off_);
    printf("[%d] class_defs_size_                        = 0x%X\n",
           i, pDexHdr->class_defs_size_);
    printf("[%d] class_defs_off_                         = 0x%X\n",
           i, pDexHdr->class_defs_off_);
    printf("[%d] data_size_                              = 0x%X\n",
           i, pDexHdr->data_size_);
    printf("[%d] data_off_                               = 0x%X\n",
           i, pDexHdr->data_off_);

    DumpData(reinterpret_cast<uint32_t*>(dex_file_ptr), pDexHdr->file_size_, dex_file_off);

    // Move across the dex file
    xp += total_methods_sz;
    printf("[%d] Dump DEX files (%d) END\n", i, pDexHdr->file_size_);

    ClassDefItem_t *pClassDefItem;
    uint8_t* class_data;
    ClassDataItem_t xClassDataItem;
    for (uint32_t idx = 0; idx < pDexHdr->class_defs_size_; idx++) {
      printf("class_def_index = %d\n", idx);

      pClassDefItem = &((ClassDefItem_t *)(pDexHdr->class_defs_off_ + dex_file_ptr))[idx];
      class_data = (uint8_t*)(pClassDefItem->class_data_off + dex_file_ptr);
      OatMethodOffsets_t* pOatMethodOffsets = GetOATClassMethodPtr(p, method_off_ptr, idx);

      printf("pClassDefItem        = %p\n", pClassDefItem);
      printf("class_data           = %p\n\n", class_data);

      printf("class_idx            = 0x%X\n", pClassDefItem->class_idx);
      printf("access_flags         = 0x%X\n", pClassDefItem->access_flags);
      printf("superclass_idx       = 0x%X\n", pClassDefItem->superclass_idx);
      printf("interfaces_off       = 0x%X\n", pClassDefItem->interfaces_off);
      printf("source_file_idx      = 0x%X\n", pClassDefItem->source_file_idx);
      printf("annotations_off      = 0x%X\n", pClassDefItem->annotations_off);
      printf("class_data_off       = 0x%X\n", pClassDefItem->class_data_off);
      printf("static_values_off    = 0x%X\n\n", pClassDefItem->static_values_off);

			DumpData((const uint32_t*)class_data, 16, 0);

			printf("class_data = %p\n", class_data);
      xClassDataItem.static_fields_size = DecodeUnsignedLeb128((const uint8_t **)&class_data);
			printf("class_data = %p\n", class_data);
      xClassDataItem.instance_fields_size = DecodeUnsignedLeb128((const uint8_t **)&class_data);
			printf("class_data = %p\n", class_data);
      xClassDataItem.direct_methods_size = DecodeUnsignedLeb128((const uint8_t **)&class_data);
			printf("class_data = %p\n", class_data);
      xClassDataItem.virtual_methods_size = DecodeUnsignedLeb128((const uint8_t **)&class_data);
			printf("class_data = %p\n", class_data);
      printf("static_fields_size   = 0x%X\n", xClassDataItem.static_fields_size);
      printf("instance_fields_size = 0x%X\n", xClassDataItem.instance_fields_size);
      printf("direct_methods_size  = 0x%X\n", xClassDataItem.direct_methods_size);
      printf("virtual_methods_size = 0x%X\n\n", xClassDataItem.virtual_methods_size);

      uint32_t nr_method = 0;

      if (xClassDataItem.direct_methods_size > 0) {
        xClassDataItem.direct_methods = (EncodedMethod_t *)
          malloc(sizeof(EncodedMethod_t) * xClassDataItem.direct_methods_size);

        for (uint32_t j = 0; j < xClassDataItem.direct_methods_size; j++, nr_method++) {
          xClassDataItem.direct_methods[j].method_idx_diff =
            DecodeUnsignedLeb128((const uint8_t **)&class_data);
          xClassDataItem.direct_methods[j].access_flags =
            DecodeUnsignedLeb128((const uint8_t **)&class_data);
          xClassDataItem.direct_methods[j].code_off =
            DecodeUnsignedLeb128((const uint8_t **)&class_data);

          OatQuickMethodHdr_t *pOatQuickMethodHdr =
            (OatQuickMethodHdr_t *)((pOatMethodOffsets + nr_method)->code_offset_
              - sizeof(OatQuickMethodHdr_t) + p);
          uint32_t* pOatCode = (uint32_t *)((pOatMethodOffsets + nr_method)->code_offset_ + p);

          printf("DIRECT  METHOD[%d] method_idx_diff           = 0x%X\n", j, xClassDataItem.direct_methods[j].method_idx_diff);
          printf("DIRECT  METHOD[%d] access_flags              = 0x%X\n", j, xClassDataItem.direct_methods[j].access_flags);
          printf("DIRECT  METHOD[%d] code_off                  = 0x%X\n\n", j, xClassDataItem.direct_methods[j].code_off);
          printf("DIRECT  METHOD[%d] registers_size            = 0x%X\n", j, xClassDataItem.direct_methods[j].xcode_item.registers_size);
          printf("DIRECT  METHOD[%d] ins_size                  = 0x%X\n", j, xClassDataItem.direct_methods[j].xcode_item.ins_size);
          printf("DIRECT  METHOD[%d] outs_size                 = 0x%X\n", j, xClassDataItem.direct_methods[j].xcode_item.outs_size);
          printf("DIRECT  METHOD[%d] tries_size                = 0x%X\n", j, xClassDataItem.direct_methods[j].xcode_item.tries_size);
          printf("DIRECT  METHOD[%d] debug_info_off            = 0x%X\n", j, xClassDataItem.direct_methods[j].xcode_item.debug_info_off);
          printf("DIRECT  METHOD[%d] insns_size                = 0x%X\n\n", j, xClassDataItem.direct_methods[j].xcode_item.insns_size);
          printf("DIRECT  METHOD[%d] OAT method code off       = 0x%X\n", j, (pOatMethodOffsets + nr_method)->code_offset_);
          printf("DIRECT  METHOD[%d] OAT GC MAP off            = 0x%X\n", j, (pOatMethodOffsets + nr_method)->gc_map_offset_);
          printf("DIRECT  METHOD[%d] OAT code_size_            = 0x%X\n", j, pOatQuickMethodHdr->code_size_);
          printf("DIRECT  METHOD[%d] OAT mapping_table_offset_ = 0x%X\n", j, pOatQuickMethodHdr->mapping_table_offset_);
          printf("DIRECT  METHOD[%d] OAT vmap_table_offset_    = 0x%X\n", j, pOatQuickMethodHdr->vmap_table_offset_);
          printf("DIRECT  METHOD[%d] OAT frame_size_in_bytes_  = 0x%X\n", j, pOatQuickMethodHdr->frame_info_.frame_size_in_bytes_);
          printf("DIRECT  METHOD[%d] OAT core_spill_mask_      = 0x%X\n", j, pOatQuickMethodHdr->frame_info_.core_spill_mask_);
          printf("DIRECT  METHOD[%d] OAT fp_spill_mask_        = 0x%X\n\n", j, pOatQuickMethodHdr->frame_info_.fp_spill_mask_);
          printf("DIRECT  METHOD[%d] OAT code dumping START\n", j);
          DumpData(pOatCode, pOatQuickMethodHdr->code_size_, (pOatMethodOffsets + nr_method)->code_offset_);
          printf("DIRECT  METHOD[%d] OAT code dumping END\n\n", j);
        }
      }

      if (xClassDataItem.virtual_methods_size > 0) {
        xClassDataItem.virtual_methods = (EncodedMethod_t *)
          malloc(sizeof(EncodedMethod_t) * xClassDataItem.virtual_methods_size);

        for (uint32_t j = 0; j < xClassDataItem.virtual_methods_size; j++, nr_method++) {
          xClassDataItem.virtual_methods[j].method_idx_diff =
            DecodeUnsignedLeb128((const uint8_t **)&class_data);
          xClassDataItem.virtual_methods[j].access_flags =
            DecodeUnsignedLeb128((const uint8_t **)&class_data);
          xClassDataItem.virtual_methods[j].code_off =
            DecodeUnsignedLeb128((const uint8_t **)&class_data);
          OatQuickMethodHdr_t *pOatQuickMethodHdr =
            (OatQuickMethodHdr_t *)((pOatMethodOffsets + nr_method)->code_offset_
              - sizeof(OatQuickMethodHdr_t) + p);
          uint32_t* pOatCode = (uint32_t *)((pOatMethodOffsets + nr_method)->code_offset_ + p);

          printf("VIRTUAL METHOD[%d] method_idx_diff           = 0x%X\n", j, xClassDataItem.virtual_methods[j].method_idx_diff);
          printf("VIRTUAL METHOD[%d] access_flags              = 0x%X\n", j, xClassDataItem.virtual_methods[j].access_flags);
          printf("VIRTUAL METHOD[%d] code_off                  = 0x%X\n\n", j, xClassDataItem.virtual_methods[j].code_off);
          printf("VIRTUAL METHOD[%d] registers_size            = 0x%X\n", j, xClassDataItem.virtual_methods[j].xcode_item.registers_size);
          printf("VIRTUAL METHOD[%d] ins_size                  = 0x%X\n", j, xClassDataItem.virtual_methods[j].xcode_item.ins_size);
          printf("VIRTUAL METHOD[%d] outs_size                 = 0x%X\n", j, xClassDataItem.virtual_methods[j].xcode_item.outs_size);
          printf("VIRTUAL METHOD[%d] tries_size                = 0x%X\n", j, xClassDataItem.virtual_methods[j].xcode_item.tries_size);
          printf("VIRTUAL METHOD[%d] debug_info_off            = 0x%X\n", j, xClassDataItem.virtual_methods[j].xcode_item.debug_info_off);
          printf("VIRTUAL METHOD[%d] insns_size                = 0x%X\n\n", j, xClassDataItem.virtual_methods[j].xcode_item.insns_size);
          printf("VIRTUAL METHOD[%d] OAT method code off       = 0x%X\n", j, (pOatMethodOffsets + nr_method)->code_offset_);
          printf("VIRTUAL METHOD[%d] OAT GC MAP off            = 0x%X\n", j, (pOatMethodOffsets + nr_method)->gc_map_offset_);
          printf("VIRTUAL METHOD[%d] OAT code_size_            = 0x%X\n", j, pOatQuickMethodHdr->code_size_);
          printf("VIRTUAL METHOD[%d] OAT mapping_table_offset_ = 0x%X\n", j, pOatQuickMethodHdr->mapping_table_offset_);
          printf("VIRTUAL METHOD[%d] OAT vmap_table_offset_    = 0x%X\n", j, pOatQuickMethodHdr->vmap_table_offset_);
          printf("VIRTUAL METHOD[%d] OAT frame_size_in_bytes_  = 0x%X\n", j, pOatQuickMethodHdr->frame_info_.frame_size_in_bytes_);
          printf("VIRTUAL METHOD[%d] OAT core_spill_mask_      = 0x%X\n", j, pOatQuickMethodHdr->frame_info_.core_spill_mask_);
          printf("VIRTUAL METHOD[%d] OAT fp_spill_mask_        = 0x%X\n\n", j, pOatQuickMethodHdr->frame_info_.fp_spill_mask_);
          printf("DIRECT  METHOD[%d] OAT code dumping START\n", j);
          DumpData(pOatCode, pOatQuickMethodHdr->code_size_, (pOatMethodOffsets + nr_method)->code_offset_);
          printf("DIRECT  METHOD[%d] OAT code dumping END\n\n", j);
        }
      }

      printf("Number of methods of class[%d]               = %d\n", idx, nr_method);
    }
  }
}

int parseElf32File(uint8_t* p, uint32_t ofs) {
	Elf32Hdr_t* pElfHdr = (Elf32Hdr_t *)p;
	Elf32ProgHdr_t* pElfProgHdr;
	Elf32SecHdr_t* pElfSecHdr;
	int32_t i, j, text_idx = 0, rodata_idx = 0;
	uint8_t *str;
	//uint8_t *ptxt;
	uint8_t *phdr;

	if (!p) {
		fprintf(stderr, "A null pointer is received\n");
		return -1;
	}

	// Roughly check the file size
	if (ofs < sizeof(Elf32Hdr_t)) {
		fprintf(stderr, "ELF32 file size is too short\n");
		return -1;
	}

	// Identify the file format
	if ((pElfHdr->e_ident.EI_MAG[0] != 0x7F)
			|| (pElfHdr->e_ident.EI_MAG[1] != 'E')
			|| (pElfHdr->e_ident.EI_MAG[2] != 'L')
			|| (pElfHdr->e_ident.EI_MAG[3] != 'F')
			|| (pElfHdr->e_ident.EI_CLASS != 1)
			|| (pElfHdr->e_ident.EI_DATA != 1)
			|| (pElfHdr->e_ident.EI_VERSION != 1)) {
		fprintf(stderr, "Invalid ELF32 magic string\n");
		return -1;
	}

	// Print out debug message
	printf("e_type      = 0x%4.4X\n", pElfHdr->e_type);
	printf("e_machine   = 0x%4.4X\n", pElfHdr->e_machine);
	printf("e_version   = 0x%8.8X\n", pElfHdr->e_version);
	printf("e_entry     = 0x%8.8X\n", pElfHdr->e_entry);
	printf("e_phoff     = 0x%8.8X\n", pElfHdr->e_phoff);
	printf("e_shoff     = 0x%8.8X\n", pElfHdr->e_shoff);
	printf("e_flags     = 0x%8.8X\n", pElfHdr->e_flags);
	printf("e_ehsize    = 0x%4.4X\n", pElfHdr->e_ehsize);
	printf("e_phentsize = 0x%4.4X\n", pElfHdr->e_phentsize);
	printf("e_phnum     = 0x%4.4X\n", pElfHdr->e_phnum);
	printf("e_shentsize = 0x%4.4X\n", pElfHdr->e_shentsize);
	printf("e_shnum     = 0x%4.4X\n", pElfHdr->e_shnum);
	printf("e_shstrndx  = 0x%4.4X\n", pElfHdr->e_shstrndx);
	printf("=============================================\n");

	printf("ELF32 Header Size        : 0x%4.4X\n", pElfHdr->e_ehsize);
	printf("Program Entry Address    : 0x%8.8X\n", pElfHdr->e_entry);
	printf("Program Header Offset    : 0x%8.8X\n", pElfHdr->e_phoff);
	printf("Program Header Size      : 0x%4.4X\n", pElfHdr->e_phentsize);
	printf("Number of Program Headers: %d\n", pElfHdr->e_phnum);
	printf("Section Header Offset    : 0x%8.8X\n", pElfHdr->e_shoff);
	printf("Section Header Size      : 0x%4.4X\n", pElfHdr->e_shentsize);
	printf("Number of Section Headers: %d\n", pElfHdr->e_shnum);

	// Iterate each program header
	pElfProgHdr = (Elf32ProgHdr_t *)(p + pElfHdr->e_phoff);
	for (i = 0; i < pElfHdr->e_phnum; i++) {
		printf("Segment descriptor type : %d(0x%8.8X)\n",
			     (pElfProgHdr + i)->p_type, (pElfProgHdr + i)->p_type);
		printf("Flags for segment       : 0x%8.8X\n", (pElfProgHdr + i)->p_flags);
		printf("File offset of segment  : 0x%8.8X\n", (pElfProgHdr + i)->p_offset);
		printf(">======================================<\n");
	}
	printf("=============================================\n");

	// Iterate each section header
	pElfSecHdr = (Elf32SecHdr_t *)(p + pElfHdr->e_shoff);
	for (i = 0; i < pElfHdr->e_shnum; ++i) {
    printf("Section name index      : %d\n", (pElfSecHdr + i)->sh_name);

		str = p + (pElfSecHdr + pElfHdr->e_shstrndx)->sh_offset + (pElfSecHdr + i)->sh_name;
		printf("Section name            : %s\n", str);
		if (!strcmp((const char*)str, ".text")) {
			text_idx = i;
			printf("\nFound .text section at index %d\n\n", i);
		}

		if (!strcmp((const char*)str, ".rodata")) {
      rodata_idx = i;
      printf("\nFound .rodata section at index %d\n\n", i);
    }

    printf("Section type            : 0x%8.8X\n", (pElfSecHdr + i)->sh_type);
    printf("Section flags           : 0x%8.8X\n", (pElfSecHdr + i)->sh_flags);
		printf("Address of 1st byte     : 0x%8.8X\n", (pElfSecHdr + i)->sh_addr);
		printf("File offset of section  : 0x%8.8X\n", (pElfSecHdr + i)->sh_offset);
		printf("Section size in bytes   : %d bytes\n", (pElfSecHdr + i)->sh_size);

		if ((pElfSecHdr + i)->sh_type == SHT_STRTAB) {
			j = i;
			printf("\nFound a String Table at index %d\n\n", j);
		}

    printf(">======================================<\n");
  }

	printf("=============================================\n");
	printf(".text section in file offset = 0x%X, size = 0x%X\n",
	       (pElfSecHdr + text_idx)->sh_offset,
	       (pElfSecHdr + text_idx)->sh_size);
	//ptxt = p + (pElfSecHdr + text_idx)->sh_offset;
	printf(".rodata section in file offset = 0x%X, size = 0x%X\n",
	       (pElfSecHdr + rodata_idx)->sh_offset,
	       (pElfSecHdr + rodata_idx)->sh_size);
	phdr = p + (pElfSecHdr + rodata_idx)->sh_offset;
	printf("=============================================\n");

	DumpOATHeader(phdr, (pElfSecHdr + rodata_idx)->sh_size);

	return 0;
}

int main(int argc, char** argv) {
	int32_t ret = 0;
	int32_t fd;
	int32_t fs, ofs;
	uint8_t* p = NULL;

	// Check the input arguments
	if( argc != 2 ) {
		help();
		return 0;
	}

	// Open the ELF32 file
	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open file %s\n", argv[1]);
		return fd;
	}

	// Get filesize
	ofs = fs = lseek(fd, 0, SEEK_END);
	if ((lseek(fd, 0, SEEK_SET) == -1) || (fs == -1)) {
    fprintf(stderr, "Failed to get the file size\n");
    return errno;
  }
	printf("File size is %d bytes\n", fs);

	// Align 4KB boundary
	if (fs % PAGE_SIZE) {
		fs = ((fs / PAGE_SIZE) + 1) * PAGE_SIZE;
  }
	printf("Aligned file size is %d bytes\n", fs);

	// Mmap the file
	p = (uint8_t*)mmap(NULL, fs, PROT_READ, MAP_PRIVATE, fd, 0);

	// Parse the ELF32 ARM executable file
	if (parseElf32File(p, ofs)) {
		fprintf(stderr, "Failed to parse ELF32 file %s\n", argv[1]);
		ret = -1;
	}

	// Unmmap the file
	munmap((void*)p, fs);

	// Release resources
	close(fd);

	// Return
	return ret;
}

