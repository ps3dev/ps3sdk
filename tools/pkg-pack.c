//
// pkg-pack.c
// by Dan Peori (dan.peori@oopo.net)
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "endian.h"
#include "pkg.h"

uint8_t keys[12][8] =
{
  "OOPO IS ", "CAKE!!! ",
  "OOPO IS ", "CAKE!!! ",
  "OOPO IS ", "CAKE!!! ",
  "OOPO IS ", "CAKE!!! ",
  "OOPO IS ", "CAKE!!! ",
  "OOPO IS ", "CAKE!!! ",
};

typedef struct
{
  char name[100];
  int nameSize;
  uint8_t *data;
  int dataSize;
}
ENTITY_t;

ENTITY_t entities[100];
int entityCount = 0;

int totalEntities = 0;
int totalDataSize = 0;
int totalNameSize = 0;

//
// Functions
//

void parseDirectory ( char *directory, char *subdirectory )
{
  char pathname[100];

  if ( subdirectory == NULL )
  {
    strcpy ( pathname, directory );
  }
  else
  {
    sprintf ( pathname, "%s/%s", directory, subdirectory );
  }

  DIR *indir = opendir ( pathname );

  if ( indir )
  {
    struct dirent *entry = NULL;

    while ( ( entry = readdir ( indir ) ) )
    {
      char fullname[100];

      if ( subdirectory == NULL )
      {
	strcpy ( fullname, entry->d_name );
      }
      else
      {
	sprintf ( fullname, "%s/%s", subdirectory, entry->d_name );
      }

      switch ( entry->d_type )
      {
	case 4: // DT_DIR:

	  if ( strncmp ( entry->d_name, ".", 1 ) != 0 )
	  {
	    memset ( &entities[entityCount], 0, sizeof ( ENTITY_t ) );

	    strcpy ( entities[entityCount].name, fullname );
	    entities[entityCount].nameSize = strlen ( fullname );
	    totalNameSize += ( entities[entityCount].nameSize + 15 ) & 0xFFFFFFF0;

	    entities[entityCount].data = NULL;
	    entities[entityCount].dataSize = 0;
	    totalDataSize += ( entities[entityCount].dataSize + 15 ) & 0xFFFFFFF0;

	    entityCount++;

	    parseDirectory ( directory, fullname );
	  }

	  break;

	case 8: // DT_REG:
	{
	  memset ( &entities[entityCount], 0, sizeof ( ENTITY_t ) );

	  strcpy ( entities[entityCount].name, fullname );
	  entities[entityCount].nameSize = strlen ( fullname );
	  totalNameSize += ( entities[entityCount].nameSize + 15 ) & 0xFFFFFFF0;

	  char filename[100];
	  sprintf ( filename, "%s/%s", directory, fullname );
	  FILE *infile = fopen ( filename, "r" );
	  fseek ( infile, 0, SEEK_END );
	  entities[entityCount].dataSize = ftell ( infile );
	  entities[entityCount].data = malloc ( entities[entityCount].dataSize );
	  fseek ( infile, 0, SEEK_SET );
	  fread ( entities[entityCount].data, entities[entityCount].dataSize, 1, infile );
	  fclose ( infile );
	  totalDataSize += ( entities[entityCount].dataSize + 15 ) & 0xFFFFFFF0;

	  entityCount++;
	}
	break;
      }
    }

    closedir ( indir );
  }
}

//
// Main Program
//

int main ( int argc, char **argv )
{
  //
  // Check the arguments.
  //

  if ( argc < 4 )
  {
    printf ( "Usage: %s <pkg file> <directory> <title id>\n", argv[0] );
    return -1;
  }

  //
  // Parse the directory.
  //

  parseDirectory ( argv[2], 0 );
  for ( int loop = 0; loop < entityCount; loop++ ) { printf ( "ENTITY: %s\n", entities[loop].name ); }

  printf ( "TOTAL SIZES: %d %d\n", totalNameSize, totalDataSize );

  //
  // Create the package header.
  //

  PKGHEADER_t pkgHeader;
  memset ( &pkgHeader, 0, sizeof ( PKGHEADER_t ) );

  pkgHeader.magicNumber    = fix32 ( 0x7F504B47 );
  pkgHeader.pkgType        = fix32 ( 0x00000001 );
  pkgHeader.pkgHeaderSize  = fix32 ( sizeof ( PKGHEADER_t ) );
  pkgHeader.unknown001     = fix32 ( 0x00000007 ); // 00 00 00 07
  pkgHeader.unknown002     = fix32 ( 0x000000A0 ); // 00 00 00 A0
  pkgHeader.pkgEntityCount = fix32 ( entityCount );
  pkgHeader.pkgSize        = fix64 ( sizeof ( PKGHEADER_t ) + ( sizeof ( PKGENTITY_t ) * entityCount ) + totalNameSize + totalDataSize );
  pkgHeader.pkgDataOffset  = fix64 ( sizeof ( PKGHEADER_t ) );
  pkgHeader.pkgDataSize    = fix64 ( ( sizeof ( PKGENTITY_t ) * entityCount ) + totalNameSize + totalDataSize );

  strcpy ( pkgHeader.pkgTitleId, argv[3] );

  for ( int loop = 0; loop < 12; loop++ )
  {
    memcpy ( &pkgHeader.pkgHashes[loop], keys[loop], 8 );
  }

  //
  // Create the package data key.
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
  // Create the package data.
  //

  uint8_t *pkgData = malloc ( fix64 ( pkgHeader.pkgDataSize ) );
  memset ( pkgData, 0, fix64 ( pkgHeader.pkgDataSize ) );

  int dataPointer = 0;

  PKGENTITY_t *pkgEntities = ( PKGENTITY_t * ) &pkgData[dataPointer];

  for ( int loop = 0; loop < entityCount; loop++ )
  {
    pkgEntities[loop].entityNameSize = fix32 ( entities[loop].nameSize );
    pkgEntities[loop].entityDataSize = fix32 ( entities[loop].dataSize );

    if ( pkgEntities[loop].entityDataSize > 0 )
    {
      pkgEntities[loop].entityType = fix32 ( 0x80000001 );
    }
    else
    {
      pkgEntities[loop].entityType = fix32 ( 0x80000004 );
    }

    dataPointer += sizeof ( PKGENTITY_t );
  }

  for ( int loop = 0; loop < entityCount; loop++ )
  {
    memcpy ( &pkgData[dataPointer], entities[loop].name, entities[loop].nameSize );
    pkgEntities[loop].entityNameOffset = fix32 ( dataPointer );

    dataPointer += ( entities[loop].nameSize + 15 ) & 0xFFFFFFF0;
  }

  for ( int loop = 0; loop < entityCount; loop++ )
  {
    memcpy ( &pkgData[dataPointer], entities[loop].data, entities[loop].dataSize );
    pkgEntities[loop].entityDataOffset = fix32 ( dataPointer );

    dataPointer += ( entities[loop].dataSize + 15 ) & 0xFFFFFFF0;
  }

  pkgEncodeData ( pkgData, fix64 ( pkgHeader.pkgDataSize ), pkgDataKey );

  //
  // Write out the package file.
  //

  FILE *pkgFile = fopen ( argv[1], "w" );

  if ( pkgFile == NULL )
  {
    printf ( "Error: Could not open the package file. (%s)\n", argv[1] );
    return -1;
  }

  fwrite ( &pkgHeader, sizeof ( PKGHEADER_t ), 1, pkgFile );

  fwrite ( pkgData, fix64 ( pkgHeader.pkgDataSize ), 1, pkgFile );

  fclose ( pkgFile );

  return 0;
}
