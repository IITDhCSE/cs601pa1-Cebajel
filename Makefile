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
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE -o ./bin/main_ijk
	./bin/main_ijk 32
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=1 -o ./bin/main_ikj
	./bin/main_ikj 32
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=2 -o ./bin/main_kij
	./bin/main_kij 32
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=3 -o ./bin/main_kji
	./bin/main_kji 32
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=4 -o ./bin/main_jik
	./bin/main_jik 32
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=5 -o ./bin/main_jki
	./bin/main_jki 32


clean:
	$(RM) *.o matvec

.PHONY: clean all
