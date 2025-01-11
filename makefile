ODIR=build/obj
SDIR=src

CC=clang
CFLAGS=-I./src -O3 -std=c23 -march=native

_DEPS = main.h cpu/arm11/interpreter.h bus.h patternmatch.h types.h
DEPS = $(patsubst %,$(SDIR)/%,$(_DEPS))

_OBJ = main.o cpu/arm11/interpreter.o bus.o cpu/arm11/alu/alu.o cpu/arm11/loadstore.o cpu/arm11/branch.o cpu/arm11/coproc/coproc.o cpu/arm11/coproc/cp15.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

VitaminC: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS)
