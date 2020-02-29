TARGET  := mipsintp

CFLAGS := -MMD -MP -march=native -mtune=native -ffast-math -fomit-frame-pointer \
			-ggdb -std=c99 -O2 -Wall -Wextra -Wpadded
LDFLAGS  := -lpthread

ifeq ($(OS),Windows_NT)
	CFLAGS	+= $(CFLAGS)
	LDFLAGS	:= -static $(LDFLAGS)
	TARGET	:= $(TARGET).exe
endif

CC := clang

OBJDIR  := build
SRCDIR  := source
OBJS    := disasm.o mipsint.o instrun.o
_OBJS   := $(OBJS:%.o=$(OBJDIR)/%.o)
_DEPS   := $(OBJS:%.o=$(OBJDIR)/%.d)

$(TARGET): $(_OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p "$(@D)"
	$(CC) $< $(CFLAGS) -c -o $@

.PHONY: clean
clean:
	@rm -rf $(OBJDIR) $(TARGET)

test: test/test.bin

test/test.bin: test/test.c
	@#mipsel-unknown-elf-as test/test.s -o test/test.elf
	mipsel-unknown-elf-gcc -O2 test/test.c -nostartfiles -Ttest/link.ld -o test/test.elf
	mipsel-unknown-elf-objcopy test/test.elf -O binary test/test.bin

.FORCE:
test/test.bin: .FORCE

include $(wildcard $(_DEPS))
