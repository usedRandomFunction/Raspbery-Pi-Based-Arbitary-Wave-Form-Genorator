# Directories
SRCDIR := src
BINDIR := bin
OBJDIR := $(BINDIR)/obj
LIBARY_DIR = ../libarys

# Compiler and linker options
COMPILERPREFIX = aarch64-elf-
CC := $(COMPILERPREFIX)gcc
LD := $(COMPILERPREFIX)ld
C_DEFINITIONS := -DAARCH64

OPATMANISER_SETTING = -O0	# gcc doesn't like me today

DEBUGFLAGS := -g

ASMFLAGS := -Iinc $(DEBUGFLAGS)
CFLAGS := -ffreestanding -std=gnu99 -c -Iinc -I$(LIBARY_DIR)/inc $(OPATMANISER_SETTING) -Wall -Werror $(C_DEFINITIONS) $(DEBUGFLAGS)
LDFLAGS := -T $(SRCDIR)/linker.ld $(OPATMANISER_SETTING) -nostdlib $(DEBUGFLAGS) -L$(LIBARY_DIR)/bin -lcommon

# Source files
C_SOURCES := $(shell find $(SRCDIR) -name '*.c')
ASM_SOURCES := $(shell find $(SRCDIR) -name '*.s')

# Object files
C_OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.c.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.s.o,$(ASM_SOURCES))

NAME := uartupld

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

run_emmulator:
	@echo !==== running ====!
	x-terminal-emulator -e 'cd ../../kernal; make run_serial_over_tcp_display'&
	sleep 0.5	# This isn't the best, should try something better
	python3 ../../uart_interface.py tcp:localhost:4444 --config_file "../../uart_interface_config.json" --exec "rua ./" 

run_halt_emmulator:
	@echo !==== running ====!
	x-terminal-emulator -e 'cd ../../kernal; make run_halt_serial_over_tcp_display'&
	sleep 0.5	# This isn't the best, should try something better
	python3 ../../uart_interface.py tcp:localhost:4444 --config_file "../../uart_interface_config.json" --exec "rua ./" 

clean:
	rm -rf $(OBJDIR) $(TARGET) $(TARGET)

lines_of_code:
		wc -l `find ./src -type f` `find ./inc -type f`

.PHONY: all clean