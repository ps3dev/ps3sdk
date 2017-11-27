//
// self.h
// by Naomi Peori (naomi@peori.ca)
//

#ifndef __SELF_H__
#define __SELF_H__

typedef uint8_t KEY_t[16];

typedef struct
{
  uint32_t magic;
  uint32_t unknown01;
  uint32_t unknown02;
  uint32_t metadataInfoOffset;
  uint64_t fileOffset;
  uint64_t fileSize;
  uint64_t unknown06;
  uint64_t programInfoOffset;
  uint64_t elfHeaderOffset;
  uint64_t elfProgramHeadersOffset;
  uint64_t elfSectionHeadersOffset;
  uint64_t unknown11;
  uint64_t unknown12;
  uint64_t controlInfoOffset;
  uint64_t controlInfoSize;
  uint64_t unknown15;
}
SELFHEADER_t;

typedef struct
{
  uint64_t programAuthId;
  uint64_t unknown01;
  uint16_t programVersion[4];
  uint64_t unknown03;
}
PROGRAMINFO_t;

typedef struct
{
  uint32_t unknown00;
  uint32_t unknown01;
  uint32_t unknown02;
  uint32_t unknown03;
  uint32_t controlFlags[8];
  uint32_t unknown05;
  uint32_t unknown06;
  uint32_t unknown07;
  uint32_t unknown08;
  char digest[64];
  uint32_t unknown10;
  uint32_t unknown11;
}
CONTROLINFO_t;

typedef struct
{
  uint8_t unknown00[32];
  uint8_t key[32];
  uint8_t ivec[32];
}
METADATAINFO_t;

typedef struct
{
  uint32_t unknown00;
  uint32_t size;
  uint32_t unknown02;
  uint32_t sectionCount;
  uint32_t keyCount;
  uint32_t unknown05;
  uint32_t unknown06;
  uint32_t unknown07;
}
METADATAHEADER_t;

typedef struct
{
  uint64_t dataOffset;
  uint64_t dataSize;
  uint32_t unknown02;
  uint32_t programIndex;
  uint32_t unknown04;
  uint32_t sha1Index;
  uint32_t unknown05;
  uint32_t keyIndex;
  uint32_t ivecIndex;
  uint32_t unknown09;
}
METADATASECTIONHEADER_t;

#endif
