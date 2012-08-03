#pragma once
#include "display.hpp"

class FramebufferConsoleBackend:
	public DisplayConsoleBackend
{
public:
	typedef uint32_t color_t;

	void get_buffer_info [[override]](void *&buffer, size_t &buffer_size);
	void new_buffer [[override]](void *buffer);
	void put_char [[override]](size_t x, size_t y, char c, Console::Color text);
	void scroll [[override]]();
	void clear [[override]]();

	void initialize(color_t *fb, size_t fb_width, size_t fb_height, size_t fb_scanline, size_t fb_size);

private:
	static color_t color_map[Console::ColorCount];

	const color_t background = 0xE6EAE3;

	const size_t font_scanline = 2304;
	const size_t font_width = 9;
	const size_t font_height = 16;
	const size_t font_height_pad = 19;
	
	size_t left;
	size_t top;
	size_t width;
	size_t height;

	size_t char_width;
	size_t char_height;

	color_t *fb;
	size_t fb_size;

	size_t scanline;

	void blit_char(size_t x, size_t y, uint8_t index, color_t color);
	void clear_frame(size_t x, size_t y, size_t width, size_t height);
};
