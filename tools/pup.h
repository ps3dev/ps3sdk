//
// pup.h
// by Dan Peori (dan.peori@oopo.net)
//

#ifndef __PUP_H__
#define __PUP_H__

//
// Structures
//

typedef struct
{
  uint64_t magicNumber;
  uint64_t pupVersion;
  uint64_t imageVersion;
  uint64_t fileCount;
  uint64_t headerSize;
  uint64_t fileSize;
}
PUPHEADER_t;

typedef struct
{
  uint64_t fileId;
  uint64_t fileOffset;
  uint64_t fileSize;
  uint8_t padding[8];
}
PUPFILEHEADER_t;

typedef struct
{
  uint64_t fileIndex;
  uint8_t fileHmac[20];
  uint8_t padding[4];
}
PUPFILEHASH_t;

typedef struct
{
  uint8_t headerHmac[20];
  uint8_t padding[12];
}
PUPHEADERHASH_t;

//
// Functions
//

char *pupGetFilename ( PUPFILEHEADER_t *fileHeader );

void pupPrintHeader ( PUPHEADER_t *header );

void pupPrintFileHeader ( PUPFILEHEADER_t *fileHeader );

#endif
