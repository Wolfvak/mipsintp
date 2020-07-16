TARGET  := mipsintp

OBJDIR  := build
SRCDIR  := source
OBJS    := main.o MIPS/Instruction.o MIPS/TLB.o MIPS/CPU.o MIPS/CPURun.o
_OBJS   := $(OBJS:%.o=$(OBJDIR)/%.o)
_DEPS   := $(OBJS:%.o=$(OBJDIR)/%.d)

CXX := clang++
CXXFLAGS := -MMD -MP -Ofast -march=native -mtune=native -ggdb \
			-ffast-math -fomit-frame-pointer -std=c++1z -Wall -Wextra -I$(SRCDIR)
LDFLAGS  := -lpthread

ifeq ($(OS),Windows_NT)
	CXXFLAGS	+= $(CXXFLAGS)
	LDFLAGS := -static $(LDFLAGS)
	TARGET	:= $(TARGET).exe
endif

$(TARGET): $(_OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	@mkdir -p "$(@D)"
	$(CXX) $< $(CXXFLAGS) -c -o $@

.PHONY: clean
clean:
	@rm -rf $(OBJDIR) $(TARGET)

test: test/test.bin

test/test.bin:
	mipsel-unknown-elf-gcc -mno-check-zero-division -ffast-math -Os $(shell ls test/*.c test/*.s) -nostartfiles -Ttest/link.ld -o test/test.elf
	mipsel-unknown-elf-objcopy test/test.elf -O binary test/test.bin

.FORCE:
test/test.bin: .FORCE

include $(wildcard $(_DEPS))
