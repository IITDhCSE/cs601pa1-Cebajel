ifeq ($(CTYPE),stack)
	type = STACKALLOCATED
endif
ifeq ($(CTYPE),parallel)
	type = PARALLEL
endif

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

matmul_schedule:
	$(CC) matmul.cpp -D $(type) -D LOOPINTERCHANGE -o main_ijk
	$(CC) matmul.cpp -D $(type) -D LOOPINTERCHANGE=1 -o main_ikj
	$(CC) matmul.cpp -D $(type) -D LOOPINTERCHANGE=2 -o main_kij
	$(CC) matmul.cpp -D $(type) -D LOOPINTERCHANGE=3 -o main_kji
	$(CC) matmul.cpp -D $(type) -D LOOPINTERCHANGE=4 -o main_jik
	$(CC) matmul.cpp -D $(type) -D LOOPINTERCHANGE=5 -o main_jki


clean:
	$(RM) *.o matvec

.PHONY: clean all
