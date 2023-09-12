input=2048

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

matmul_schedule: matmul.cpp
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE -o ./bin/main_ijk
	./bin/main_ijk $(input)
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=1 -o ./bin/main_ikj
	./bin/main_ikj $(input)
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=2 -o ./bin/main_kij
	./bin/main_kij $(input)
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=3 -o ./bin/main_kji
	./bin/main_kji $(input)
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=4 -o ./bin/main_jik
	./bin/main_jik $(input)
	$(CC) matmul.cpp -D PARALLEL -D LOOPINTERCHANGE=5 -o ./bin/main_jki
	./bin/main_jki $(input)


clean:
	$(RM) *.o matvec

.PHONY: clean all
