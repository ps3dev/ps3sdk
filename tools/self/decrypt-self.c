//
// decrypt-self.c
// by Dan Peori (dan.peori@oopo.net)
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/aes.h>

#include <zlib.h>

#include <elf.h>
#include "self.h"

#include <byteswap.h>

#ifdef __BIG_ENDIAN__
#define fix16(value) value
#define fix32(value) value
#define fix64(value) (uint64_t)value
#else
#define fix16(value) bswap_16(value)
#define fix32(value) bswap_32(value)
#define fix64(value) (uint64_t)bswap_64(value)
#endif

uint8_t erk[] = { /* REDACTED */ };
uint8_t riv[] = { /* REDACTED */ };

int decompress ( uint8_t *source, int sourceSize, uint8_t *destination, int destinationSize )
{
  z_stream stream;

  stream.opaque = Z_NULL;
  stream.zalloc = Z_NULL;
  stream.zfree  = Z_NULL;

  if ( inflateInit ( &stream ) != Z_OK ) { return -1; }

  stream.next_in = source;
  stream.avail_in = sourceSize;

  stream.next_out = destination;
  stream.avail_out = destinationSize;

  int result = inflate ( &stream, Z_NO_FLUSH );

  inflateEnd ( &stream );

  return result;
}

int main ( int argc, char **argv )
{
  if ( argc != 4 )
  {
    printf ( "Usage: %s [self file] [elf file] [fix]\n", argv[0] );
    return -1;
  }

  //
  // Open the files.
  //

  FILE *selfFile = fopen ( argv[1], "r" );
  if ( selfFile == NULL ) { printf ( "ERROR: Could not open the SELF file. (%s)\n", argv[1] ); }

  FILE *elfFile = fopen ( argv[2], "w" );
  if ( elfFile == NULL ) { printf ( "ERROR: Could not open the ELF file. (%s)\n", argv[2] ); }

  //
  // Read in the self header.
  //

  SELFHEADER_t selfHeader;

  fseek ( selfFile, 0, SEEK_SET );
  fread ( &selfHeader, sizeof ( SELFHEADER_t ), 1, selfFile );

  //
  // Read in and write out the elf header.
  //

  Elf64_Ehdr elfHeader;

  fseek ( selfFile, fix64 ( selfHeader.elfHeaderOffset ), SEEK_SET );
  fread ( &elfHeader, sizeof ( Elf64_Ehdr ), 1, selfFile );

  fseek ( elfFile, 0, SEEK_SET );
  fwrite ( &elfHeader, sizeof ( Elf64_Ehdr ), 1, elfFile );

  //
  // Read in and write out the program headers.
  //

  Elf64_Phdr elfProgramHeaders[100];

  fseek ( selfFile, fix64 ( selfHeader.elfProgramHeadersOffset ), SEEK_SET );
  fread ( elfProgramHeaders, sizeof ( Elf64_Phdr ), fix16 ( elfHeader.e_phnum ), selfFile );

  fseek ( elfFile, fix64 ( elfHeader.e_phoff ), SEEK_SET );
  fwrite ( elfProgramHeaders, sizeof ( Elf64_Phdr ), fix16 ( elfHeader.e_phnum ), elfFile );

  //
  // Read in and write out the section headers.
  //

  Elf64_Shdr elfSectionHeaders[100];

  fseek ( selfFile, fix64 ( selfHeader.elfSectionHeadersOffset ), SEEK_SET );
  fread ( elfSectionHeaders, sizeof ( Elf64_Shdr ), fix16 ( elfHeader.e_shnum ), selfFile );

  fseek ( elfFile, fix64 ( elfHeader.e_shoff ), SEEK_SET );
  fwrite ( elfSectionHeaders, sizeof ( Elf64_Shdr ), fix16 ( elfHeader.e_shnum ), elfFile );

  //
  // Read in and decrypt the metadata info.
  //

  METADATAINFO_t metadataInfo;
  METADATAINFO_t metadataInfo_d;

  fseek ( selfFile, fix32 ( selfHeader.metadataInfoOffset ), SEEK_SET );
  fread ( &metadataInfo, sizeof ( METADATAINFO_t ), 1, selfFile );

  AES_KEY aesDecryptKey;

  AES_set_decrypt_key ( erk, 256, &aesDecryptKey );

  AES_cbc_encrypt ( metadataInfo.key,  metadataInfo_d.key,  32, &aesDecryptKey, riv, AES_DECRYPT );
  AES_cbc_encrypt ( metadataInfo.ivec, metadataInfo_d.ivec, 32, &aesDecryptKey, riv, AES_DECRYPT );

  printf ( "\nmetadataInfo:\n" );
  printf ( "  unknown00: "); for ( int loop = 0; loop < 32; loop++ ) { printf ( "%02X ", metadataInfo_d.unknown00[loop] ); } printf ( "\n" );
  printf ( "  key:       "); for ( int loop = 0; loop < 32; loop++ ) { printf ( "%02X ", metadataInfo_d.key[loop]       ); } printf ( "\n" );
  printf ( "  ivec:      "); for ( int loop = 0; loop < 32; loop++ ) { printf ( "%02X ", metadataInfo_d.ivec[loop]      ); } printf ( "\n" );

  //
  // Read in and decrypt the metadata header.
  //

  METADATAHEADER_t metadataHeader;
  METADATAHEADER_t metadataHeader_d;

  fread ( &metadataHeader, sizeof ( METADATAHEADER_t ), 1, selfFile );

  AES_KEY aesEncryptKey;

  AES_set_encrypt_key ( metadataInfo_d.key, 128, &aesEncryptKey );

  uint8_t ecount[AES_BLOCK_SIZE];
  uint32_t num = 0;

  AES_ctr128_encrypt ( ( uint8_t * ) &metadataHeader, ( uint8_t * ) &metadataHeader_d, sizeof ( METADATAHEADER_t ), &aesEncryptKey, metadataInfo_d.ivec, ecount, &num );

  printf ( "\nmetadataHeader:\n" );
  printf ( "  unknown00    = %x\n", fix32(metadataHeader_d.unknown00)    );
  printf ( "  size         = %x\n", fix32(metadataHeader_d.size)         );
  printf ( "  unknown02    = %x\n", fix32(metadataHeader_d.unknown02)    );
  printf ( "  sectionCount = %x\n", fix32(metadataHeader_d.sectionCount) );
  printf ( "  KeyCount     = %x\n", fix32(metadataHeader_d.keyCount)     );
  printf ( "  unknown05    = %x\n", fix32(metadataHeader_d.unknown05)    );
  printf ( "  unknown06    = %x\n", fix32(metadataHeader_d.unknown06)    );
  printf ( "  unknown07    = %x\n", fix32(metadataHeader_d.unknown07)    );

  //
  // Read in and decrypt the metadata section headers.
  //

  METADATASECTIONHEADER_t metadataSectionHeaders[100];
  METADATASECTIONHEADER_t metadataSectionHeaders_d[100];

  fread ( metadataSectionHeaders, sizeof ( METADATASECTIONHEADER_t ), fix32 ( metadataHeader_d.sectionCount ), selfFile );

  AES_ctr128_encrypt ( ( uint8_t * ) metadataSectionHeaders, ( uint8_t * ) metadataSectionHeaders_d, sizeof ( METADATASECTIONHEADER_t ) * fix32 ( metadataHeader_d.sectionCount ), &aesEncryptKey, metadataInfo_d.ivec, ecount, &num );

  for ( int loop = 0; loop < fix32 ( metadataHeader_d.sectionCount ); loop++ )
  {
    printf ( "\nmetadataSectionHeaders[%02X]\n", loop );
    printf ( "  dataOffset    = %lX\n", fix64 ( metadataSectionHeaders_d[loop].dataOffset   ) );
    printf ( "  dataSize      = %lX\n", fix64 ( metadataSectionHeaders_d[loop].dataSize     ) );
    printf ( "  unknown02     = %X\n",  fix32 ( metadataSectionHeaders_d[loop].unknown02    ) );
    printf ( "  programIndex  = %X\n",  fix32 ( metadataSectionHeaders_d[loop].programIndex ) );
    printf ( "  unknown04     = %X\n",  fix32 ( metadataSectionHeaders_d[loop].unknown04    ) );
    printf ( "  sha1Index     = %X\n",  fix32 ( metadataSectionHeaders_d[loop].sha1Index    ) );
    printf ( "  keyIndex      = %X\n",  fix32 ( metadataSectionHeaders_d[loop].keyIndex     ) );
    printf ( "  ivecIndex     = %X\n",  fix32 ( metadataSectionHeaders_d[loop].ivecIndex    ) );
    printf ( "  unknown09     = %X\n",  fix32 ( metadataSectionHeaders_d[loop].unknown09    ) );
  }

  //
  // Read in and decrypt the metadata keys.
  //

  KEY_t metadataKeys[100];
  KEY_t metadataKeys_d[100];

  fread ( metadataKeys, sizeof ( KEY_t ), fix32 ( metadataHeader_d.keyCount ), selfFile );

  AES_ctr128_encrypt ( ( uint8_t * ) metadataKeys, ( uint8_t * ) metadataKeys_d, sizeof ( KEY_t ) * fix32 ( metadataHeader_d.keyCount ), &aesEncryptKey, metadataInfo_d.ivec, ecount, &num );

  printf ( "\n" );
  for ( int loop = 0; loop < fix32 ( metadataHeader_d.keyCount ); loop++ )
  {
    printf ( "metadataKeys[%02X]: ", loop );
    for ( int poop = 0; poop < 16; poop++ ) { printf ( "%02X ", metadataKeys_d[loop][poop] ); }
    printf ( "\n" );
  }

  //
  // Write out the section data.
  //

  printf ( "\nSECTIONS: \n" );
  for ( int loop = 0; loop < fix16 ( elfHeader.e_shnum ); loop++ )
  {
    int fix = atoi ( argv[3] );

    int offset = fix64 ( elfSectionHeaders[loop].sh_offset );
    int size   = fix64 ( elfSectionHeaders[loop].sh_size   );

    if ( ( ( offset + fix ) > 0 ) && ( size > 0 ) )
    {
      uint8_t *sectionBuffer = malloc ( size );

      fseek ( selfFile, ( offset + fix ), SEEK_SET );
      fread ( sectionBuffer, size, 1, selfFile );

      fseek ( elfFile, offset, SEEK_SET );
      fwrite ( sectionBuffer, size, 1, elfFile );

      printf ( "SECTION %02X: %X ==> %X (%X)\n", loop, offset + fix, offset, size );

      free ( sectionBuffer );
    }
  }

  //
  // Read in, decrypt, decompress and write out the program data.
  //

  for ( int loop = 0; loop < fix16 ( elfHeader.e_phnum ); loop++ )
  {
    int elfOffset = fix64 ( elfProgramHeaders[loop].p_offset );
    int elfSize   = fix64 ( elfProgramHeaders[loop].p_filesz );

    if ( elfSize > 0 )
    {
      uint8_t *key  = metadataKeys_d[fix32 ( metadataSectionHeaders_d[loop].keyIndex) ];
      uint8_t *ivec = metadataKeys_d[fix32 ( metadataSectionHeaders_d[loop].ivecIndex )];

      int selfOffset = fix64 ( metadataSectionHeaders_d[loop].dataOffset   );
      int selfSize   = fix64 ( metadataSectionHeaders_d[loop].dataSize     );
      selfSize = ( selfSize + 15 ) & 0xFFFFFFF0;

      uint8_t *selfBuffer = malloc ( selfSize );
      uint8_t *selfBuffer_d = malloc ( selfSize );

      fseek ( selfFile, selfOffset, SEEK_SET );
      fread ( selfBuffer, selfSize, 1, selfFile );

      AES_set_encrypt_key ( key, 128, &aesEncryptKey );

      AES_ctr128_encrypt ( selfBuffer, selfBuffer_d, selfSize, &aesEncryptKey, ivec, ecount, &num );

      uint8_t *elfBuffer = malloc ( elfSize );

      decompress ( selfBuffer_d, selfSize, elfBuffer, elfSize );

      fseek ( elfFile, elfOffset, SEEK_SET );
      fwrite ( elfBuffer, elfSize, 1, elfFile );

      free ( elfBuffer );

      free ( selfBuffer );
      free ( selfBuffer_d );
    }
  }

  //
  // Close the files.
  //

  fclose ( elfFile );

  fclose ( selfFile );

  return 0;
}
