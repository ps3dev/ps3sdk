//
// pup-unpack.c
// by Naomi Peori (naomi@peori.ca)
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "endian.h"

#include "pup.h"

int main ( int argc, char **argv )
{
  //
  // Check the arguments.
  //

  if ( argc != 3 )
  {
    printf ( "Usage: %s [filename] [directory]\n", argv[0] );
    return -1;
  }

  //
  // Open the PUP file.
  //

  FILE *infile = fopen ( argv[1], "r" );

  if ( infile == NULL )
  {
    printf ( "Error: Unable to open the PUP file. (%s)\n", argv[1] );
    return -1;
  }

  //
  // Read in the PUP header.
  //

  PUPHEADER_t header;

  fread ( &header, sizeof ( PUPHEADER_t ), 1, infile );

  pupPrintHeader ( &header );

  //
  // Read in the PUP file headers.
  //

  int fileCount = fix64 ( header.fileCount );

  PUPFILEHEADER_t fileHeaders[fileCount];

  fread ( fileHeaders, sizeof ( PUPFILEHEADER_t ), fileCount, infile );

  for ( int loop = 0; loop < fileCount; loop++ )
  {
    pupPrintFileHeader ( &fileHeaders[loop] );
  }

  //
  // Read in and write out the PUP file data.
  //

  mkdir ( argv[2], 0777 );

  for ( int loop1 = 0; loop1 < fileCount; loop1++ )
  {
    int fileSize = fix64 ( fileHeaders[loop1].fileSize );

    void *buffer = malloc ( fileSize );

    if ( buffer )
    {
      int fileOffset = fix64 ( fileHeaders[loop1].fileOffset );

      fseek ( infile, fileOffset, SEEK_SET );

      if ( fread ( buffer, fileSize, 1, infile ) )
      {
	char filename[256] = { 0 };

	snprintf ( filename, sizeof filename, "%s/%s", argv[2], pupGetFilename ( &fileHeaders[loop1] ) );

	FILE *outfile = fopen ( filename, "w" );

	if ( outfile )
	{
	  fwrite ( buffer, fileSize, 1, outfile );

	  fclose ( outfile );
	}
      }

      free ( buffer );
    }
  }

  //
  // Close the PUP file.
  //

  fclose ( infile );

  return 0;
}
