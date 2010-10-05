#include "console.hpp"
#include "arch.hpp"
#include "io.hpp"

Console console;

const Console::Color Console::black = {0};
const Console::Color Console::blue = {1};
const Console::Color Console::green = {2};
const Console::Color Console::cyan = {3};
const Console::Color Console::red = {4};
const Console::Color Console::magenta = {5};
const Console::Color Console::brown = {6};
const Console::Color Console::light_gray = {7};
const Console::Color Console::dark_gray = {8};
const Console::Color Console::light_blue = {9};
const Console::Color Console::light_green = {10};
const Console::Color Console::light_cyan = {11};
const Console::Color Console::light_red = {12};
const Console::Color Console::light_magenta = {13};
const Console::Color Console::yellow = {14};
const Console::Color Console::white = {15};

const unsigned Console::max_chars = 2000;

const unsigned Console::size_x = 80;
const unsigned Console::size_y = 25;
	
const unsigned Console::min_x = 1;
const unsigned Console::min_y = 1;
	
const unsigned Console::max_x = 79;
const unsigned Console::max_y = 25;

decltype(Console::endl) Console::endl;

void (*Console::flush_line)() = 0;

uint16_t *const Console::vga = (uint16_t *)0xb8000;

Console::Console() : x(min_x), y(min_y)
{
	fg(light_gray).bg(black);
}

void Console::do_panic()
{
	Arch::panic();
}

Console &Console::Console::panic()
{
	flush_line = do_panic;
	newline(); newline();
	
	*this << Console::light_red << "Panic" << Console::white << ": ";
	
	return *this;
}

Console &Console::operator <<(const char *str)
{
	return s(str);
}

Console &Console::operator <<(decltype(endl) &)
{
	newline();
	
	if(flush_line != 0)
		flush_line();
		
	return *this;
}

Console &Console::operator <<(const Color &new_fg)
{
	fg(new_fg);
	
	return *this;
}

Console &Console::operator <<(const unsigned long value)
{
	put_base(value, 10);
	
	return *this;
}

Console &Console::fg(const Color &new_fg)
{
	color = new_fg.value | (color & 0xF0);
	
	return *this;
}

Console &Console::bg(const Color &new_bg)
{
	color = (color & 0x0F) | new_bg.value << 4;
	
	return *this;
}

Console &Console::lb()
{
	newline();
	
	return *this;
}

Console &Console::w(unsigned long count)
{
	while(count--)
		c(' ');
	
	return *this;
}

Console &Console::hex(const unsigned long value)
{
	s("0x");
	put_base_padding(value, 16, sizeof(value));
	
	return *this;
}

const char Console::digits[] = "0123456789ABCDEF";

void Console::put_base(size_t value, size_t base)
{
	size_t temp = value / base;
	
	if(temp)
		put_base(temp, base);
	
	c(digits[value % base]);
}

void Console::put_base_padding(size_t value, size_t base, size_t min_size)
{
	size_t temp = value / base;
	
	if(min_size)
		put_base_padding(temp, base, min_size - 1);
	
	c(digits[value % base]);
}

void Console::update_cursor()
{
	uint16_t loc = y * 80 + x;
	
	Arch::outb(0x3D4, 14);
	Arch::outb(0x3D5, loc >> 8);
	Arch::outb(0x3D4, 15);
	Arch::outb(0x3D5, loc);
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

Console &Console::c(const char c)
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

Console &Console::s(const char *str)
{
	if(!str)
		return *this;
	
	while(*str)
	{
		c(*str);
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
