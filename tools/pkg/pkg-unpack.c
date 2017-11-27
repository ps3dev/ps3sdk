//
// pkg-unpack.c
// by Naomi Peori (naomi@peori.ca)
//

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "endian.h"
#include "pkg.h"

#define MAX_FILENAME_LEN 100

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

  PKGENTITY_t *pkgEntities[fix32 ( pkgHeader.pkgEntityCount )];

  char *pkgEntityNames[fix32 ( pkgHeader.pkgEntityCount )];

  uint8_t *pkgEntityDatas[fix32 ( pkgHeader.pkgEntityCount )];

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

  if ( 0 != mkdir ( argv[2], 0777 ) && errno != EEXIST )
  {
    printf("ERROR: could not create directory '%s'.\n", argv[2]);
    return -1;
  }

  //
  // Write out the package data entities.
  //

  for ( int loop = 0; loop < fix32 ( pkgHeader.pkgEntityCount ); loop++ )
  {
    char filename[MAX_FILENAME_LEN] = { 0 };

    if (MAX_FILENAME_LEN - 1 < 1 + strlen ( argv[2] ) + fix32 ( pkgEntities[loop]->entityNameSize ) )
    {
      printf("Path name is too long for buffer, aborting\n");
      return -1;
    }

    sprintf ( filename, "%s/", argv[2] );
    strncat ( filename, pkgEntityNames[loop], fix32 ( pkgEntities[loop]->entityNameSize )  );

    switch ( TYPE_MASK & fix32 ( pkgEntities[loop]->entityType ) )
    {
      case TYPE_DIRECTORY:
      {
	printf ( "MKDIR %s\n", filename );

	if ( 0 != mkdir ( filename, 0777 ) && errno != EEXIST )
        {
          printf("ERROR: could not create directory '%s'\n", filename);
          return -1;
        }
      }
      break;

      default:
      {
	printf ( "WRITE %s\n", filename );

	FILE *outfile = fopen ( filename, "w" );
	if ( outfile == NULL )
        {
          printf ( "ERROR: could not open '%s' for writing\n", filename );
          return -1;
        }
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
