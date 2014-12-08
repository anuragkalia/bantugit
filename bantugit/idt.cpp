#include "idt.h"
#include "debug.h"
#include "stdfunc.h"

namespace myos
{
	struct idt_entry
	{
		uint16_t base_lo;
		uint16_t sel;        /* Our kernel segment goes here! */
		uint8_t always0;     /* This will ALWAYS be set to 0! */
		uint8_t flags;       /* Set using the above table! */
		uint16_t base_hi;
	} __attribute__((packed));

	struct idt_ptr
	{
		uint16_t limit;
		idt_entry * base;
	} __attribute__((packed));

	/*In boot.s*/

	extern "C" void idt_load();
	extern "C" idt_entry * loadidtp();
}

myos::idt_entry idt[256];
myos::idt_ptr idtp;

namespace myos
{
	/* Declare an IDT of 256 entries. Although we will only use the
	*  first 32 entries in this tutorial, the rest exists as a bit
	*  of a trap. If any undefined IDT entry is hit, it normally
	*  will cause an "Unhandled Interrupt" exception. Any descriptor
	*  for which the 'presence' bit is cleared (0) will generate an
	*  "Unhandled Interrupt" exception */
	
	/* Use this function to set an entry in the IDT. Alot simpler
	*  than twiddling with the GDT ;) */
	void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
	{
		/* We'll leave you to try and code this function: take the
		*  argument 'base' and split it up into a high and low 16-bits,
		*  storing them in idt[num].base_hi and base_lo. The rest of the
		*  fields that you must set in idt[num] are fairly self-
		*  explanatory when it comes to setup */
		//log(OK, "In idt_set gate \n");
		idt[num].sel = sel;
		idt[num].flags = flags;
		idt[num].base_lo = base & 0xFFFF;
		idt[num].base_hi = (base >> 16) & 0xFFFF;
		idt[num].always0 = 0;


	}

	/* Installs the IDT */
	void idt_install()
	{
		static_assert(sizeof(idt_entry) == 8, "sizeof idt entry is not 64-bits");
		static_assert(sizeof(idt_ptr) == 6, "sizeof idt_ptr is not 48-bits");
		static_assert(sizeof(idt_entry *) == 4, "sizeof idt_entry * is not 4 bytes \n");

		/* Sets the special IDT pointer up, just like in 'gdt.c' */
		idtp.limit = (sizeof(idt_entry) * 256) - 1;
		idtp.base = loadidtp();

		/* Clear out the entire IDT, initializing it to zeros */
		//memsetinternalzero(idtp.base, sizeof(struct idt_entry) * 256);
		
		idt_entry allzero_entry;
		memset(reinterpret_cast<uint8_t *>(&allzero_entry), uint8_t(0), sizeof(allzero_entry));
		
		memset(idtp.base, allzero_entry, 256);
		/* Add any new ISRs to the IDT here using idt_set_gate */

		/* Points the processor's internal register to the new IDT */
		idt_load();
	}
}