# Directories
SRCDIR := src
BINDIR := bin
DATADIR := data
OBJDIR := $(BINDIR)/obj

# Compiler and linker options
COMPILERPREFIX = aarch64-elf-
CC := $(COMPILERPREFIX)gcc
LD := $(COMPILERPREFIX)ld
C_DEFINITIONS := -DAARCH64

OPATMANISER_SETTING = -O2

DEBUGFLAGS := -g

ASMFLAGS := -Iinc $(DEBUGFLAGS)
CFLAGS := -ffreestanding -nostartfiles  -std=gnu99 -c -Iinc $(OPATMANISER_SETTING) -Wall -Werror $(C_DEFINITIONS) -include inc/kconfig.h $(DEBUGFLAGS)
LDFLAGS := -T $(SRCDIR)/linker.ld $(OPATMANISER_SETTING) -nostdlib $(DEBUGFLAGS)

# Source files
C_SOURCES := $(shell find $(SRCDIR) -name '*.c')
ASM_SOURCES := $(shell find $(SRCDIR) -name '*.s')
DATA_SOURCES := $(shell find $(DATADIR) -type f)

# Object files
C_OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.c.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.s.o,$(ASM_SOURCES))
DATA_OBJECTS := $(patsubst $(DATADIR)/%, $(OBJDIR)/%.o, $(DATA_SOURCES))

# Targets
TARGET := $(BINDIR)/kernel.elf

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(ASM_OBJECTS) $(DATA_OBJECTS)
	@echo !==== linking ====!
	$(LD) -o $@ $^ $(LDFLAGS)
	@echo !==== Image Genoration ====!
	aarch64-elf-objcopy $(BINDIR)/kernel.elf -O binary $(BINDIR)/kernel8.img

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
	wc -l `find ./src -type f` `find ./inc -type f`

run_halt:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x00000,cpu-num=0 -drive file=$(BINDIR)/diskimage.img,if=sd,format=raw -display none -serial stdio -s -S 

run:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x00000,cpu-num=0 -drive file=$(BINDIR)/diskimage.img,if=sd,format=raw -display none -serial stdio


run_halt_serial_over_tcp:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x80000,cpu-num=0 -drive file=$(BINDIR)/diskimage.img,if=sd,format=raw -display none -serial tcp::4444,server=on -s -S

run_serial_over_tcp:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x80000,cpu-num=0 -drive file=$(BINDIR)/diskimage.img,if=sd,format=raw -display none -serial tcp::4444,server=on

run_halt_display:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x00000,cpu-num=0 -drive file=$(BINDIR)/diskimage.img,if=sd,format=raw -serial stdio -s -S

run_display:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x00000,cpu-num=0 -drive file=$(BINDIR)/diskimage.img,if=sd,format=raw -serial stdio


run_halt_serial_over_tcp_display:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x80000,cpu-num=0 -drive file=$(BINDIR)/diskimage.img,if=sd,format=raw -serial tcp::4444,server=on -s -S

run_serial_over_tcp_display:
	@echo !==== Running ====!
	qemu-system-aarch64 -M raspi3b -device loader,file=bin/kernel8.img,addr=0x80000,cpu-num=0 -drive file=$(BINDIR)/diskimage.img,if=sd,format=raw -serial tcp::4444,server=on


run_bootloader:
	@echo !==== Running ====!
	sudo python3 ./bootloader.py

mount_disk:
	@echo !==== Mounting disk image ====!
	@mkdir -p $(BINDIR)/mount_dir
	losetup -o1048576 /dev/loop0 $(BINDIR)/diskimage.img
	mount -tvfat -o uid=1000,gid=1000 /dev/loop0 $(BINDIR)/mount_dir

umount_disk:
	@echo !==== Unmounting disk image ====!
	-umount /dev/loop0
	losetup -d /dev/loop0

.PHONY: all clean