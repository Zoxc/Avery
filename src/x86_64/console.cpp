#include "console.hpp"
#include "io.hpp"

Console console;

const unsigned Console::max_chars = 2000;

const unsigned Console::size_x = 80;
const unsigned Console::size_y = 25;
	
const unsigned Console::min_x = 1;
const unsigned Console::min_y = 1;
	
const unsigned Console::max_x = 79;
const unsigned Console::max_y = 25;

uint16_t *const Console::vga = (uint16_t *)0xb8000;

Console::Console() : x(min_x), y(min_y), color((uint8_t)LightGray | (uint8_t)Black << 4)
{
}

Console::Color Console::get_fg()
{
	return (Color)(color & 0x0F);
}

Console::Color Console::get_bg()
{
	return (Color)((color & 0xF0) >> 4);
}

Console &Console::fg(Color new_fg)
{
	color = ((uint8_t)new_fg & 0x0F) | (this->color & 0xF0);
	
	return *this;
}

Console &Console::bg(Color new_bg)
{
	color = (color & 0x0F) | ((uint8_t)new_bg << 4);
	
	return *this;
}

void Console::update_cursor()
{
	uint16_t loc = y * 80 + x;
	
	X86::outb(0x3D4, 14);
	X86::outb(0x3D5, loc >> 8);
	X86::outb(0x3D4, 15);
	X86::outb(0x3D5, loc);
}

void Console::scroll()
{
	for (size_t i = 0; i < size_y * size_x; i++)
	{
		vga[i] = vga[i + 80];
	}
	
	for (size_t x = 0; x < size_x; x++)
	{
		*(vga + (size_y - 1) * 80 + x) = ' ' | (color << 8);
	}
}

void Console::newline()
{
	y++;
	x = min_x;
	
	if(y >= max_y)
	{
		scroll();
		y = max_y - 1;
	}
	
	update_cursor();
}

Console &Console::putc(const char c)
{
	switch((uint8_t)c)
	{
		case '\n':
			newline();
			break;
			
		case '\t':
			x = (x + 4) & ~(4 - 1);
			
			if(x >= max_x)
				newline();
			else
				update_cursor();
			break;
			
		default:
			if(x >= max_x)
				newline();
			
			*(vga + y * 80 + x++) = (uint8_t)c | (color << 8);

			update_cursor();
			break;
	}
	
	return *this;
}

Console &Console::puts(const char *str)
{
	if(!str)
		return *this;
	
	while(*str)
	{
		putc(*str);
		str++;
	}
	
	return *this;
}

Console & Console::clear(void)
{
	for(uint16_t *pos = vga; pos < vga + max_chars; pos++)
		*pos = ' ' | (color << 8);

	x = min_x;
	y = min_y;
	update_cursor();
	
	return *this;
}
