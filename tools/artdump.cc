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

#define PAGE_SIZE       4096

static void help(void) {
	fprintf(stderr, "Author: Hung, Merck <merckhung@gmail.com>\n");
	fprintf(stderr, "artdump, A raw .art file dumper\n");
	fprintf(stderr, "USAGE: \n   artdump <ART_FILE>\n\n");
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

bool parseArtFile(uint8_t* p, uint32_t sz) {
  ArtHdr_t* pArtHdr = (ArtHdr_t*)p;

  fprintf(stderr, "file size             = 0x%X\n", sz);
  fprintf(stderr, "header size           = 0x%X\n", sizeof(ArtHdr_t));
  fprintf(stderr, "magic_                = \"%c%c%c%c\"\n",
          pArtHdr->magic_[0],
          pArtHdr->magic_[1],
          pArtHdr->magic_[2],
          pArtHdr->magic_[3]);
  fprintf(stderr, "version_              = \"%c%c%c%c\"\n",
          pArtHdr->version_[0],
          pArtHdr->version_[1],
          pArtHdr->version_[2],
          pArtHdr->version_[3]);
  fprintf(stderr, "image_begin_          = 0x%X\n", pArtHdr->image_begin_);
  fprintf(stderr, "image_size_           = 0x%X\n", pArtHdr->image_size_);
  fprintf(stderr, "image_bitmap_offset_  = 0x%X\n", pArtHdr->image_bitmap_offset_);
  fprintf(stderr, "image_bitmap_size_    = 0x%X\n", pArtHdr->image_bitmap_size_);
  fprintf(stderr, "oat_checksum_         = 0x%X\n", pArtHdr->oat_checksum_);
  fprintf(stderr, "oat_file_begin_       = 0x%X\n", pArtHdr->oat_file_begin_);
  fprintf(stderr, "oat_data_begin_       = 0x%X\n", pArtHdr->oat_data_begin_);
  fprintf(stderr, "oat_data_end_         = 0x%X\n", pArtHdr->oat_data_end_);
  fprintf(stderr, "oat_file_end_         = 0x%X\n", pArtHdr->oat_file_end_);
  fprintf(stderr, "patch_delta_          = 0x%X\n", pArtHdr->patch_delta_);
  fprintf(stderr, "image_roots_          = 0x%X\n", pArtHdr->image_roots_);

  return true;
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

	// Open the file
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

	// Parse the .art file
	if (parseArtFile(p, ofs) == false) {
		fprintf(stderr, "Failed to parse .art file %s\n", argv[1]);
		ret = -1;
	}

	// Unmmap the file
	munmap((void*)p, fs);

	// Release resources
	close(fd);

	// Return
	return ret;
}

