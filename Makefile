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

all: $(OBJ)/matvec matmul_schedule matmul_optlevel matmul_blas

$(OBJ)/matvec: $(BIN)/matvec.o $(BIN)/timeutil.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BIN)/matvec.o: $(SRC)/matvec.cpp
	$(CC) -c -o $@ $(CFLAGS) $<

$(SRC)/timeutil.cpp: $(INC)/timeutil.h

$(BIN)/timeutil.o: $(SRC)/timeutil.cpp $(INC)/timeutil.h
	$(CC) -c -o $@ -I$(INC) $<

# 1(a) Balaji
matmul_schedule: $(BIN) $(BIN)/matmul_ijk $(BIN)/matmul_ikj $(BIN)/matmul_kij $(BIN)/matmul_kji $(BIN)/matmul_jik $(BIN)/matmul_jki

$(BIN):
	mkdir bin

$(BIN)/matmul_ijk: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=0 -o $@
	$@ $(input)
$(BIN)/matmul_ikj: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=1 -o $@
	$@ $(input)
$(BIN)/matmul_kij: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=2 -o $@
	$@ $(input)
$(BIN)/matmul_kji: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=3 -o $@
	$@ $(input)
$(BIN)/matmul_jik: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=4 -o $@
	$@ $(input)
$(BIN)/matmul_jki: $(SRC)/matmul.cpp
	$(CC) $^ -D PARALLEL -D LOOPINTERCHANGE=5 -o $@
	$@ $(input)

# 1(b) Cebajel
matmul_optlevel: $(BIN) $(BIN)/matmul_o0 $(BIN)/matmul_o1 $(BIN)/matmul_o2 $(BIN)/matmul_o3 $(BIN)/matmul_o4

$(BIN)/matmul_o0: $(SRC)/matmul.cpp
	$(CC) $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0
	$@ $(input)

$(BIN)/matmul_o1: $(SRC)/matmul.cpp
	$(CC) -O1 $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0
	$@ $(input)

$(BIN)/matmul_o2: $(SRC)/matmul.cpp
	$(CC) -O2 $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0
	$@ $(input)

$(BIN)/matmul_o3: $(SRC)/matmul.cpp
	$(CC) -O3 $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0
	$@ $(input)

$(BIN)/matmul_o4: $(SRC)/matmul.cpp
	$(CC) -O4 $^ -o $@ -D PARALLEL -D LOOPINTERCHANGE=0
	$@ $(input)

# 1(c) Aditya
matmul_blas: $(BIN) $(BIN)/matmul_a $(BIN)/matmul_b

$(BIN)/matmul_a: $(SRC)/matmul.cpp
	$(CC) $^ $(blaspath) -DBLAS=1 -D PARALLEL  -o $@
	$@ $(input)

$(BIN)/matmul_b: $(SRC)/matmul.cpp
	$(CC) $^ $(blaspath) -DBLAS=2 -D PARALLEL -o $@
	$@ $(input)

clean:
	$(RM) $(BIN)/*.o matvec
	$(RM) $(OBJ)/*

.PHONY: clean all matmul_schedule matmul_optlevel matmul_blas