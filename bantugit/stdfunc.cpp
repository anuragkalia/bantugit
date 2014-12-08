#include "stdfunc.h"
#include "terminal.h"

namespace myos
{
	size_t strlen(const char* str)
	{
		size_t ret = 0;
		while (str[ret] != 0)
			ret++;
		return ret;
	}

	bool uitoa_n(uint32_t in_i, char * const out_s, const uint8_t n, uint8_t base)
	{
		if (base < 2 || base > 36)
		{
			return false;
		}
		else
		{
			//terminal_writestring("In uitoa_n\n");
			char buf[sizeof(in_i) * 8  + 1]; //cannot exceed 32 digits

			char * const beg = buf;
			char *cur = beg;

			do
			{
				*cur++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + in_i % base];
				//terminal_putchar(*(cur - 1));
				//terminal_putchar(' ');
				
				in_i /= base;
			} while (in_i);

			int sz = cur - beg + 1;

			if (sz > n)
			{
				return false;
			}
			else
			{
				
				char *out_cur = out_s, *buf_cur = --cur, *before_buf = beg - 1;

				while (buf_cur > before_buf)
				{
					*out_cur++ = *buf_cur--;
				}
				*out_cur = '\0';
				//terminal_writestring(buf);
				//terminal_writestring("\nOut: uitoa_n\n");
				return true;
			}
		}
	}

	unsigned int strncpy(char *dest, int dest_sz, const char *src, char delim)
	{
		int i;
		for (i = 0; i < dest_sz - 1 && src[i] != delim; i++)
		{
			dest[i] = src[i];
		}
		dest[i] = '\0';
		return i;
	}

	int strcmp(const char *str1, const char *str2)
	{
		int i = 0;

		while (str1[i] == str2[i] && str1[i] != '\0')
		{
			i++;
		}

		if (str1[i] == str2[i])
			return 0;
		else if (str1[i] < str2[i])
			return -1;
		else
			return 1;
	}

	int stricmp(const char *str1, const char *str2)
	{
		int i = 0;
		char c1, c2;


		while (str1[i] != '\0' && str2[i] != '\0')
		{
			c1 = tolower(str1[i]);
			c2 = tolower(str2[i]);

			if (c1 == c2)
			{
				i++;
			}
			else if (c1 < c2)
			{
				return -1;
			}
			else
			{
				return 1;
			}
		}

		if (str1[i] == str2[i]) //== 0
		{
			return 0;
		}
		else if (str1[i] == 0)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	char tolower(char c) //assumption: 'a-z' and 'A-Z' are contiguous in charmap
	{
		int diff = c - 'A';
		if (diff >= 0 && diff < 26)
		{
			return 'a' + diff;
		}
		else
		{
			return c;
		}
	}
	char toupper(char c)
	{
		int diff = c - 'a';
		if (diff >= 0 && diff < 26)
		{
			return 'A' + diff;
		}
		else
		{
			return c;
		}
	}
	
	bool islower(char c)
	{
		if (c >= 'a' && c <= 'z')
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool isupper(char c)
	{
		if (c >= 'A' && c <= 'Z')
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool isalpha(char c)
	{
		return islower(c) || isupper(c);
	}

	bool isdigit(char c)
	{
		if (c >= '0' && c <= '9')
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool isalnum(char c)
	{
		return isalpha(c) || isdigit(c);
	}

	bool isspace(char c)
	{
		switch (c)
		{
		case ' ':
		case '\n':
		case '\t':
		case '\r':
			return true;
		default:
			return false;
		}
	}

}