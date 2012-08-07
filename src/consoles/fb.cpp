#include "fb.hpp"
#include "../lib.hpp"

FramebufferConsoleBackend::color_t FramebufferConsoleBackend::color_map[Console::ColorCount] = {
	0x49535C, // Default
	0x96492A, // Panic
	0x202020, // Strong
	0x364627 // Number
};

void FramebufferConsoleBackend::initialize(color_t *fb, size_t fb_width, size_t fb_height, size_t fb_scanline, size_t fb_size)
{
	this->fb = fb;
	this->fb_size = fb_size;
	scanline = fb_scanline;
	
	size_t border = 25;

	char_width = fb_width - border * 2;
	char_width = char_width / font_width;
	width = char_width * font_width;
	
	left = (fb_width - width) / 2;
	
	char_height = fb_height - border * 2;
	char_height = char_height / font_height_pad;
	height = char_height * font_height_pad;

	top = (fb_height - height) / 2;

	clear_frame(0, 0, fb_width, fb_height);

	DisplayConsoleBackend::initialize(char_width, char_height);
}

extern "C" void *raw_bitmap_font;

void FramebufferConsoleBackend::blit_char(size_t x, size_t y, uint8_t index, color_t color)
{
	uint8_t *row = ((uint8_t *)&raw_bitmap_font) + index * font_width;
	uint8_t *row_stop = row + font_scanline * font_height;
	color_t *pixel_row = fb + x + scanline * y;
	
	for(; row < row_stop; row += font_scanline, pixel_row += scanline)
	{
		uint8_t *bit = row;
		color_t *pixel_stop = pixel_row + font_width;
		
		for(color_t *pixel = pixel_row; pixel < pixel_stop; ++bit, ++pixel)
			if(*bit)
				*pixel = color;
	}
}

void FramebufferConsoleBackend::clear_frame(size_t x, size_t y, size_t width, size_t height)
{
	auto row = fb + scanline * y + x;
	auto row_stop = row + scanline * height;
	
	for(; row < row_stop; row += scanline)
	{
		auto pixel_stop = row + width;
		
		for(auto pixel = row; pixel < pixel_stop; ++pixel)
			*pixel = background;
	}
}

void FramebufferConsoleBackend::scroll()
{
	auto row = fb + scanline * top + left;
	auto row_stop = row + scanline * (height - font_height_pad);
	
	for(; row < row_stop; row += scanline)
		memcpy(row, row + scanline * font_height_pad, width * sizeof(color_t));

	clear_frame(left, top + height - font_height_pad, width, font_height_pad);
}

void FramebufferConsoleBackend::clear()
{
	clear_frame(left, top, width, height);
}

void FramebufferConsoleBackend::put_char(size_t x, size_t y, char c, Console::Color text)
{
	blit_char(left + x * font_width, top + y * font_height_pad, c, color_map[text]);
}

void FramebufferConsoleBackend::get_buffer_info(addr_t &buffer, size_t &buffer_size)
{
	buffer = (addr_t)fb;
	buffer_size = fb_size;
}

void FramebufferConsoleBackend::new_buffer(void *buffer)
{
	fb = (color_t *)buffer;
}
