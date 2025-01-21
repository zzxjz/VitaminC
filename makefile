OBJDIR := build/obj
SRCDIR := src

CC := clang
CFLAGS := -MP -MMD -O3 -std=c23 -march=native -flto -fwrapv -Wimplicit-fallthrough -Wall -Wno-unused-variable

OBJS := $(shell find $(SRCDIR) -name '*.c')
OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(OBJS:.c=.o))

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(CFLAGS)

VitaminC: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	@rm -rf build VitaminC
