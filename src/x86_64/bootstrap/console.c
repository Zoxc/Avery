#include "common.h"

BOOTSTRAP_CODE

#include "console.h"
#include <stdarg.h>
#include "io.h"

#define MAX_SIZE 4000
#define SIZE_X 80
#define SIZE_Y 25

#define MIN_X 1
#define MIN_Y 1
#define MAX_X 79
#define MAX_Y 25

#define VM 0xb8000

static uint8_t x = MIN_X;
static uint8_t y = MIN_Y;
static uint8_t color = 15;

enum console_color console_get_fg(void)
{
	return color & 0x0F;
}

enum console_color console_get_bg(void)
{
	return (color & 0xF0) >> 4;
}

void console_fg(enum console_color fg)
{
	color = (fg & 0x0F) | (color & 0xF0);
}

void console_bg(enum console_color bg)
{
	color = (color & 0x0F) | (bg << 4);
}

static void console_update_cursor()
{
	uint16_t loc = y * 80 + x;
   
	outb(0x3D4, 14);
	outb(0x3D5, loc >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, loc);
}

static const char digits[] = "0123456789ABCDEF";

void console_put_base_padding(size_t value, size_t base, size_t min_size)
{
	size_t temp = value / base;
	
	if(min_size)
		console_put_base_padding(temp, base, min_size - 1);
	
	console_putc(digits[value % base]);
}

static void console_scroll(void)
{
	for (size_t i = 0; i < SIZE_Y * SIZE_X; i++)
	{
		((uint16_t *)VM)[i] = ((uint16_t *)VM)[i + 80];
	}
	
	for (size_t x = 0; x < SIZE_X; x++)
	{
		*((uint16_t *)VM + (SIZE_Y - 1) * 80 + x) = ' ' | (color << 8);
	}
}

static void console_newline(void)
{
	y++;
	x = MIN_X;
	
	if(y >= MAX_Y)
	{
		console_scroll();
		y = MAX_Y - 1;
	}
	
	console_update_cursor();
}

void console_putc(char c)
{
	switch((uint8_t)c)
	{
		case '\n':
			console_newline();
			break;
			
		case '\t':
			x = (x + 4) & ~(4 - 1);
			
			if(x >= MAX_X)
				console_newline();
			else
				console_update_cursor();
			break;
			
		default:
			if(x >= MAX_X)
				console_newline();
			
			*((uint16_t *)VM + y * 80 + x++) = (uint8_t)c | (color << 8);

			console_update_cursor();
			break;
	}
}

void console_puts(const char *str)
{
	if(!str)
		return;
	
	while(*str)
	{
		console_putc(*str);
		str++;
	}
}

void console_cls(void)
{
	for(uint16_t *pos = (uint16_t *)VM; pos < (uint16_t *)(VM + MAX_SIZE); pos++)
		*pos = ' ' | (color << 8);

	x = MIN_X;
	y = MIN_Y;
	console_update_cursor();
}
