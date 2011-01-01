//
// pkg.c
// by Dan Peori (dan.peori@oopo.net)
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#include "endian.h"
#include "pkg.h"

//
// Functions
//

void pkgDecodeData ( uint8_t *data, int dataSize, uint64_t *key )
{
  SHA_CTX shaContext;
  uint8_t shaDigest[20];

  while ( dataSize > 0 )
  {
    SHA1_Init ( &shaContext );
    SHA1_Update ( &shaContext, key, 64 );
    SHA1_Final ( shaDigest, &shaContext );

    int count = 16;
    if ( dataSize < 16 ) { count = dataSize; }
    for ( int loop = 0; loop < count; loop++ ) { *data++ ^= shaDigest[loop]; }
    dataSize -= count;

    key[7] = fix64 ( fix64 ( key[7] ) + 1 );
  }
}

void pkgEncodeData ( uint8_t *data, int dataSize, uint64_t *key )
{
  pkgDecodeData ( data, dataSize, key );
}

void pkgPrintEntity ( PKGENTITY_t *pkgEntity )
{
  printf ( "\n[ENTITY]\n" );
  printf ( "  entityNameOffset: %08X\n", fix32 ( pkgEntity->entityNameOffset ) );
  printf ( "  entityNameSize:   %08X\n", fix32 ( pkgEntity->entityNameSize   ) );
  printf ( "  unknown001:       %08X\n", fix32 ( pkgEntity->unknown001       ) );
  printf ( "  entityDataOffset: %08X\n", fix32 ( pkgEntity->entityDataOffset ) );
  printf ( "  unknown002:       %08X\n", fix32 ( pkgEntity->unknown002       ) );
  printf ( "  entityDataSize:   %08X\n", fix32 ( pkgEntity->entityDataSize   ) );
  printf ( "  entityType:       %08X\n", fix32 ( pkgEntity->entityType       ) );
  printf ( "  unknown003:       %08X\n", fix32 ( pkgEntity->unknown003       ) );
}

void pkgPrintHeader ( PKGHEADER_t *pkgHeader )
{
  printf ( "\n[HEADER]\n" );
  printf ( "  magicNumber:    %08X\n",    fix32 ( pkgHeader->magicNumber    ) );
  printf ( "  pkgYype:        %08X\n",    fix32 ( pkgHeader->pkgType        ) );
  printf ( "  pkgHeaderSize:  %08X\n",    fix32 ( pkgHeader->pkgHeaderSize  ) );
  printf ( "  unknown001:     %08X\n",    fix32 ( pkgHeader->unknown001     ) );
  printf ( "  unknown002:     %08X\n",    fix32 ( pkgHeader->unknown002     ) );
  printf ( "  pkgEntityCount: %08X\n",    fix32 ( pkgHeader->pkgEntityCount ) );
  printf ( "  pkgSize:        %016llX\n", fix64 ( pkgHeader->pkgSize        ) );
  printf ( "  pkgDataOffset:  %016llX\n", fix64 ( pkgHeader->pkgDataOffset  ) );
  printf ( "  pkgDataSize:    %016llX\n", fix64 ( pkgHeader->pkgDataSize    ) );
  printf ( "  pkgTitleId:     %s\n", pkgHeader->pkgTitleId );
  printf ( "  pkgHashes:\n" );
  for ( int loop = 0; loop < 12; loop++ ) { printf ( "    %016llX\n", fix64 ( pkgHeader->pkgHashes[loop] ) ); }
}
