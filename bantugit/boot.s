# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot


.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM



.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.global _start

#io_asm.h
.global inportb
.global inportw
.global inportl
.global outportb
.global outportw
.global outportl
.global rep_inportw
.global rep_outportw

#debug.h
.global magic_breakpoint

#lock_asm.h
.global asm_acquire_lock
.global asm_release_lock

# mittal boot.s START

.global getaddress
.global gdt_flush
.global gdtptr
.global memsetinternalzero 
.global idt_load
.global loadidtp
.global isr0
.global isr1
.global isr8

.global irq0
.global irq1
.global irq15
.global setinterrupt

#mittal boot.s END

#paging_asm.h
.global flush_tlb_asm
.global paging_enable_asm
.global is_paging_asm
.global load_pdbr_asm
.global interrupt1asm
.global get_pdbr_asm

.section .text

.type _start, @function
_start:
	
	movl $stack_top, %esp
	pushl %ebx
	call kernel_main

	cli
	hlt
.Lhang:
	jmp .Lhang

.type inportb, @function
inportb:
	pushl %edx
	movl 8(%esp), %edx #port
	
	inb %dx, %al
	popl %edx
	ret

.type inportw, @function
inportw:
	pushl %edx
	movl 8(%esp), %edx #port
	
	inw %dx, %ax
	popl %edx
	ret

.type inportl, @function
inportl:
	pushl %edx
	movl 8(%esp), %edx #port
	
	inl %dx, %eax
	popl %edx
	ret
	
.type outportb, @function
outportb:
	pushl %edx
	pushl %eax
	movl 12(%esp), %edx #port
	movl 16(%esp), %eax #data
	
	outb %al, %dx
	popl %eax
	popl %edx
	ret

.type outportw, @function
outportw:
	pushl %edx
	pushl %eax
	movl 12(%esp), %edx #port
	movl 16(%esp), %eax #data
	
	outw %ax, %dx
	popl %eax
	popl %edx
	ret

.type outportl, @function
outportl:
	push %edx
	push %eax
	movl 12(%esp), %edx #port
	movl 16(%esp), %eax #data
	
	outl %eax, %dx
	popl %eax
	popl %edx
	ret

.type rep_inportw, @function
rep_inportw:
	pushl %edx
	pushl %edi
	pushl %ecx
	movl 16(%esp), %edx #port
	movl 20(%esp), %edi #buffer
	movl 24(%esp), %ecx #word count - to be transferred
	rep insw
	popl %ecx
	popl %edi
	popl %edx
	ret

.type rep_outportw, @function
rep_outportw:
	pushl %edx
	pushl %esi
	pushl %ecx
	movl 16(%esp), %edx #port
	movl 20(%esp), %esi #buffer
	movl 24(%esp), %ecx #word count - to be transferred
	rep outsw
	popl %ecx
	popl %esi
	popl %edx
	ret

.type magic_breakpoint, @function
magic_breakpoint:
	xchg %bx, %bx
	ret

.type memsetinternalzero, @function
memsetinternalzero:
movl 4(%esp),%eax  #base
movl 8(%esp),%ecx #count

setzero:
movb $0,(%eax)
inc %eax
loop setzero

ret


# This will set up our new segment registers. We need to do
# something special in order to set CS. We do what is called a
#far jump. A jump that includes a segment as well as an offset.
# This is declared in C as 'extern void gdt_flush();'
    #Allows the C code to link to this
           #Says that '_gp' is in another file

.extern gp 
.type gdt_flush, @function
gdt_flush:

    lgdt (gp)        #Load the GDT with our '_gp' which is a special pointer
    mov $0x10,%ax      #0x10 is the offset in the GDT to our data segment
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%gs
    mov %ax,%ss
    jmp $0x08,$flush2   #0x08 is the offset to our code segment: Far jump!
flush2:
    ret               # Returns back to the C code!

.extern idtp
.type idt_load, @function
idt_load:
lidt (idtp)
ret

.type gdtptr, @function
#base address of ds is 0
gdtptr:
#leal eax,gdt[0];
leal gdt,%eax
ret


