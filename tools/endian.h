//
// endian.h
// by Dan Peori (dan.peori@oopo.net)
//

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <byteswap.h>

// #define __BIG_ENDIAN__

#ifdef __BIG_ENDIAN__
#define fix16(value) value
#define fix32(value) value
#define fix64(value) (long long unsigned)value
#else
#define fix16(value) bswap_16(value)
#define fix32(value) bswap_32(value)
#define fix64(value) (long long unsigned)bswap_64(value)
#endif

#endif
