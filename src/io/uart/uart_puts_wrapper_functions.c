#include "io/uart.h"

#include "lib/string.h"
#include "lib/alloc.h"
#include "lib/math.h"

#ifndef __INTELLISENSE__ // To make VSCODE shut up
// #ifdef __GNUC__
// #define alloca(x) __builtin_alloca((x))
// #else
// #error failed to wind alloca
// #endif

#endif


// #include <malloc.h>

void uart_puti(int integer)
{
	if (integer < 0)
	{
	 	integer = -integer;
		uart_putc('-');
	}

	uart_putui((unsigned int)integer);
}

void uart_putf(float number, uint8_t decimals)
{
	if (number < 0)
	{
	 	number = -number;
		uart_putc('-');
	}

	int digits = decimals == 0 ? 0 : (decimals + 1);
    for (unsigned int x = number == 0 ? 1 : number
		; x != 0;
		 x /=10, digits++) {}

	char* buffer = alloca(digits + 1);
	buffer[digits] = '\0';

	number *= pow(10, (unsigned int)decimals);

	for (int i = digits - 1; i >= 0 ; i--)
	{
		int remander = (int)number % 10;
		number /= 10;
		buffer[i] = 0x30 + remander;


		if (decimals != 0 && decimals != UINT8_MAX)
			decimals--;
		
		if (decimals == 0)
		{
			buffer[--i] = '.';
			decimals--;
		}

		if (number < 0)
			break;
	}

	uart_puts(buffer);
}

void uart_putui(unsigned int integer)
{
	int decimals = 0;
    for (unsigned int x = integer == 0 ? 1 : integer
		; x != 0;
		 x /=10, decimals++) {}

	char* buffer = alloca(decimals + 1);
	buffer [decimals] = '\0';

	for (int i = decimals - 1; i >= 0 ; i--)
	{
		int remander = integer % 10;
		integer /= 10;
		buffer[i] = 0x30 + remander;

		if (integer == 0)
			break;
	}

	uart_puts(buffer);
}

void uart_puts(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; i ++)
		uart_putc((unsigned char)str[i]);
}

void uart_put_hexdigit(uint8_t digit)
{
	if (digit > 9)
		digit += 0x7;
	digit += 0x30;

	uart_putc(digit);
}

void uart_puts_reversed(const char* str)
{
	size_t length = strlen(str);

	for (size_t i = length - 1; i != SIZE_MAX; i --)
		uart_putc((unsigned char)str[i]);
}

void uart_put_buffer(const void* buffer, size_t length)
{
	const unsigned char* ptr = (unsigned char*)buffer;

	for (size_t i = 0; i < length; i ++)
		uart_putc((unsigned char)ptr[i]);
}

void uart_put_memory_dump_formated(void* ptr, size_t size)
{	
	for (int i = sizeof(size_t) * (4 * 2) - 4; i >= 0; i -= 4)
		uart_putc(' ');
	uart_puts(" 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	size_t ptr_as_number = *(size_t*)&ptr;
	size_t ptr_alligned_16 = (ptr_as_number << 4) >> 4;
	size_t ptr_offset = ptr_as_number - ptr_alligned_16;
	if (ptr_offset != 0)
	{
		uart_putc('\n');
		for (int i = sizeof(size_t) * (4 * 2) - 4; i >= 0; i -= 4)
			uart_put_hexdigit((ptr_alligned_16 >> i) & 0xF);

		for (int i = 0; i < 1 + ptr_offset * 3; i++)
			uart_putc(' ');
	}

	for ( ; size > 0; size--, ptr++)
	{
		ptr_as_number = *(size_t*)&ptr;
		if ((ptr_as_number & 0xF) == 0)
		{
			uart_putc('\n');
			ptr_alligned_16 = (ptr_as_number << 4) >> 4;
			for (int i = sizeof(size_t) * (4 * 2) - 4; i >= 0; i -= 4)
				uart_put_hexdigit((ptr_alligned_16 >> i) & 0xF);
			uart_putc(' ');
		}

		uart_put_hexdigit(*((uint8_t*)ptr) >> 4);
		uart_put_hexdigit(*((uint8_t*)ptr) & 0xF);
		uart_putc(' ');
	}
	
}

void uart_put_number_as_hex(size_t number)
{
	uart_putc('0');
	uart_putc('x');

	for (int i = sizeof(number) * (4 * 2) - 4; i >= 0; i -= 4)
		uart_put_hexdigit((number >> i) & 0xF);
}

void uart_put_number_as_hex_without_leading_zeros(size_t number)
{
	uart_putc('0');
	uart_putc('x');

	int startPossition = 0;

	for (int i = 0; i < sizeof(number) * 8; i += 4)
	{
		if (number & (0xF << i))
		{
			startPossition = i;
		}
	}


	for (int i = startPossition; i >= 0; i -= 4)
		uart_put_hexdigit((number >> i) & 0xF);
}