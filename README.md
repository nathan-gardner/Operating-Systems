# OperatingSystems

This is one of the operating systems programs for my OS course. This is a non-preemptive implementation of a simple two stage operating system built around the x86 architecture. 

**BUILD PROCESS**
1. Run make clean to delete remaining build files: `>> make clean`
2. Make to compile assembly and C files for buddy, convert, kernel, and link them together to create kernel.exe, create the stage 1 and 2 boot loaders, boot1 and boot2: `>> make`
3. Create a virtual floppy install the stage 1 boot loader and the stage 2 OS: `>> make install`
4. Run the OS using qemu cli: `>> make run`
