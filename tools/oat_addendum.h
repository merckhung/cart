
typedef struct {
  uint32_t class_idx;
  uint32_t access_flags;
  uint32_t superclass_idx; /* index into typeIds for superclass */
  uint32_t interfaces_off; /* file offset to DexTypeList */
  uint32_t source_file_idx; /* index into stringIds for source file name */
  uint32_t annotations_off; /* file offset to annotations_directory_item */
  uint32_t class_data_off; /* file offset to class_data_item */
  uint32_t static_values_off; /* file offset to DexEncodedArray */
} ClassDefItem_t;

/* class defs */
typedef struct {
  uint16_t registers_size;
  uint16_t ins_size;
  uint16_t outs_size;
  uint16_t tries_size;
  uint32_t debug_info_off;
  uint32_t insns_size;
  uint16_t *insns;
  /*
  uint16_t padding;
  try_item
  handlers
  */
} CodeItemOld_t;

typedef struct {
  uint32_t method_idx_diff;
  uint32_t access_flags;
  uint32_t code_off;
  CodeItemOld_t xcode_item;
} EncodedMethod_t;

typedef struct {
  uint32_t static_fields_size;
  uint32_t instance_fields_size;
  uint32_t direct_methods_size;
  uint32_t virtual_methods_size;
  EncodedMethod_t *direct_methods;
  EncodedMethod_t *virtual_methods;
} ClassDataItem_t;

