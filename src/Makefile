# Nathan Gardner
# Makefile
# CSC 4100 Operating Systems - Homework 2

CC = gcc

all: boot2 boot1

boot2: kernel.exe
	objcopy -j .text* -j .data* -j .rodata* -S -O binary kernel.exe boot2

boot1: kernel.exe boot1.asm
	nasm -DENTRY=0x$(shell nm kernel.exe | grep main | cut -d " " -f 1) boot1.asm

kernel.exe: kernel_c.o kernel_asm.o buddy_c.o convert_c.o queue_c.o
	ld -g -melf_i386 -Ttext 0x10000 -e main -o kernel.exe kernel_c.o kernel_asm.o convert_c.o buddy_c.o queue_c.o

kernel_c.o: kernel.c buddy.h kernel.h io.h queue.h convert.h
	$(CC) -g -m32 -fno-stack-protector -c -o kernel_c.o kernel.c

kernel_asm.o: kernel.asm
	nasm -g -f elf -F dwarf -o kernel_asm.o kernel.asm

convert_c.o: convert.c
	$(CC) -g -m32 -fno-stack-protector -c -o convert_c.o convert.c

buddy_c.o: buddy.c io.h convert.h buddy.h
	$(CC) -g -m32 -fno-stack-protector -c -o buddy_c.o buddy.c

queue_c.o: queue.c queue.h buddy.h
	$(CC) -g -m32 -fno-stack-protector -c -o queue_c.o queue.c

.PHONY: install
install:
	dd if=/dev/zero of=a.img bs=512 count=2880
	/sbin/mkdosfs a.img
	dd if=boot1 of=a.img bs=1 count=512 conv=notrunc
	mcopy -o boot2 a:BOOT2

.PHONY: run
run:
	qemu-system-i386 -drive format=raw,file=a.img,if=floppy

.PHONY: debug
debug:
	qemu-system-i386 -S -s -boot a -fda a.img &
	ddd --debugger 'gdb -ex "target remote localhost:1234" -ex "break main" -ex "continue"' kernel.exe

.PHONY: clean
clean:
	rm -f *.o *.exe boot1 boot2 a.img
