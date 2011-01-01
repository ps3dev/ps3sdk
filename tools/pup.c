//
// pup.c
// by Dan Peori (dan.peori@oopo.net)
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "endian.h"

#include "pup.h"

//
// Structures
//

typedef struct
{
  uint64_t fileId;
  char fileName[256];
}
PUPFILEINFO_t;

PUPFILEINFO_t pupFileInfo[8] =
{
  { 0x0000000000000100, "version.txt"      },
  { 0x0000000000000101, "license.txt"      },
  { 0x0000000000000102, "TOP_SIKRIT.txt"   },
  { 0x0000000000000103, "zeroes.txt"       },
  { 0x0000000000000200, "updater.self"     },
  { 0x0000000000000201, "vsh.tar"          },
  { 0x0000000000000202, "dots.txt"         },
  { 0x0000000000000300, "update_files.tar" },
};

//
// Functions
//

char *pupGetFilename ( PUPFILEHEADER_t *fileHeader )
{
  for ( int loop = 0; loop < 8; loop++ )
  {
    int fileId = fix64 ( fileHeader->fileId );

    if ( fileId == pupFileInfo[loop].fileId )
    {
      return pupFileInfo[loop].fileName;
    }
  }

  return NULL;
}

void pupPrintHeader ( PUPHEADER_t *header )
{
  printf ( "\n[HEADER]\n" );
  printf ( "  magicNumber  = 0x%016llX\n",  fix64 ( header->magicNumber  ) );
  printf ( "  pupVersion   = 0x%016llX\n",  fix64 ( header->pupVersion   ) );
  printf ( "  imageVersion = 0x%016llX\n",  fix64 ( header->imageVersion ) );
  printf ( "  fileCount    = %llu\n",       fix64 ( header->fileCount    ) );
  printf ( "  headerSize   = %llu bytes\n", fix64 ( header->headerSize   ) );
  printf ( "  fileSize     = %llu bytes\n", fix64 ( header->fileSize     ) );
  printf ( "\n" );
}

void pupPrintFileHeader ( PUPFILEHEADER_t *fileHeader )
{
  printf ( "\n[%s]\n", pupGetFilename ( fileHeader ) );
  printf ( "  fileId     = 0x%016llX\n",  fix64 ( fileHeader->fileId     ) );
  printf ( "  fileOffset = %llu bytes\n", fix64 ( fileHeader->fileOffset ) );
  printf ( "  fileSize   = %llu bytes\n", fix64 ( fileHeader->fileSize   ) );
  printf ("\n" );
}
