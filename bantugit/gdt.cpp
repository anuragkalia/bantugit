#include "gdt.h"

namespace myos
{
	struct gdt_entry
	{
		uint16_t limit_low;
		uint16_t base_low;
		uint8_t base_middle;
		uint8_t access;
		uint8_t granularity;
		uint8_t base_high;
	} __attribute__((packed));

	/* Special pointer which includes the limit: The max bytes
	*  taken up by the GDT, minus 1. Again, this NEEDS to be packed */
	struct gdt_ptr
	{
		uint16_t limit;
		uint32_t base;
	} __attribute__((packed));
}
	myos::gdt_entry gdt[3];
	myos::gdt_ptr gp;

namespace myos
{
	/* Our GDT, with 3 entries, and finally our special GDT pointer */

	/* Setup a descriptor in the Global Descriptor Table */
	void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
	{
		/* Setup the descriptor base address */
		gdt[num].base_low = (base & 0xFFFF);
		gdt[num].base_middle = (base >> 16) & 0xFF;
		gdt[num].base_high = (base >> 24) & 0xFF;

		/* Setup the descriptor limits */
		gdt[num].limit_low = (limit & 0xFFFF);
		gdt[num].granularity = ((limit >> 16) & 0x0F);

		/* Finally, set up the granularity and access flags */
		gdt[num].granularity |= (gran & 0xF0);
		gdt[num].access = access;
	}

	/* Should be called by main. This will setup the special GDT
	*  pointer, set up the first 3 entries in our GDT, and then
	*  finally call gdt_flush() in our assembler file in order
	*  to tell the processor where the new GDT is and update the
	*  new segment registers */
	void gdt_install()
	{
		static_assert(sizeof(gdt_entry) == 8, "sizeof gdt entry is not 64-bits");
		static_assert(sizeof(gdt_ptr) == 6, "sizeof gdt_ptr is not 48-bits");
		/* Setup the GDT pointer and limit */
		gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
		gp.base = gdtptr();


		/* Our NULL descriptor */
		gdt_set_gate(0, 0, 0, 0, 0);

		/* The second entry is our Code Segment. The base address
		*  is 0, the limit is 4GBytes, it uses 4KByte granularity,
		*  uses 32-bit opcodes, and is a Code Segment descriptor.
		*  Please check the table above in the tutorial in order
		*  to see exactly what each value means */
		gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

		/* The third entry is our Data Segment. It's EXACTLY the
		*  same as our code segment, but the descriptor type in
		*  this entry's access byte says it's a Data Segment */
		gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

		/* Flush out the old GDT and install the new changes! */
		gdt_flush();
	}

}