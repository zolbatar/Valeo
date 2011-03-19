#include <asm-macros.h>

	.text
	.globl	Start
Start:
	jmp	entry

	.ALIGN	4
multiboot_header:
	.long 0x1BADB002 // Magic number
	.long 0x00000003 // Multiboot flags
	.long -(0x1BADB002 + 0x00000003)

	.ALIGN	4
	.globl gdt
gdt:
	.quad 0x0000000000000000
	.quad 0x00cf9a000000ffff // 0x08 Kernel Code 4GB @ 0x00000000
	.quad 0x00cf92000000ffff // 0x10 Kernel Data 4GB @ 0x00000000
	.quad 0x0000000000000000 // 0x18 Task Code
	.quad 0x0000000000000000 // 0x20 Task Data
	.quad 0x0000000000000000 // 0x28 TSS Data
gdtr:
	.word	6 * 8 -1
	.long	gdt

	.ALIGN	4
	.globl idt
	.globl idtr
idt:
	.skip   256 * 8, 0
idtr:
    .word   256 * 8	- 1	 // IDT contains 256 entries
	.long   idt 

	.globl  KernelSize
KernelSize:
	.long 0x00000000
	.globl  MultibootAddress
MultibootAddress:
        .long 0x00000000

#define KernelPhysical 0x100000
#define KernelRelocateAddress 0xF0000000
#define PageDirectoryRelocate 0xFFC00000
#define ADDRADJUST	0xEFF00000

// Logical
// -------
// Kernel at 0xF0000000
//
// Physical
// --------
//
// At 0x200000, we have:
// Page Directory (One Page)
// Core Page Table For 0xF0000000 (One Page)
// Stack (16 Pages)
// Zero Page Table (One Page)

#define PageDirectory 0x200000
#define CorePageTable 0x201000
#define Stack 0x202000
#define StackPages 16
#define ZeroPageTable (Stack + (4096 * StackPages))

entry:
// Note all the ADDRADJUST references are to adjust addresses meant
// for 0xF0000000 to 0x100000 (where the kernel is loaded)
// Once we jump to 0xF0000000 we can stop doing this

        // Calculate kernel size
        movl	$_end, %edi
        subl    $Start, %edi
        addl    $4095, %edi
        shr     $12, %edi
        movl    %edi, (KernelSize - ADDRADJUST)

        // Create stack
        movl	$Stack + (4096 * StackPages), %edi
        movl	%edi, %esp

        // Check multiboot magic number is correct
	cmpl	$0x2BADB002, %eax
	je	multiboot

        // Halt because bootloader is bad (i.e. not grub)
	jmp	halt

multiboot:

        // Save ptr to multiboot structure
	movl    %ebx, (MultibootAddress - ADDRADJUST)

        // Clear a page for page directory, set to 2
        movl    $PageDirectory, %edi
        movl    $1024, %ecx
        movl    $0x2, %eax
        cld
        rep stosl

	// Zero out two pages for core page table/zero page at end of kernel
        movl    $CorePageTable, %edi
        movl    $2048, %ecx
        movl    $0x0, %eax
        cld
        rep stosl

        // Map kernel from virtual 0xF0000000 to physical 0x100000
        movl    $CorePageTable, %edi
        movl    (KernelSize - ADDRADJUST), %ecx
        movl    $KernelPhysical | 0x3, %eax
        cld
mapkernel:
        stosl
        addl    $4096, %eax
        loop    mapkernel

	// Now map page directory just after kernel
        movl    $1, %ecx
        movl    $PageDirectory | 0x3, %eax
        cld
mappagedirectory:
        stosl
        addl    $4096, %eax
        loop    mappagedirectory

	// Now map stack
        movl	$StackPages, %ecx
        movl    $Stack + 0x3, %eax
        cld
mapstack:
        stosl
        addl    $4096, %eax
        loop    mapstack

        // Map the core page table entry (for 0xF0000000) into the page directory
        movl    $CorePageTable | 0x3, %eax
        movl    $PageDirectory, %edi
        addl    $KernelRelocateAddress >> 20, %edi
        movl    %eax, (%edi)

        // Map the page directory (for 0xFFC00000) into the page directory
        movl    $PageDirectory | 0x3, %eax
        movl    $PageDirectory, %edi
        addl    $PageDirectoryRelocate >> 20, %edi
        movl    %eax, (%edi)

        // Map first 4MB address space
        // We do this so we can access Multiboot structure and also
        // to stop segfault when we switch to 0xF0000000
        // (we will clear this once we jump to virtual)
        movl    $ZeroPageTable, %edi
        movl    $1024, %ecx
        movl    $0x3, %eax
        cld
map4MBSpace:
        stosl
        addl    $4096, %eax
        loop    map4MBSpace

        // Map the zero page table entry (for 0x00000000) into the page directory
        movl	$ZeroPageTable | 0x3, %eax
        movl    $PageDirectory, %edi
        movl    %eax, (%edi)

        // Enable paging
        movl    $PageDirectory, %eax
        movl    %eax, %cr3
        movl    %cr0, %eax
        orl     $0x80000000, %eax
        movl    %eax, %cr0

        pushl   $Paging_Enabled
        ret
Paging_Enabled:
        // Jump for joy, we are now running at 0xF0000000+, Yippee!!

	// Load our GDT
	lgdtl   gdtr

	// And reload cs/ds/es/fs/gs, do a long jump after to reload segment descriptor
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss
	ljmp	$0x08, $reloadcs
reloadcs:
        // Update stack pointer
        movl	$KernelRelocateAddress - Stack, %eax
        addl    %eax, %esp

	// Jump to main, start of C code
	movl    (MultibootAddress), %eax
	pushl   %eax
	call	Kernel_Main

	// Halt
	.globl halt
