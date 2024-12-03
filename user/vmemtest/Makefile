# Directories
SRCDIR := src
BINDIR := bin
OBJDIR := $(BINDIR)/obj
COMMONDIR = ../common

# Compiler and linker options
COMPILERPREFIX = aarch64-elf-
CC := $(COMPILERPREFIX)gcc
LD := $(COMPILERPREFIX)ld
C_DEFINITIONS := -DAARCH64

OPATMANISER_SETTING = -O2

DEBUGFLAGS := -g

ASMFLAGS := -Iinc $(DEBUGFLAGS)
CFLAGS := -ffreestanding -std=gnu99 -c -Iinc -I$(COMMONDIR)/inc $(OPATMANISER_SETTING) -Wall -Werror $(C_DEFINITIONS) $(DEBUGFLAGS)
LDFLAGS := -T $(SRCDIR)/linker.ld $(OPATMANISER_SETTING) -nostdlib $(DEBUGFLAGS) -L$(COMMONDIR)/bin -lcommon

# Source files
C_SOURCES := $(shell find $(SRCDIR) -name '*.c')
ASM_SOURCES := $(shell find $(SRCDIR) -name '*.s')

# Object files
C_OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.c.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.s.o,$(ASM_SOURCES))

NAME := vmemtest

# Targets
TARGET := $(BINDIR)/$(NAME).elf

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(ASM_OBJECTS)
	@echo !==== linking ====!
	$(LD) -o $@ $^ $(LDFLAGS)
	@echo !==== Image Genoration ====!
	aarch64-elf-objcopy $(TARGET) -O binary $(BINDIR)/$(NAME).img

$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	@echo !==== Compiling $^ ====!
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.s.o: $(SRCDIR)/%.s
	@echo !==== Compiling $^ ====!
	@mkdir -p $(dir $@)
	$(CC) $(ASMFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(TARGET) $(TARGET)

lines_of_code:
		wc -l `find ./src -type f` `find ./inc -type f`

.PHONY: all clean