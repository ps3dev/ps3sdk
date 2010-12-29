//
// rebuild-elf.c
// by Dan Peori (dan.peori@oopo.net)
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
  if ( argc < 3 )
  {
    printf ( "Usage: %s [self file] [elf file]\n", argv[0] );
    return -1;
  }

  //
  // Open the files.
  //

  FILE *selfFile = fopen ( argv[1], "r" );

  FILE *elfFile = fopen ( argv[2], "w" );

  //
  // Read in the self header.
  //

  SELFHEADER_t selfHeader;

  fseek ( selfFile, 0, SEEK_SET );
  fread ( &selfHeader, sizeof ( SELFHEADER_t ), 1, selfFile );

  //
  // Write out the elf header.
  //

  Elf64_Ehdr elfHeader;

  fseek ( selfFile, fix64 ( selfHeader.elfHeaderOffset ), SEEK_SET );
  fread ( &elfHeader, sizeof ( Elf64_Ehdr ), 1, selfFile );

  fseek ( elfFile, 0, SEEK_SET );
  fwrite ( &elfHeader, sizeof ( Elf64_Ehdr ), 1, elfFile );

  //
  // Write out the section headers.
  //

  Elf64_Shdr sectionHeaders[100];

  fseek ( selfFile, fix64 ( selfHeader.elfSectionHeadersOffset ), SEEK_SET );
  fread ( sectionHeaders, sizeof ( Elf64_Shdr ), fix16 ( elfHeader.e_shnum ), selfFile );

  fseek ( elfFile, fix64 ( elfHeader.e_shoff ), SEEK_SET );
  fwrite ( sectionHeaders, sizeof ( Elf64_Shdr ), fix16 ( elfHeader.e_shnum ), elfFile );

  //
  // Write out the program headers.
  //

  Elf64_Phdr programHeaders[100];

  fseek ( selfFile, fix64 ( selfHeader.elfProgramHeadersOffset ), SEEK_SET );
  fread ( programHeaders, sizeof ( Elf64_Phdr ), fix16 ( elfHeader.e_phnum ), selfFile );

  fseek ( elfFile, fix64 ( elfHeader.e_phoff ), SEEK_SET );
  fwrite ( programHeaders, sizeof ( Elf64_Phdr ), fix16 ( elfHeader.e_phnum ), elfFile );

  //
  // Write out the section data.
  //

  if ( argc == 4 )
  {
    for ( int loop = 0; loop < fix16 ( elfHeader.e_shnum ); loop++ )
    {
      int fix    = atoi ( argv[3] );
      int offset = fix64 ( sectionHeaders[loop].sh_offset );
      int size   = fix64 ( sectionHeaders[loop].sh_size   );

      if ( ( offset > fix ) && ( size > 0 ) )
      {
	uint8_t *sectionBuffer = ( uint8_t * ) malloc ( size );

	fseek ( selfFile, ( offset - fix ), SEEK_SET );
	fread ( sectionBuffer, size, 1, selfFile );

	fseek ( elfFile, offset, SEEK_SET );
	fwrite ( sectionBuffer, size, 1, elfFile );

	free ( sectionBuffer );
      }
    }
  }

  //
  // Write out the program data.
  //

  for ( int loop = 0; loop < fix16 ( elfHeader.e_phnum ); loop++ )
  {
    int offset = fix64 ( programHeaders[loop].p_offset );
    int size   = fix64 ( programHeaders[loop].p_filesz );

    if ( size > 0 )
    {
      char filename[256] = { 0 };

      sprintf ( filename, "program-%03d.bin", loop );
      FILE *programFile = fopen ( filename, "r" );

      if ( programFile )
      {
	uint8_t *programBuffer = ( uint8_t * ) malloc ( size );

	fseek ( programFile, 0, SEEK_SET );
	fread ( programBuffer, size, 1, programFile );

	fseek ( elfFile, offset, SEEK_SET );
	fwrite ( programBuffer, size, 1, elfFile );

	free ( programBuffer );

	fclose ( programFile );
      }
    }
  }

  //
  // Close the files.
  //

  fclose ( elfFile );

  fclose ( selfFile );

  return 0;
}
