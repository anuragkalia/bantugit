#include "terminal.h"
#include "io_asm.h"
#include "stdfunc.h"
#include "debug.h"

namespace myos
{
	const size_t VGA_WIDTH = 80;
	const size_t VGA_HEIGHT = 25;

	size_t terminal_row;
	size_t terminal_column;
	uint8_t terminal_color;
	uint16_t* terminal_buffer;

	/*internal functions */

	void terminal_movecursor(int row = terminal_row, int col = terminal_column);
	void internal_terminal_putchar(char c);


	/* Hardware text mode color constants. */
	enum vga_color
	{
		COLOR_BLACK = 0,
		COLOR_BLUE = 1,
		COLOR_GREEN = 2,
		COLOR_CYAN = 3,
		COLOR_RED = 4,
		COLOR_MAGENTA = 5,
		COLOR_BROWN = 6,
		COLOR_LIGHT_GREY = 7,
		COLOR_DARK_GREY = 8,
		COLOR_LIGHT_BLUE = 9,
		COLOR_LIGHT_GREEN = 10,
		COLOR_LIGHT_CYAN = 11,
		COLOR_LIGHT_RED = 12,
		COLOR_LIGHT_MAGENTA = 13,
		COLOR_LIGHT_BROWN = 14,
		COLOR_WHITE = 15,
	};

	uint8_t make_color(enum vga_color fg, enum vga_color bg)
	{
		return fg | bg << 4;
	}

	uint16_t make_vgaentry(char c, uint8_t color)
	{
		uint16_t c16 = c;
		uint16_t color16 = color;
		return c16 | color16 << 8;
	}


	void terminal_setcolor(uint8_t color)
	{
		terminal_color = color;
	}

	void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
	{
		const size_t index = y * VGA_WIDTH + x;
		terminal_buffer[index] = make_vgaentry(c, color);
	}

	void terminal_writestring(const char* data);

	int scr_x(uint16_t *pixel)
	{
		int dist = pixel - terminal_buffer;
		return dist % VGA_WIDTH;
	}

	int scr_y(uint16_t *pixel)
	{
		int dist = pixel - terminal_buffer;
		return dist / VGA_WIDTH;
	}

	void terminal_movecursor(int row, int col)
	{
		uint16_t index = VGA_WIDTH * row + col;

#ifndef VCPP
		outportb(0x3D4, 0xF);
		outportb(0x3D5, uint8_t(index));
		outportb(0x3D4, 0xE);
		outportb(0x3D5, uint8_t(index >> 8));
#endif
	}

	void internal_terminal_putchar(char c)
	{
		if (c != '\n')
		{
			terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		}

		if (c == '\n' || ++terminal_column == VGA_WIDTH)
		{
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
			{
				terminal_scroll(1);
				--terminal_row;
			}
		}
	}

	void terminal_putchar(char c)
	{
#ifndef VCPP
		internal_terminal_putchar(c);
		terminal_movecursor();
#else
		std::putchar(c);
#endif
	}

	void terminal_writestring(const char* data)
	{
#ifndef VCPP
		size_t datalen = myos::strlen(data);
		for (size_t i = 0; i < datalen; i++)
			terminal_putchar(data[i]);

		terminal_movecursor();
#else
		std::cout << data;
#endif
	}

	void terminal_writenumber(uint32_t num, int base)
	{
		char buf[40];
		uitoa_n(num, buf, 40, base);
		terminal_writestring(buf);
	}

	void terminal_writebool(bool cond)
	{
		if (cond == true)
			terminal_writestring("Yes");
		else
			terminal_writestring("No");
	}

	void terminal_clear(size_t ul_x, size_t ul_y, size_t br_x, size_t br_y, uint8_t tcolor)
	{
		for (size_t y = ul_y; y <= br_y; y++)
		{
			for (size_t x = ul_x; x < br_x; x++)
			{
				const size_t index = y * VGA_WIDTH + x;
				terminal_buffer[index] = make_vgaentry(' ', tcolor);
			}
		}

		terminal_row = ul_y;
		terminal_column = ul_x;

		terminal_movecursor();
	}

	void terminal_clear(uint8_t tcolor)
	{
		terminal_clear(0, 0, VGA_WIDTH - 1, VGA_HEIGHT - 1, tcolor);
	}

	void terminal_scroll(size_t n)
	{

		int row = terminal_row;
		int col = terminal_column;

		if (n > 24)
		{
			terminal_clear();
		}
		else
		{
			uint16_t * first_pixel = terminal_buffer + n * VGA_WIDTH;
			size_t total_pixels = VGA_WIDTH * (VGA_HEIGHT - n);
			uint16_t * last_past_1_pixel = terminal_buffer + total_pixels;

			memcpy<uint16_t>(terminal_buffer, first_pixel, total_pixels);
			terminal_clear(scr_x(last_past_1_pixel), scr_y(last_past_1_pixel), VGA_WIDTH - 1, VGA_HEIGHT - 1);
		}


		terminal_row = row;
		terminal_column = col;

		terminal_movecursor();
	}


	void terminal_init()
	{
		terminal_row = 0;
		terminal_column = 0;
		terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
		terminal_buffer = (uint16_t*) 0xB8000;

		terminal_clear();
	}


	extern "C" void sys_putchar(char c)
	{
		terminal_putchar(c);
	}
}