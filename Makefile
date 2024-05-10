# Directories
SRCDIR := src
BINDIR := bin
OBJDIR := $(BINDIR)/obj

# Compiler and linker options
CC := aarch64-elf-gcc
CXX := aarch64-elf-g++
LD := aarch64-elf-ld
C_DEFINITIONS := -DAARCH64

OPATMANISER_SETTING = -O2

CFLAGS := -ffreestanding -std=gnu99 -c -Iinc $(OPATMANISER_SETTING) -Werror $(C_DEFINITIONS) -include inc/kconfig.h
CXXFLAGS := -ffreestanding -std=c++17 -c -Iinc $(OPATMANISER_SETTING) -Werror -fno-exceptions -fno-rtti $(C_DEFINITIONS) -include inc/kconfig.h
LDFLAGS := -T $(SRCDIR)/linker.ld $(OPATMANISER_SETTING) -nostdlib

# Source files
C_SOURCES := $(shell find $(SRCDIR) -name '*.c')
CPP_SOURCES := $(shell find $(SRCDIR) -name '*.cpp')
ASM_SOURCES := $(shell find $(SRCDIR) -name '*.s')

# Object files
C_OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
CPP_OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(CPP_SOURCES))
ASM_OBJECTS := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.o,$(ASM_SOURCES))

# Targets
TARGET := $(BINDIR)/kernal.elf

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(CPP_OBJECTS) $(ASM_OBJECTS)
	@echo !==== linking ====!
	$(LD) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo !==== Compiling $^ ====!
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo !==== Compiling $^ ====!
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	@echo !==== Compiling $^ ====!
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

image:
	@echo !==== Image Genoration ====!
	aarch64-elf-objcopy $(BINDIR)/kernal.elf -O binary $(BINDIR)/kernel.img

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean

runPiZero: 
	qemu-system-arm -M raspi0 -serial stdio -kernel bin/kernal.elf

runPi3b:
	qemu-system-aarch64 -M raspi3b -serial stdio -kernel bin/kernal.elf