
#include "isr.h"
#include "idt.h"
#include "debug.h"

#include <stdint.h>

namespace myos
{
	/* These are function prototypes for all of the exception
	*  handlers: The first 32 entries in the IDT are reserved
	*  by Intel, and are designed to service exceptions! */
	extern "C" void isr0();
	extern "C" void isr1();
	extern "C" void isr8();
	extern "C" uint32_t getaddress();

	extern "C" void get_clk_isr();
	extern "C" void get_kbd_isr();
	extern "C" void put_dsp_isr();

	/* This is a very repetitive function... it's not hard, it's
	*  just annoying. As you can see, we set the first 32 entries
	*  in the IDT to the first 32 ISRs. We can't use a for loop
	*  for this, because there is no way to get the function names
	*  that correspond to that given entry. We set the access
	*  flags to 0x8E. This means that the entry is present, is
	*  running in ring 0 (kernel level), and has the lower 5 bits
	*  set to the required '14', which is represented by 'E' in
	*  hex. */
	void isrs_install()
	{
		//magic_breakpoint_msg("in isrs install\n");
		//unsigned long base = getaddress();
		idt_set_gate(0, (unsigned) isr0, 0x08, 0x8E);
		idt_set_gate(1, (unsigned) isr1, 0x08, 0x8E);
		idt_set_gate(8, (unsigned) isr8, 0x08, 0x8E);
		//    idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);

		/* Fill in the rest of these ISRs here */

		//  idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
		// idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);

		idt_set_gate(0x50, (unsigned) get_clk_isr, 0x08, 0x8E);
		idt_set_gate(0x51, (unsigned) get_kbd_isr, 0x08, 0x8E);
		idt_set_gate(0x52, (unsigned) put_dsp_isr, 0x08, 0x8E);
	}

	/* This is a simple string array. It contains the message that
	*  corresponds to each and every exception. We get the correct
	*  message by accessing like:
	*  exception_message[interrupt_number] */
	const char *exception_messages [] =
	{
		"Division By Zero",
		"Debug",
		"Non Maskable Interrupt",

		/* Fill in the rest here from our Exceptions table */

		"Reserved",
		"Reserved"
	};

	/* All of our Exception handling Interrupt Service Routines will
	*  point to this function. This will tell us what exception has
	*  happened! Right now, we simply halt the system by hitting an
	*  endless loop. All ISRs disable interrupts while they are being
	*  serviced as a 'locking' mechanism to prevent an IRQ from
	*  happening and messing up kernel data structures */
	void fault_handler(struct regs *r)
	{

		magic_breakpoint_msg("In fault handler \n");
		/* Is this a fault whose number is from 0 to 31? */
		if (r->int_no < 32)
		{
			/* Display the description for the Exception that occurred.
			*  In this tutorial, we will simply halt the system using an
			*  infinite loop */
			magic_breakpoint_msg(exception_messages[r->int_no]);
			magic_breakpoint_msg(" Exception. System Halted!\n");
			for (;;);
		}
	}

}