//
// readself.c
// by Naomi Peori (naomi@peori.ca)
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <byteswap.h>

#include <elf.h>
#include "self.h"

#ifdef __BIG_ENDIAN__
#define fix16(value) value
#define fix32(value) value
#define fix64(value) (long long unsigned)value
#else
#define fix16(value) bswap_16(value)
#define fix32(value) bswap_32(value)
#define fix64(value) (long long unsigned)bswap_64(value)
#endif

int main ( int argc, char **argv )
{
  if ( argc != 2 )
  {
    printf ( "Usage: %s <self file>\n", argv[0] );
    return -1;
  }

  //
  // Read in the SELF data.
  //

  SELFHEADER_t selfHeader;

  Elf64_Ehdr elfHeader;
  Elf64_Phdr programHeaders[100];
  Elf64_Shdr sectionHeaders[100];

  FILE *infile = fopen ( argv[1], "r" );

  if ( infile )
  {
    fseek ( infile, 0, SEEK_SET );
    fread ( &selfHeader, sizeof ( selfHeader ), 1, infile );

    fseek ( infile, fix64(selfHeader.elfHeaderOffset), SEEK_SET );
    fread ( &elfHeader, sizeof ( Elf64_Ehdr ), 1, infile );

    fseek ( infile, fix64 ( selfHeader.elfProgramHeadersOffset ), SEEK_SET );
    fread ( programHeaders, sizeof ( Elf64_Phdr ), fix16 ( elfHeader.e_phnum ), infile );

    fseek ( infile, fix64 ( selfHeader.elfSectionHeadersOffset ), SEEK_SET );
    fread ( sectionHeaders, sizeof ( Elf64_Shdr ), fix16 ( elfHeader.e_shnum ), infile );

    fclose ( infile );
  }

  //
  // Output the SELF data.
  //

  printf ( "[SELFHEADER]\n" );
  printf ( "  unknown01               = 0x%08X\n",    fix32 ( selfHeader.unknown01               ) );
  printf ( "  unknown02               = 0x%08X\n",    fix32 ( selfHeader.unknown02               ) );
  printf ( "  metadataInfoOffset      = 0x%08X\n",    fix32 ( selfHeader.metadataInfoOffset      ) );
  printf ( "  fileOffset              = 0x%016llX\n", fix64 ( selfHeader.fileOffset              ) );
  printf ( "  fileSize                = 0x%016llX\n", fix64 ( selfHeader.fileSize                ) );
  printf ( "  unknown06               = 0x%016llX\n", fix64 ( selfHeader.unknown06               ) );
  printf ( "  programInfoOffset       = 0x%016llX\n", fix64 ( selfHeader.programInfoOffset       ) );
  printf ( "  elfHeaderOffset         = 0x%016llX\n", fix64 ( selfHeader.elfHeaderOffset         ) );
  printf ( "  elfProgramHeadersOffset = 0x%016llX\n", fix64 ( selfHeader.elfProgramHeadersOffset ) );
  printf ( "  elfSectionHeadersOffset = 0x%016llX\n", fix64 ( selfHeader.elfSectionHeadersOffset ) );
  printf ( "  unknown11               = 0x%016llX\n", fix64 ( selfHeader.unknown11               ) );
  printf ( "  unknown12               = 0x%016llX\n", fix64 ( selfHeader.unknown12               ) );
  printf ( "  controlInfoOffset       = 0x%016llX\n", fix64 ( selfHeader.controlInfoOffset       ) );
  printf ( "  controlInfoSize         = 0x%016llX\n", fix64 ( selfHeader.controlInfoSize         ) );

  printf ( "\n[ELFHEADER]\n" );
  printf ( "  e_type      = 0x%04X\n",    fix16 ( elfHeader.e_type      ) );
  printf ( "  e_machine   = 0x%04X\n",    fix16 ( elfHeader.e_machine   ) );
  printf ( "  e_version   = 0x%08X\n",    fix32 ( elfHeader.e_version   ) );
  printf ( "  e_entry     = 0x%016llX\n", fix64 ( elfHeader.e_entry     ) );
  printf ( "  e_phoff     = 0x%016llX\n", fix64 ( elfHeader.e_phoff     ) );
  printf ( "  e_shoff     = 0x%016llX\n", fix64 ( elfHeader.e_shoff     ) );
  printf ( "  e_flags     = 0x%08X\n",    fix32 ( elfHeader.e_flags     ) );
  printf ( "  e_ehsize    = 0x%04X\n",    fix16 ( elfHeader.e_ehsize    ) );
  printf ( "  e_phentsize = 0x%04X\n",    fix16 ( elfHeader.e_phentsize ) );
  printf ( "  e_phnum     = 0x%04X\n",    fix16 ( elfHeader.e_phnum     ) );
  printf ( "  e_shentsize = 0x%04X\n",    fix16 ( elfHeader.e_shentsize ) );
  printf ( "  e_shnum     = 0x%04X\n",    fix16 ( elfHeader.e_shnum     ) );
  printf ( "  e_shstrndx  = 0x%04X\n",    fix16 ( elfHeader.e_shstrndx  ) );

  for ( int loop = 0; loop < fix16 ( elfHeader.e_phnum ); loop++ )
  {
    printf ( "\n[PROGRAM 0x%02X]\n", loop );
    printf ( "  p_type   = 0x%08X\n",    fix32 ( programHeaders[loop].p_type   ) );
    printf ( "  p_flags  = 0x%08X\n",    fix32 ( programHeaders[loop].p_flags  ) );
    printf ( "  p_offset = 0x%016llX\n", fix64 ( programHeaders[loop].p_offset ) );
    printf ( "  p_vaddr  = 0x%016llX\n", fix64 ( programHeaders[loop].p_vaddr  ) );
    printf ( "  p_paddr  = 0x%016llX\n", fix64 ( programHeaders[loop].p_paddr  ) );
    printf ( "  p_filesz = 0x%016llX\n", fix64 ( programHeaders[loop].p_filesz ) );
    printf ( "  p_memsz  = 0x%016llX\n", fix64 ( programHeaders[loop].p_memsz  ) );
    printf ( "  p_align  = 0x%016llX\n", fix64 ( programHeaders[loop].p_align  ) );
  }

  for ( int loop = 0; loop < fix16 ( elfHeader.e_shnum ); loop++ )
  {
    printf ( "\n[SECTION 0x%02X]\n", loop );
    printf ( "  sh_name      = 0x%08X\n",    fix32 ( sectionHeaders[loop].sh_name      ) );
    printf ( "  sh_type      = 0x%08X\n",    fix32 ( sectionHeaders[loop].sh_type      ) );
    printf ( "  sh_flags     = 0x%016llX\n", fix64 ( sectionHeaders[loop].sh_flags     ) );
    printf ( "  sh_addr      = 0x%016llX\n", fix64 ( sectionHeaders[loop].sh_addr      ) );
    printf ( "  sh_offset    = 0x%016llX\n", fix64 ( sectionHeaders[loop].sh_offset    ) );
    printf ( "  sh_size      = 0x%016llX\n", fix64 ( sectionHeaders[loop].sh_size      ) );
    printf ( "  sh_link      = 0x%08X\n",    fix32 ( sectionHeaders[loop].sh_link      ) );
    printf ( "  sh_info      = 0x%08X\n",    fix32 ( sectionHeaders[loop].sh_info      ) );
    printf ( "  sh_addralign = 0x%016llX\n", fix64 ( sectionHeaders[loop].sh_addralign ) );
    printf ( "  sh_entsize   = 0x%016llX\n", fix64 ( sectionHeaders[loop].sh_entsize   ) );
  }

  return 0;
}
