SSE_ARCH_FLAGS=-msse3

CC=g++
ARCH=-m64 $(SSE_ARCH_FLAGS)
CFLAGS= -O2 $(ARCH) -g -Wall
LDFLAGS=$(ARCH)
LIBS=-lrt

all: matvec 

matvec: matvec.o timeutil.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

matvec.o: matvec.cpp
	$(CC) -c -o $@ $(CFLAGS) $<

timeutil.cpp: timeutil.h

clean:
	$(RM) *.o matvec

.PHONY: clean all
