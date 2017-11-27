//
// endian.h
// by Naomi Peori (naomi@peori.ca)
//

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

static inline unsigned short bswap_16(unsigned short x) {
  return (x>>8) | (x<<8);
}

static inline unsigned int bswap_32(unsigned int x) {
  return (bswap_16(x&0xffff)<<16) | (bswap_16(x>>16));
}

static inline unsigned long long bswap_64(unsigned long long x) {
  return (((unsigned long long)bswap_32(x&0xffffffffull))<<32) | (bswap_32(x>>32));
}

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
