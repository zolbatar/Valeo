# Compiler flags etc.
INCLUDE   = -I include/ -I include/lib 
CCFLAGS   = -O3 -march=i686 -fno-builtin -nostdinc -fno-builtin -Wall
AS 	  = i386-elf-gcc $(CCFLAGS) $(INCLUDE) -c
CC 	  = i386-elf-gcc $(CCFLAGS) $(INCLUDE) -c
LD        = i386-elf-ld -nostdlib -N -T kernel.ld
OBJDUMP   = i386-elf-objdump -D -S -x
STRIP     = i386-elf-strip

# kernel
KERNELOBJS =  src/boot.o \
	      src/interrupt.o \
	      src/kernel-main.o \
	      src/mm.o \
	      src/mm_page.o \
	      src/mm_allocator.o \
	      src/mm_pp.o \
	      src/resource.o \
	      src/syslog.o \
	      src/timer.o \
	      src/arch/x86/8259a.o \
	      src/arch/x86/8254.o \
	      src/arch/x86/textmodevideo.o \
	      src/arch/x86/descriptor.o \
	      src/arch/x86/exception.o \
              src/lib/ctype.o \
              src/lib/printf.o \
              src/lib/scanf.o \
              src/lib/strtol.o
KERNEL =      x86-kernel

# Make rules
.S.o:
	$(AS) -o $@ $<

.c.o:
	$(CC) $< -o $@

all:	$(KERNEL)

$(KERNEL): $(KERNELOBJS)
	$(LD) $(KERNELOBJS) -o $(KERNEL)
	$(OBJDUMP) $(KERNEL) > $(KERNEL).dis
	$(STRIP) $(KERNEL)

install: $(KERNEL)
	 cp $(KERNEL) A:\System\Microkernel
	 sync

clean:
	-rm $(KERNEL)
	-rm $(KERNEL).dis
	-rm $(KERNELOBJS)