.type loadidtp, @function
loadidtp:
leal idt,%eax  #this gives absolute add of idt as data segment is at 0
ret

.type getaddress, @function
getaddress:
leal isr0,%eax
ret

#0: Divide By Zero Exception
.type isr0, @function
isr0:
    cli
    push $0    #A normal ISR stub that pops a dummy error code to keep a
                   # uniform stack frame
    push $0
    jmp isr_common_stub

#  1: Debug Exception
.type isr1, @function
isr1:
    cli
    push $0
    push $1
    jmp isr_common_stub


#  8: Double Fault Exception (With Error Code!)
.type isr8, @function
isr8:
    cli
    push $8    #Note that we DON'T push a value on the stack in this one!
                   # It pushes one already! Use this type of stub for exceptions
                   #that pop error codes!
    jmp isr_common_stub

          # You should fill in from _isr9 to _isr31 here. Remember to
           # use the correct stubs to handle error codes and push dummies!


#This is our common ISR stub. It saves the processor state, sets
# up for kernel mode segments, calls the C-level fault handler,
# and finally restores the stack frame.
.type isr_common_stub, @function
isr_common_stub:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10,%ax   # Load the Kernel Data Segment descriptor!
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%gs
    mov %esp,%eax   # Push us the stack
    push %eax
    mov $fault_handler,%eax
    call *%eax       #A special call, preserves the 'eip' register
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8,%esp    # Cleans up the pushed error code and pushed ISR number
    iret           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!



.type irq0,@function
irq0:
    cli
    push $0    
    push $32   #interrupt number 32
    jmp irq_common_stub
ret

.type irq1,@function
irq1:
  cli
  push $0
  push $33
  jmp irq_common_stub
ret

#interrupt 47: IRQ15
.type irq15,@function
irq15:
    cli
    push $0
    push $47
    jmp irq_common_stub
ret

.type irq_common_stub,@function
irq_common_stub:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10,%ax
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%gs
    mov %esp,%eax
    push %eax
    mov $irq_handler,%eax
    call *%eax
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8,%esp
    iret

.type setinterrupt,@function
setinterrupt:
sti
ret

.type asm_acquire_lock,@function
asm_acquire_lock:
retry:
    lock bts $0, 4(%esp)
    jc retry
    ret
 
.type asm_release_lock,@function 
asm_release_lock:
    lock btr $0, 4(%esp)
    ret		

.type flush_tlb_asm,@function
flush_tlb_asm:
             movl 4(%esp),%eax           
		cli
		invlpg (%eax)	
		sti
	
ret
.type paging_enable_asm,@function
paging_enable_asm:
mov	%cr0,%eax
or      0x80000000,%eax		//set bit 31
mov	%eax,%cr0
ret
.type is_paging_asm,@function
is_paging_asm:
mov %cr0,%eax
ret

.type load_pdbr_asm,@function
load_pdbr_asm:
mov 4(%esp),%eax
mov %eax,%cr3
ret
.type get_pdbr_asm,@function
get_pdbr_asm:
mov %cr3,%eax
ret

.global put_dsp_isr
.type put_dsp_isr, @function
put_dsp_isr:
	cli
	pushal
    push %eax
	call sys_putchar
	pop %eax
    popal
	sti
    iret

.global get_kbd_isr
.type get_kbd_isr, @function
get_kbd_isr:
	push %ebx
	push %ecx
	push %edx
	push %esi
	push %edi
	push %ebp
	sti
	call sys_getchar
    pop %ebp
	pop %edi
	pop %esi
	pop %edx
	pop %ecx
	pop %ebx
    iret

.global get_clk_isr
.type get_clk_isr, @function
get_clk_isr:
	push %ebx
	push %ecx
	push %edx
	push %esi
	push %edi
	push %ebp
	call sys_getclock
    pop %ebp
	pop %edi
	pop %esi
	pop %edx
	pop %ecx
	pop %ebx
    iret

.global call_process_at
.type call_process_at, @function
call_process_at:
	sti
	movl 4(%esp), %eax
	pusha
#	xchg %bx, %bx
	call %eax
	popa
	xor %eax, %eax
	ret

.size _start, . - _start
