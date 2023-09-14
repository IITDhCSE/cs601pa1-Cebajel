SSE_ARCH_FLAGS=-msse3
blaspath=-I /home/resiliente/cs410software/openblas_0_3_21_install/include/ -L /home/resiliente/cs410software/openblas_0_3_21_install/lib/ -lopenblas
SRC=./src
BIN=./bin
INC=./inc
OBJ=./obj
CC=g++
ARCH=-m64 $(SSE_ARCH_FLAGS)
CFLAGS= -O2 $(ARCH) -g -Wall
LDFLAGS=$(ARCH)
LIBS=-lrt
input=2048
N=11

all: matmul_schedule matmul_optlevel matmul_blas matvec matvec_matmul

# 1(a) Balaji
matmul_schedule: $(BIN) $(BIN)/matmul_ijk $(BIN)/matmul_ikj $(BIN)/matmul_kij $(BIN)/matmul_kji $(BIN)/matmul_jik $(BIN)/matmul_jki
	$(BIN)/matmul_ijk $(input)
	$(BIN)/matmul_ikj $(input)
	$(BIN)/matmul_kij $(input)
	$(BIN)/matmul_kji $(input)
	$(BIN)/matmul_jik $(input)
	$(BIN)/matmul_jki $(input)

$(BIN):
	mkdir bin

$(BIN)/matmul_ijk: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=0 -o $@
$(BIN)/matmul_ikj: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=1 -o $@
$(BIN)/matmul_kij: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=2 -o $@
$(BIN)/matmul_kji: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=3 -o $@
$(BIN)/matmul_jik: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=4 -o $@
$(BIN)/matmul_jki: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=5 -o $@

# 1(b) Cebajel
matmul_optlevel: $(BIN) $(BIN)/matmul_o0 $(BIN)/matmul_o1 $(BIN)/matmul_o2 $(BIN)/matmul_o3 $(BIN)/matmul_o4
	$(BIN)/matmul_o0 $(input)
	$(BIN)/matmul_o1 $(input)
	$(BIN)/matmul_o2 $(input)
	$(BIN)/matmul_o3 $(input)
	$(BIN)/matmul_o4 $(input)

$(BIN)/matmul_o0: $(SRC)/matmul.cpp
	$(CC) $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0

$(BIN)/matmul_o1: $(SRC)/matmul.cpp
	$(CC) -O1 $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0

$(BIN)/matmul_o2: $(SRC)/matmul.cpp
	$(CC) -O2 $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0

$(BIN)/matmul_o3: $(SRC)/matmul.cpp
	$(CC) -O3 $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0

$(BIN)/matmul_o4: $(SRC)/matmul.cpp
	$(CC) -O4 $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0

# 1(c) Aditya
matmul_blas: $(BIN) $(BIN)/matmul_a $(BIN)/matmul_b
	$(BIN)/matmul_a $(input)
	$(BIN)/matmul_b $(input)

$(BIN)/matmul_a: $(SRC)/matmul.cpp
	$(CC) $^ $(blaspath) -DBLAS=1 -D PARALLEL  -o $@

$(BIN)/matmul_b: $(SRC)/matmul.cpp
	$(CC) $^ $(blaspath) -DBLAS=2 -D PARALLEL -o $@

# 2 Cebajel
matvec: $(OBJ)/matvec
	$^

$(OBJ)/matvec: $(BIN)/matvec.o $(BIN)/timeutil.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BIN)/matvec.o: $(SRC)/matvec.cpp
	$(CC) -c -o $@ $(CFLAGS) $<

$(SRC)/timeutil.cpp: $(INC)/timeutil.h

$(BIN)/timeutil.o: $(SRC)/timeutil.cpp $(INC)/timeutil.h
	$(CC) -c -o $@ -I$(INC) $<

# 3 Cebajel
matvec_matmul: $(OBJ)/matvec_matmul
	$^

$(OBJ)/matvec_matmul: $(BIN)/matvec_matmul.o $(BIN)/timeutil.o
	$(CC) -O3 $(LDFLAGS) -o $@ $^ $(LIBS)

$(BIN)/matvec_matmul.o: $(SRC)/matvec.cpp
	$(CC) -c -o $@ $(CFLAGS) -D MATMUL -D SIZE2=$(N) -DPARALLEL $^

clean:
	$(RM) $(BIN)/*
	$(RM) $(OBJ)/*

.PHONY: clean all matmul_schedule matmul_optlevel matmul_blas matvec matvec_matmul
