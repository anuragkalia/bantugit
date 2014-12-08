#ifndef __TERMINAL_H
#define __TERMINAL_H

#include <stdint.h>
#include <stddef.h>

namespace myos
{
	extern uint8_t terminal_color;
	//extern size_t terminal_row;
	//extern size_t terminal_column;
	//extern uint16_t* terminal_buffer;

	/*In terminal.cpp*/
	void terminal_movecursor(int row, int col);

	void terminal_putchar(char c);
	void terminal_writestring(const char* data);
	void terminal_writenumber(uint32_t num, int base = 10);
	void terminal_writebool(bool cond);
	
	void terminal_setcolor(uint8_t color);

	void terminal_clear(size_t ul_x, size_t ul_y, size_t br_x, size_t br_y, uint8_t tcolor = terminal_color);
	void terminal_clear(uint8_t tcolor = terminal_color);
	void terminal_scroll(size_t n);
	void terminal_init();

	extern "C" void sys_putchar(char c);
}

#endif //__TERMINAL_H