halt:
	cli
	jmp halt

DefaultIntHandlerMsg:
	.asciz "Unknown interrupt - Halting\n"
DefaultExceptionMsg:
	.asciz "Unhandled Exception %d - Halting\n"
ModuleName:
	.asciz "BOOT.S"

	.ALIGN 	4
	.globl DefaultIntHandler
DefaultIntHandler:
	pushl	$2
	pushl 	$DefaultIntHandlerMsg
	pushl	$ModuleName
	call	Syslog_Entry
	jmp halt

	.globl EXC_00
EXC_00:
        ENTEREXCEPTION
	pushl	$0x00
	pushl 	$DefaultExceptionMsg
	pushl	$ModuleName
	pushl	$2
	call	Syslog_Entry
	cli
	hlt
        EXITEXCEPTION

	.globl EXC_01
EXC_01:
        ENTEREXCEPTION
	call	Exception_Debug
        cli
        hlt
        EXITEXCEPTION

	.globl EXC_02
EXC_02:
        ENTEREXCEPTION
	pushl	$0x02
	pushl 	$DefaultExceptionMsg
	pushl	$ModuleName
	pushl	$2
	call	Syslog_Entry
	cli
	hlt

	.globl EXC_03
EXC_03:
        ENTEREXCEPTION
	pushl	$0x03
	pushl 	$DefaultExceptionMsg
	pushl	$ModuleName
	pushl	$2
	call	Syslog_Entry
	cli
	hlt

	.globl EXC_04
EXC_04:
        ENTEREXCEPTION
	pushl	$0x04
	pushl 	$DefaultExceptionMsg
	pushl	$ModuleName
	pushl	$2
	call	Syslog_Entry
	cli
	hlt

	.globl EXC_05
EXC_05:
        ENTEREXCEPTION
	pushl	$0x05
	pushl 	$DefaultExceptionMsg
	pushl	$ModuleName
	pushl	$2
	call	Syslog_Entry
	cli
	hlt
        EXITEXCEPTION

	.globl EXC_INVALIDOPCODE
EXC_INVALIDOPCODE:
        ENTEREXCEPTION
	call	Exception_InvalidOpCode
	cli
	hlt
        EXITEXCEPTION

	.globl EXC_07
EXC_07:
        ENTEREXCEPTION
	pushl	$0x07
	pushl 	$DefaultExceptionMsg
	pushl	$ModuleName
	pushl	$2
	call	Syslog_Entry
	cli
	hlt
        EXITEXCEPTION

	.globl EXC_DOUBLEFAULT
EXC_DOUBLEFAULT:
        ENTEREXCEPTION
	call	Exception_Double
	cli
	hlt
        EXITEXCEPTION_WITHERROR

	.globl EXC_09
EXC_09:
        ENTEREXCEPTION
	pushl	$0x09
	pushl 	$DefaultExceptionMsg
	pushl	$ModuleName
	pushl	$2
	call	Syslog_Entry
	cli
	hlt
        EXITEXCEPTION

	.globl EXC_0A
EXC_0A:
        ENTEREXCEPTION
	pushl	$0x0A
	pushl 	$DefaultExceptionMsg
	pushl	$ModuleName
	pushl	$2
	call	Syslog_Entry
	cli
	hlt
        EXITEXCEPTION_WITHERROR

	.globl EXC_0B
EXC_0B:
        ENTEREXCEPTION
	pushl	$0x0B
	pushl 	$DefaultExceptionMsg
	pushl	$ModuleName
	pushl	$2
	call	Syslog_Entry
	cli
	hlt
        EXITEXCEPTION_WITHERROR

	.globl EXC_STACKFAULT
EXC_STACKFAULT:
        ENTEREXCEPTION
	call	Exception_StackFault
        cli
        hlt
        EXITEXCEPTION_WITHERROR

	.globl EXC_GPF
EXC_GPF:
        ENTEREXCEPTION
	call	Exception_GPF
        cli
        hlt
        EXITEXCEPTION_WITHERROR

	.globl EXC_PAGEFAULT
EXC_PAGEFAULT:
        ENTEREXCEPTION
	call	Exception_Page
        EXITEXCEPTION_WITHERROR

	.globl IRQ_00
IRQ_00:
        ENTER_INTERRUPT
        pushl   $0x00
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_01
IRQ_01:
        ENTER_INTERRUPT
        pushl   $0x01
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_02
IRQ_02:
        ENTER_INTERRUPT
        pushl   $0x02
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_03
IRQ_03:
        ENTER_INTERRUPT
        pushl   $0x03
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_04
IRQ_04:
        ENTER_INTERRUPT
        pushl   $0x04
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_05
IRQ_05:
        ENTER_INTERRUPT
        pushl   $0x05
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_06
IRQ_06:
        ENTER_INTERRUPT
        pushl   $0x06
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_07
IRQ_07:
        ENTER_INTERRUPT
        pushl   $0x07
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_08
IRQ_08:
        ENTER_INTERRUPT
        pushl   $0x08
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_09
IRQ_09:
        ENTER_INTERRUPT
        pushl   $0x09
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_0A
IRQ_0A:
        ENTER_INTERRUPT
        pushl   $0x0A
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_0B
IRQ_0B:
        ENTER_INTERRUPT
        pushl   $0x0B
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_0C
IRQ_0C:
        ENTER_INTERRUPT
        pushl   $0x0C
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_0D
IRQ_0D:
        ENTER_INTERRUPT
        pushl   $0x0D
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_0E
IRQ_0E:
        ENTER_INTERRUPT
        pushl   $0x0E
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT

	.globl IRQ_0F
IRQ_0F:
        ENTER_INTERRUPT
	pushl   $0x0F
	call	Interrupt_Handle
	addl 	$4, %esp
        EXIT_INTERRUPT
