//
// pkg-unpack.c
// by Dan Peori (dan.peori@oopo.net)
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "endian.h"
#include "pkg.h"

int main ( int argc, char **argv )
{
  //
  // Check the arguments.
  //

  if ( argc < 3 )
  {
    printf ( "Usage: %s <pkg file> <directory>\n", argv[0] );
    return -1;
  }

  //
  // Open the package file.
  //

  FILE *pkgFile = fopen ( argv[1], "r" );

  if ( pkgFile == NULL )
  {
    printf ( "Error: Could not open the package file. (%s)\n", argv[1] );
    return -1;
  }

  //
  // Read in the package header.
  //

  PKGHEADER_t pkgHeader;

  fseek ( pkgFile, 0, SEEK_SET );
  fread ( &pkgHeader, sizeof ( PKGHEADER_t ), 1, pkgFile );

  pkgPrintHeader ( &pkgHeader );

  //
  // Generate the package data key.
  //

  uint64_t pkgDataKey[8];

  pkgDataKey[0] = pkgHeader.pkgHashes[0];
  pkgDataKey[1] = pkgHeader.pkgHashes[0];
  pkgDataKey[2] = pkgHeader.pkgHashes[1];
  pkgDataKey[3] = pkgHeader.pkgHashes[1];
  pkgDataKey[4] = 0;
  pkgDataKey[5] = 0;
  pkgDataKey[6] = 0;
  pkgDataKey[7] = 0;

  //
  // Read in the package data.
  //

  uint8_t *pkgData = malloc ( fix64 ( pkgHeader.pkgDataSize ) );

  fseek ( pkgFile, fix64 ( pkgHeader.pkgDataOffset ), SEEK_SET );
  fread ( pkgData, fix64 ( pkgHeader.pkgDataSize ), 1, pkgFile );

  pkgDecodeData ( pkgData, fix64 ( pkgHeader.pkgDataSize ), pkgDataKey );

  //
  // Parse the package data entities.
  //

  PKGENTITY_t *pkgEntities[100];

  char *pkgEntityNames[100];

  uint8_t *pkgEntityDatas[100];

  for ( int loop = 0; loop < fix32 ( pkgHeader.pkgEntityCount ); loop++ )
  {
    pkgEntities[loop] = ( PKGENTITY_t * ) &pkgData[sizeof ( PKGENTITY_t ) * loop];
    pkgPrintEntity ( pkgEntities[loop] );

    pkgEntityNames[loop] = ( char * ) &pkgData[fix32 ( pkgEntities[loop]->entityNameOffset )];

    pkgEntityDatas[loop] = ( uint8_t * ) &pkgData[fix32 ( pkgEntities[loop]->entityDataOffset )];
  }

  //
  // Create the output directory.
  //

  mkdir ( argv[2], 0777 );

  //
  // Write out the package data entities.
  //

  for ( int loop = 0; loop < fix32 ( pkgHeader.pkgEntityCount ); loop++ )
  {
    char filename[100] = { 0 };

    sprintf ( filename, "%s/", argv[2] );
    strncat ( filename, pkgEntityNames[loop], fix32 ( pkgEntities[loop]->entityNameSize )  );

    switch ( fix32 ( pkgEntities[loop]->entityType ) )
    {
      case 0x80000004:
      {
	printf ( "MKDIR %s\n", filename );

	mkdir ( filename, 0777 );
      }
      break;

      default:
      {
	printf ( "WRITE %s\n", filename );

	FILE *outfile = fopen ( filename, "w" );
	fwrite ( pkgEntityDatas[loop], fix32 ( pkgEntities[loop]->entityDataSize ), 1, outfile );
	fclose ( outfile );
      }
      break;
    }
  }

  //
  // Close the package file.
  //

  fclose ( pkgFile );

  return 0;
}
