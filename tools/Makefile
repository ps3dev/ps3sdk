CC = gcc -std=c99 -Wall -pedantic
LIBS = -lssl

TARGETS = pkg-pack pkg-unpack

default: all

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

pkg-pack: endian.h pkg.c pkg.h pkg-pack.c
	$(CC) pkg.c pkg-pack.c $(LIBS) -o pkg-pack

pkg-unpack: endian.h pkg.c pkg.h pkg-unpack.c
	$(CC) pkg.c pkg-unpack.c $(LIBS) -o pkg-unpack
