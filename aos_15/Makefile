main.o:main.c
	gcc -m32 -fno-builtin -c -o main.o main.c

init.o:init.c
	gcc -m32 -fno-builtin -c -o init.o init.c 

interrupt.o:interrupt.c
	gcc -m32 -fno-stack-protector -fno-builtin -c -o interrupt.o interrupt.c 

print.o:print.asm
	nasm -f elf32 -o print.o print.asm

kernel.o:kernel.asm
	nasm -f elf32 -o kernel.o kernel.asm 

debug.o:debug.c
	gcc -m32 -fno-builtin -c -o debug.o debug.c

string.o:string.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o string.o string.c

bitmap.o:bitmap.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o bitmap.o bitmap.c

memory.o:memory.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o memory.o memory.c

thread.o:thread.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o thread.o thread.c

timer.o:timer.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o timer.o timer.c

deque.o:deque.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o deque.o deque.c

sync.o:sync.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o sync.o sync.c

console.o:console.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o console.o console.c

switch.o:switch.asm
	nasm -f elf32 -o switch.o switch.asm

keyboard.o:keyboard.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o keyboard.o keyboard.c

iobuf.o:iobuf.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o iobuf.o iobuf.c

tss.o:tss.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o tss.o tss.c

process.o:process.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o process.o process.c

syscall.o:syscall.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o syscall.o syscall.c

syscallinit.o:syscallinit.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o syscallinit.o syscallinit.c

stdio.o:stdio.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o stdio.o stdio.c

ide.o:ide.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o ide.o ide.c

stdiokernel.o:stdiokernel.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o stdiokernel.o stdiokernel.c

fs.o:fs.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o fs.o fs.c

file.o:file.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o file.o file.c

dir.o:dir.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o dir.o dir.c

inode.o:inode.c
	gcc -m32 -fno-builtin -fno-stack-protector -c -o inode.o inode.c

all:main.o init.o interrupt.o print.o kernel.o debug.o string.o bitmap.o memory.o thread.o timer.o switch.o deque.o keyboard.o sync.o console.o iobuf.o tss.o process.o syscall.o syscallinit.o stdio.o ide.o stdiokernel.o fs.o file.o dir.o inode.o
	ld -m elf_i386 -Ttext 0xc0001500 main.o init.o debug.o interrupt.o print.o kernel.o string.o bitmap.o memory.o thread.o timer.o switch.o deque.o iobuf.o keyboard.o sync.o console.o tss.o process.o syscall.o syscallinit.o stdio.o ide.o stdiokernel.o file.o fs.o dir.o inode.o -e main -o kernel.bin 

