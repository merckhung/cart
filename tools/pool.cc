#include <memory>

#include <stdio.h>

#include "decompressed_code.h"

static size_t Hash(const char* s) {
  // This is the java.lang.String hashcode for convenience, not interoperability.
  size_t hash = 0;
  for (; *s != '\0'; ++s) {
    hash = hash * 31 + *s;
  }
  return hash;
}

int main(int argc, char** argv) {
  const char* list[] = {"ABC", "DEF", "RTY", "989", "JAVA", "NEWMY"};
  size_t sz[] = {92, 486, 128, 32, 1040, 32};
  int nr = 6;
  void* ptr;
  std::unique_ptr<cart::DecompressedCode> dc;

  dc.reset(cart::DecompressedCode::LoadFile("/tmp/TestCOAT.coat_cache"));
  if (dc.get() == nullptr) {
    fprintf(stderr, "Failed to load COAT cache, will create a new one\n");
    dc.reset(cart::DecompressedCode::Create("/tmp/TestCOAT.coat_cache"));
  }
  if (dc.get() == nullptr) {
    fprintf(stderr, "Failed to create COAT cache\n");
    return -1;
  }
  dc->DumpRecord();
  fprintf(stderr, "======================================\n\n");

	for (int i = 0; i < nr; ++i) {
	  ptr = dc->GetPtr(Hash(list[i]));
	  if (ptr != nullptr) {
	    printf("(1)ptr of Hash(%s) = %p\n", list[i], ptr);
	    continue;
	  } else {
	    printf("Allocating memory for Hash(%s), size = %d\n", list[i], sz[i]);
	    ptr = dc->Alloc(Hash(list[i]), sz[i]);
	    if (ptr == nullptr) {
	      printf("Out of Memory\n");
	      break;
	    }
	    printf("(2)ptr of Hash(%s) = %p\n", list[i], ptr);
	    ptr = dc->GetPtr(Hash(list[i]));
	    printf("(3)ptr of Hash(%s) = %p\n\n", list[i], ptr);
	  }
	}

	for (int i = 0; i < nr; ++i) {
    ptr = dc->GetPtr(Hash(list[i]));
    if (ptr != nullptr) {
      printf("(1)ptr of Hash(%s) = %p\n", list[i], ptr);
      continue;
    } else {
      printf("Allocating memory for Hash(%s), size = %d\n", list[i], sz[i]);
      ptr = dc->Alloc(Hash(list[i]), sz[i]);
      if (ptr == nullptr) {
        printf("Out of Memory\n");
        break;
      }
      printf("(2)ptr of Hash(%s) = %p\n", list[i], ptr);
      ptr = dc->GetPtr(Hash(list[i]));
      printf("(3)ptr of Hash(%s) = %p\n\n", list[i], ptr);
    }
  }

	//dc->DumpBitmap();
	//dc->DumpRecord();

  for (int i = 0; i < nr; ++i) {
    ptr = dc->GetPtr(Hash(list[i]));
    if (ptr == nullptr) {
      printf("Cannot find (1)ptr of Hash(%s)\n", list[i]);
      continue;
    }
    printf("(1)ptr of Hash(%s) = %p\n", list[i], ptr);
    printf("Deallocating memory for Hash(%s), size = %d\n", list[i], sz[i]);
    dc->Dealloc(Hash(list[i]));
    ptr = dc->GetPtr(Hash(list[i]));
    printf("(2)ptr of Hash(%s) = %p\n\n", list[i], ptr);
  }

	//dc->DumpBitmap();
  //dc->DumpRecord();

  for (int i = 0; i < nr; ++i) {
    ptr = dc->GetPtr(Hash(list[i]));
    if (ptr != nullptr) {
      printf("(1)ptr of Hash(%s) = %p\n", list[i], ptr);
      continue;
    } else {
      printf("Allocating memory for Hash(%s), size = %d\n", list[i], sz[i]);
      ptr = dc->Alloc(Hash(list[i]), sz[i]);
      if (ptr == nullptr) {
        printf("Out of Memory\n");
        break;
      }
      printf("(2)ptr of Hash(%s) = %p\n", list[i], ptr);
      ptr = dc->GetPtr(Hash(list[i]));
      printf("(3)ptr of Hash(%s) = %p\n\n", list[i], ptr);
    }
  }

  //dc->DumpBitmap();
  //dc->DumpRecord();

	return 0;
}

