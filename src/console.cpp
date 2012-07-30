#include "console.hpp"
#include "x86_64/boot.hpp"
#include "lib.hpp"

Console console;

const Console::Color Console::black = {0};
const Console::Color Console::blue = {0};
const Console::Color Console::green = {2};
const Console::Color Console::cyan = {3};
const Console::Color Console::red = {4};
const Console::Color Console::magenta = {5};
const Console::Color Console::brown = {6};
const Console::Color Console::light_gray = {0x5c5c5c};
const Console::Color Console::dark_gray = {8};
const Console::Color Console::light_blue = {9};
const Console::Color Console::light_green = {0x364627};
const Console::Color Console::light_cyan = {11};
const Console::Color Console::light_red = {0x96492A};
const Console::Color Console::light_magenta = {13};
const Console::Color Console::yellow = {14};
const Console::Color Console::white = {0x373737};

void (*Console::flush_line)() = 0;

Console::Console() : fg_color(0x49535C), bg_color(0xE6EAE3), hex_fg(&light_green)
{
	
}

void Console::update_frame_buffer()
{
	frame = (color_t *)Boot::parameters.frame_buffer;
}

void Console::initialize()
{
	auto &params = Boot::parameters;
	
	frame = (color_t *)params.frame_buffer;
	scanline = params.frame_buffer_scanline;
	
	size_t border = 25;
	
	min_x = 0;
	min_y = 0;
	
	max_x = params.frame_buffer_width - border * 2;
	max_x = max_x / font_width;
	width = max_x * font_width;
	max_x--;
	
	left = (params.frame_buffer_width - width) / 2;
	
	max_y = params.frame_buffer_height - border * 2;
	max_y = max_y / font_height_pad;
	height = max_y * font_height_pad;
	max_y--;
	
	x_offset = min_x;
	y_offset = min_y; 
	
	top = (params.frame_buffer_width - width) / 2;
	
	clear();
}

void Console::do_panic()
{
	Arch::panic();
}

Console &Console::Console::panic()
{
	flush_line = do_panic;
	newline(); newline();
	
	fg(Console::light_red).s("Panic").fg(Console::white).s(": ");
	
	return *this;
}

Console &Console::endl()
{
	newline();
	
	if(flush_line != 0)
		flush_line();
	
	return *this;
}

Console &Console::fg(const Color &new_fg)
{
	fg_color = new_fg.value;
	
	return *this;
}

Console &Console::bg(const Color &new_bg)
{
	bg_color = new_bg.value;
	
	return *this;
}

Console &Console::lb()
{
	newline();
	
	return *this;
}

Console &Console::a(unsigned long count)
{
	c(' ');
	
	while((x_offset - min_x) % count)
		c(' ');
	
	return *this;
}

Console &Console::w(unsigned long count)
{
	while(count--)
		c(' ');
	
	return *this;
}

Console &Console::u(const unsigned long value)
{
	put_base(value, 10);
	
	return *this;
	
}

Console &Console::x(const unsigned long value)
{
	color_t temp = fg_color;
	
	if(hex_fg)
		fg(*hex_fg);
	
	c('0').c('x');
	
	put_base_padding(value, 16, sizeof(value) * 2);
	
	fg_color = temp;
	
	return *this;
}

Console &Console::x(const void *value)
{
	x((unsigned long)value);
	
	return *this;
}

const char Console::digits[] = "0123456789abcdef";

void Console::put_base(size_t value, size_t base)
{
	size_t temp = value / base;
	
	if(temp)
		put_base(temp, base);
	
	c(digits[value % base]);
}

void Console::put_base_padding(size_t value, size_t base, size_t min_size)
{
	min_size--;

	size_t temp = value / base;
	
	if(min_size || temp)
		put_base_padding(temp, base, min_size);
	
	c(digits[value % base]);
}

void Console::update_cursor()
{
	return;
}

extern "C" void *raw_bitmap_font;

void Console::blit_char(size_t x, size_t y, uint8_t index, color_t color)
{
	uint8_t *row = ((uint8_t *)&raw_bitmap_font) + index * font_width;
	uint8_t *row_stop = row + font_scanline * font_height;
	color_t *pixel_row = frame + x + scanline * y;
	
	for(; row < row_stop; row += font_scanline, pixel_row += scanline)
	{
		uint8_t *bit = row;
		color_t *pixel_stop = pixel_row + font_width;
		
		for(color_t *pixel = pixel_row; pixel < pixel_stop; ++bit, ++pixel)
			if(*bit)
				*pixel = color;
	}
}

void Console::clear_frame(size_t x, size_t y, size_t width, size_t height)
{
	auto row = frame + scanline * y + x;
	auto row_stop = row + scanline * height;
	
	for(; row < row_stop; row += scanline)
	{
		auto pixel_stop = row + width;
		
		for(auto pixel = row; pixel < pixel_stop; ++pixel)
			*pixel = bg_color;
	}
}

void Console::scroll()
{
	auto row = frame + scanline * top + left;
	auto row_stop = row + scanline * (height - font_height_pad);
	
	for(; row < row_stop; row += scanline)
		memcpy(row, row + scanline * font_height_pad, width * sizeof(color_t));

	clear_frame(left, top + height - font_height_pad, width, font_height_pad);
}

void Console::newline()
{
	y_offset++;
	x_offset = min_x;
	
	if(y_offset >= max_y)
	{
		scroll();
		y_offset = max_y - 1;
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
			x_offset = (x_offset + 4) & ~(4 - 1);
			
			if(x_offset >= max_x)
				newline();
			else
				update_cursor();
			break;
			
		default:
			if(x_offset >= max_x)
				newline();
			
			blit_char(left + x_offset++ * font_width, top + y_offset * font_height_pad, c, fg_color);
			
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

Console &Console::clear(void)
{
	clear_frame(0, 0, Boot::parameters.frame_buffer_width, Boot::parameters.frame_buffer_height);
	
	x_offset = min_x;
	y_offset = min_y;
	update_cursor();
	
	return *this;
}
