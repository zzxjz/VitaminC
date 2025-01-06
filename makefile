ODIR=build/obj
SDIR=src

CC=clang
CFLAGS=-I./src -O3 -std=c23

_DEPS = main.h cpu/arm11/interpreter.h cpu/arm11/alu/alu.h bus.h patternmatch.h types.h
DEPS = $(patsubst %,$(SDIR)/%,$(_DEPS))

_OBJ = main.o cpu/arm11/interpreter.o bus.o cpu/arm11/alu/alu.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

VitaminC: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS)
