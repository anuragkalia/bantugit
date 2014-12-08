#ifndef __STDFUNC_H
#define __STDFUNC_H

#include <stdint.h>
#include <stddef.h>

namespace myos
{
	/*In stdfunc.cpp*/
	size_t strlen(const char* str);

	template <class T>
	void memcpy(T * des, const T * src, const size_t n)
	{
		for (size_t i = 0; i < n; i++)
		{
			*des++ = *src++;
		}
	}

	template <class T>
	void memset(T * des, const T val, const size_t n)
	{
		for (size_t i = 0; i < n; i++)
		{
			*des++ = val;
		}
	}

	bool uitoa_n(uint32_t in_i, char * const out_s, const uint8_t n, const uint8_t base);

	unsigned int strncpy(char *dest, int dest_sz, const char *src, char delim = '\0');

	int strcmp(const char *str1, const char *str2);
	int stricmp(const char *str1, const char *str2);
	
	char tolower(char c);
	char toupper(char c);
	bool islower(char c);
	bool isupper(char c);
	bool isalpha(char c);
	bool isdigit(char c);
	bool isalnum(char c);
	bool isspace(char c);
	/*boot.s*/
	extern "C" void memsetinternalzero(unsigned int des, unsigned short n);
}

#endif //__STDFUNC_H