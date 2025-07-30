# Define the cross-compiler and other tools
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
AS = nasm

# Compiler flags:
# -ffreestanding: Compile without relying on standard library functions (essential for OS development).
# -O2: Optimization level 2.
# -Wall -Wextra: Enable all common and extra warning messages.
CFLAGS = -ffreestanding -O2 -Wall -Wextra

# Linker flags:
# -T linker.ld: Use the specified linker script.
LDFLAGS = -T linker.ld

# List of kernel object files.
# Make sure the paths match your project structure (e.g., boot/ for boot.o, kernel/ for C files).
KERNEL_OBJS = boot/boot.o kernel/kernel.o kernel/kprint.o kernel/kinput.o kernel/kutils.o kernel/kmath.o

# Default target: builds the ISO image.
all: iso/boot/kernel.elf grub.iso

# Rule to compile boot.asm into boot/boot.o.
# -f elf64: Output in ELF64 format.
boot/boot.o: boot/boot.asm
	$(AS) -f elf64 $< -o $@

# Generic rule to compile any .c file into a .o file.
# This assumes C source files are in the 'kernel/' directory.
# For example, kernel/kernel.c -> kernel/kernel.o
# kernel/kprint.c -> kernel/kprint.o
# kernel/kinput.c -> kernel/kinput.o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link all kernel object files into the final ELF executable.
# The executable will be placed in iso/boot/kernel.elf as required by GRUB.
iso/boot/kernel.elf: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Rule to create the GRUB bootable ISO image.
# This rule now explicitly creates the grub.cfg file.
grub.iso: iso/boot/kernel.elf
	# Create the directory for grub.cfg if it doesn't exist
	mkdir -p iso/boot/grub
	# Create the grub.cfg file with a simple menu entry for our kernel
	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "My VERY WORKING OS" {' >> iso/boot/grub/grub.cfg
	echo '    multiboot2 /boot/kernel.elf' >> iso/boot/grub/grub.cfg
	echo '    boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	# Use grub-mkrescue to create the ISO from the 'iso' directory
	grub-mkrescue -o grub.iso iso

# Clean target: removes all generated object files and the ISO.
clean:
	rm -f $(KERNEL_OBJS) iso/boot/kernel.elf grub.iso
	rm -rf iso/boot/grub # Also remove the generated grub directory
