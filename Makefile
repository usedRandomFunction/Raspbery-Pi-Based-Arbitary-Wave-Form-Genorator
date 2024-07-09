# Directories
SRCDIR := src
BINDIR := bin
OBJDIR := $(BINDIR)/obj

# Compiler and linker options
# COMPILERPREFIX = aarch64-linux-gnu-
COMPILERPREFIX = aarch64-elf-
CC := $(COMPILERPREFIX)gcc
CXX := $(COMPILERPREFIX)g++
LD := $(COMPILERPREFIX)ld
C_DEFINITIONS := -DAARCH64

OPATMANISER_SETTING = -O2

DEBUGFLAGS := -g

ASMFLAGS := -Iinc $(DEBUGFLAGS)
CFLAGS := -ffreestanding -nostartfiles  -std=gnu99 -c -Iinc $(OPATMANISER_SETTING) -Werror $(C_DEFINITIONS) -include inc/kconfig.h -g
CXXFLAGS := -ffreestanding -nostartfiles  -std=c++17 -c -Iinc $(OPATMANISER_SETTING) -Werror -fno-exceptions -fno-rtti $(C_DEFINITIONS) -include inc/kconfig.h $(DEBUGFLAGS)
LDFLAGS := -T $(SRCDIR)/linker.ld $(OPATMANISER_SETTING) -nostdlib $(DEBUGFLAGS)

# Source files
C_SOURCES := $(shell find $(SRCDIR) -name '*.c')
CPP_SOURCES := $(shell find $(SRCDIR) -name '*.cpp')
ASM_SOURCES := $(shell find $(SRCDIR) -name '*.s')

# Object files
C_OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
CPP_OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(CPP_SOURCES))
ASM_OBJECTS := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.o,$(ASM_SOURCES))

# Targets
TARGET := $(BINDIR)/kernel.elf

all: $(TARGET) image

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
	$(CC) $(ASMFLAGS) -c -o $@ $<

image:
	@echo !==== Image Genoration ====!
	aarch64-elf-objcopy $(BINDIR)/kernel.elf -O binary $(BINDIR)/kernel8.img

clean:
	rm -rf $(OBJDIR) $(TARGET) $(BINDIR)/kernel.elf
	rm -rf $(OBJDIR) $(TARGET) $(BINDIR)/kernel8.img

run_halt:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x00000,cpu-num=0 -display none -serial stdio -s -S

run:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x00000,cpu-num=0 -display none -serial stdio


run_halt_serial_over_tcp:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x80000,cpu-num=0 -display none -serial tcp::4444,server=on -s -S

run_serial_over_tcp:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x80000,cpu-num=0 -display none -serial tcp::4444,server=on

run_bootloader:
	@echo !==== Running ====!
	sudo python3 ./bootloader.py

.PHONY: all clean