# Directories
SRCDIR := src
BINDIR := bin
OBJDIR := $(BINDIR)/obj

# Compiler and linker options
COMPILERPREFIX = aarch64-elf-
CC := $(COMPILERPREFIX)gcc
AR := $(COMPILERPREFIX)ar
C_DEFINITIONS := -DAARCH64 -DCENTRAL_BLOCK_MEMORY_ALLOCATOR_PRINT_IF_FAILED_ALLOC -DCENTRAL_BLOCK_MEMORY_ALLOCATOR_PRINT_IF_FAILED_FREE

LIB_NAME := central_block_memory_allocator

OPATMANISER_SETTING = -O2

DEBUGFLAGS := -g

ASMFLAGS := -Iinc/$(LIB_NAME) $(DEBUGFLAGS)
CFLAGS := -ffreestanding -nostartfiles  -std=gnu99 -c -Iinc/$(LIB_NAME) -I../inc $(OPATMANISER_SETTING) -Wall -Werror $(C_DEFINITIONS)
ARFLAGS := rcs

# Source files
C_SOURCES := $(shell find $(SRCDIR) -name '*.c')
ASM_SOURCES := $(shell find $(SRCDIR) -name '*.s')

# Object files
C_OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.c.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.s.o,$(ASM_SOURCES))

# Targets
TARGET := $(BINDIR)/lib$(LIB_NAME).a

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(ASM_OBJECTS)
	@echo !==== linking ====!
	$(AR) $(ARFLAGS) $@ $^
	
	@# Yes it is ugly, but it works so no touchy

	ln -sf $(CURDIR)/bin/lib$(LIB_NAME).a $(CURDIR)/../bin/lib$(LIB_NAME).a
	ln -sfn ../$(LIB_NAME)/inc/$(LIB_NAME) $(CURDIR)/../inc/$(LIB_NAME)


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
	wc -l `find ./src -type f` +  wc -l `find ./inc -type f`

.PHONY: all clean
