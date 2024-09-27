# Directories
SRCDIR := src
BINDIR := bin
DATADIR := data
OBJDIR := $(BINDIR)/obj

# Compiler and linker options
COMPILERPREFIX = aarch64-elf-
CC := $(COMPILERPREFIX)gcc
AR := $(COMPILERPREFIX)ar
C_DEFINITIONS := -DAARCH64

OPATMANISER_SETTING = -O2

DEBUGFLAGS := -g

ASMFLAGS := -Iinc $(DEBUGFLAGS)
CFLAGS := -ffreestanding -nostartfiles  -std=gnu99 -c -Iinc $(OPATMANISER_SETTING) -Wall -Werror $(C_DEFINITIONS)
ARFLAGS := rcs

# Source files
C_SOURCES := $(shell find $(SRCDIR) -name '*.c')
ASM_SOURCES := $(shell find $(SRCDIR) -name '*.s')
DATA_SOURCES := $(shell find $(DATADIR) -type f)

# Object files
C_OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.c.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.s.o,$(ASM_SOURCES))
DATA_OBJECTS := $(patsubst $(DATADIR)/%, $(OBJDIR)/%.o, $(DATA_SOURCES))

# Targets
TARGET := $(BINDIR)/libcommon.a

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(ASM_OBJECTS) $(DATA_OBJECTS)
	@echo !==== linking ====!
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	@echo !==== Compiling $^ ====!
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.s.o: $(SRCDIR)/%.s
	@echo !==== Compiling $^ ====!
	@mkdir -p $(dir $@)
	$(CC) $(ASMFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(DATADIR)/%
	@echo !==== Embeding $^ ====!
	@mkdir -p $(dir $@)
	aarch64-elf-ld -r -b binary -o $@ $<

clean:
	rm -rf $(OBJDIR) $(TARGET) $(BINDIR)/kernel.elf
	rm -rf $(OBJDIR) $(TARGET) $(BINDIR)/kernel8.img

lines_of_code:
	wc -l `find ./src -type f` +  wc -l `find ./inc -type f`

.PHONY: all clean