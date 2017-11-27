//
// pkg.h
// by Naomi Peori (naomi@peori.ca)
//

#ifndef __PKG_H__
#define __PKG_H__

enum
{
  TYPE_NPDRMSELF     = 1,
  TYPE_RAW           = 3,
  TYPE_DIRECTORY     = 4
};

#define TYPE_MASK         0x0FFFFFFF
#define OVERWRITE_ALLOWED 0x80000000

typedef struct
{
  uint32_t magicNumber;
  uint32_t pkgType;
  uint32_t pkgHeaderSize;
  uint32_t unknown001; // 00 00 00 07
  uint32_t unknown002; // 00 00 00 A0
  uint32_t pkgEntityCount;
  uint64_t pkgSize;
  uint64_t pkgDataOffset;
  uint64_t pkgDataSize;
  char pkgTitleId[48];
  uint64_t pkgHashes[12];
}
PKGHEADER_t;

typedef struct
{
  uint32_t entityNameOffset;
  uint32_t entityNameSize;
  uint32_t unknown001;
  uint32_t entityDataOffset;
  uint32_t unknown002;
  uint32_t entityDataSize;
  uint32_t entityType;
  uint32_t unknown003;
}
PKGENTITY_t;

//
// Functions
//

void pkgDecodeData ( uint8_t *data, int dataSize, uint64_t *key );

void pkgEncodeData ( uint8_t *data, int dataSize, uint64_t *key );

void pkgPrintEntity ( PKGENTITY_t *pkgEntity );

void pkgPrintHeader ( PKGHEADER_t *pkgHeader );

#endif
