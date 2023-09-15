SSE_ARCH_FLAGS=-msse3
blaspath=-I /home/resiliente/cs410software/openblas_0_3_21_install/include/ -L /home/resiliente/cs410software/openblas_0_3_21_install/lib/ -lopenblas
SRC=./src
BIN=./bin
INC=./inc
OBJ=./obj
CC=g++
MY_ARGS=-march=native
ARCH=-m64 $(SSE_ARCH_FLAGS)
CFLAGS= -O3 $(ARCH) -g -Wall
LDFLAGS=$(ARCH)
LIBS=-lrt
input=2048
N=11

all: matmul_schedule matmul_optlevel matmul_blas matvec matvec_matmul

# 1(a) Balaji
matmul_schedule: $(BIN) $(BIN)/matmul_ijk $(BIN)/matmul_ikj $(BIN)/matmul_kij $(BIN)/matmul_kji $(BIN)/matmul_jik $(BIN)/matmul_jki
	$(BIN)/matmul_ijk $(input)
	echo ""
	$(BIN)/matmul_ikj $(input)
	echo ""
	$(BIN)/matmul_kij $(input)
	echo ""
	$(BIN)/matmul_kji $(input)
	echo ""
	$(BIN)/matmul_jik $(input)
	echo ""
	$(BIN)/matmul_jki $(input)
	echo ""

$(BIN):
	mkdir bin

$(BIN)/matmul_ijk: $(SRC)/matmul.cpp
	$(CC) -o $@ $^ -D PARALLEL -D LOOPINTERCHANGE=0
$(BIN)/matmul_ikj: $(SRC)/matmul.cpp
	$(CC) -o $@ $^ -D PARALLEL -D LOOPINTERCHANGE=1
$(BIN)/matmul_kij: $(SRC)/matmul.cpp
	$(CC) -o $@ $^ -D PARALLEL -D LOOPINTERCHANGE=2
$(BIN)/matmul_kji: $(SRC)/matmul.cpp
	$(CC) -o $@ $^ -D PARALLEL -D LOOPINTERCHANGE=3
$(BIN)/matmul_jik: $(SRC)/matmul.cpp
	$(CC) -o $@ $^ -D PARALLEL -D LOOPINTERCHANGE=4
$(BIN)/matmul_jki: $(SRC)/matmul.cpp
	$(CC) -o $@ $^ -D PARALLEL -D LOOPINTERCHANGE=5

# 1(b) Cebajel
matmul_optlevel: $(BIN) $(BIN)/matmul_o0 $(BIN)/matmul_o1 $(BIN)/matmul_o2 $(BIN)/matmul_o3 $(BIN)/matmul_o4
	$(BIN)/matmul_o0 $(input)
	echo ""
	$(BIN)/matmul_o1 $(input)
	echo ""
	$(BIN)/matmul_o2 $(input)
	echo ""
	$(BIN)/matmul_o3 $(input)
	echo ""
	$(BIN)/matmul_o4 $(input)
	echo ""

$(BIN)/matmul_o0: $(SRC)/matmul.cpp
	$(CC) $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0

$(BIN)/matmul_o1: $(SRC)/matmul.cpp
	$(CC) $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0 -O1

$(BIN)/matmul_o2: $(SRC)/matmul.cpp
	$(CC) $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0 -O2

$(BIN)/matmul_o3: $(SRC)/matmul.cpp
	$(CC) $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0 -O3

$(BIN)/matmul_o4: $(SRC)/matmul.cpp
	$(CC) $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0 -O4

# 1(c) Aditya
matmul_blas: $(BIN) $(BIN)/matmul_a $(BIN)/matmul_b
	$(BIN)/matmul_a $(input)
	echo ""
	$(BIN)/matmul_b $(input)
	echo ""

$(BIN)/matmul_a: $(SRC)/matmul.cpp
	$(CC) $^ -o $@ $(blaspath) -DBLAS=1 -D PARALLEL $(MY_ARGS)

$(BIN)/matmul_b: $(SRC)/matmul.cpp
	$(CC) $^ -o $@ $(blaspath) -DBLAS=2 -D PARALLEL $(MY_ARGS)

# 2 Cebajel
matvec: $(OBJ) $(OBJ)/matvec
	$^
	echo ""

$(OBJ):
	mkdir obj

$(OBJ)/matvec: $(BIN)/matvec.o $(BIN)/timeutil.o
	$(CC) -o $@ $^ $(MY_ARGS) $(LDFLAGS) $(LIBS) -fopenmp -lgomp

$(BIN)/matvec.o: $(SRC)/matvec.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(MY_ARGS) -fopenmp -lgomp

$(SRC)/timeutil.cpp: $(INC)/timeutil.h

$(BIN)/timeutil.o: $(SRC)/timeutil.cpp $(INC)/timeutil.h
	$(CC) -c -o $@ $< -I$(INC) $(MY_ARGS)

# 3 Cebajel
matvec_matmul: $(OBJ) $(OBJ)/matvec_matmul
	$^
	echo ""

$(OBJ)/matvec_matmul: $(BIN)/matvec_matmul.o $(BIN)/timeutil.o
	$(CC) -o $@ $^ $(LIBS) $(MY_ARGS) -O3 $(LDFLAGS) -fopenmp -lgomp

$(BIN)/matvec_matmul.o: $(SRC)/matvec.cpp
	$(CC) -c -o $@ $^ $(CFLAGS) $(MY_ARGS) -D MATMUL -D SIZE2=$(N) -fopenmp -lgomp

clean:
	$(RM) $(BIN)/*
	$(RM) $(OBJ)/*

.PHONY: clean all matmul_schedule matmul_optlevel matmul_blas matvec matvec_matmul
