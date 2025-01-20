OBJDIR := build/obj
SRCDIR := src

CC := clang
CFLAGS := -O3 -std=c23 -march=native -flto -fwrapv -Wimplicit-fallthrough -Wall -Wno-unused-variable

DEPS := $(shell find $(SRC_DIR) -name '*.h')
DEPS := $(patsubst $(SRCDIR)/%,$(SRCDIR)/%,$(DEPS))

OBJS := $(shell find $(SRCDIR) -name '*.c')
OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(OBJS:.c=.o))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
    @mkdir -p $(dir $@)
    $(CC) -c -o $@ $< $(CFLAGS)

VitaminC: $(OBJS)
    $(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
    @rm -rf build VitaminC
