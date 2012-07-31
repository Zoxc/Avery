#pragma once
#include "common.hpp"

class Console
{
public:
	typedef uint32_t color_t;
	
	class Color
	{
		public:
			color_t value;
	};
	
	static const Color black;
	static const Color blue;
	static const Color green;
	static const Color cyan;
	static const Color red;
	static const Color magenta;
	static const Color brown;
	static const Color light_gray;
	static const Color dark_gray;
	static const Color light_blue;
	static const Color light_green;
	static const Color light_cyan;
	static const Color light_red;
	static const Color light_magenta;
	static const Color yellow;
	static const Color white;
	
	Console();
	
	Console &clear();
	
	Console &panic();

	static void (*flush_line)();
	
	Console &fg(const Color &new_fg);
	Console &bg(const Color &new_bg);
	
	Console &u(const unsigned long value);
	
	Console &x(const unsigned long value);
	Console &x(const void *value);
	
	Console &lb();
	Console &endl();
	
	Console &w(unsigned long count = 1);
	Console &a(unsigned long count = 8);
	
	Console &c(const char c);
	Console &s(const char *str);

	void initialize();
	void update_frame_buffer();
	
private:
	const size_t font_scanline = 2304;
	const size_t font_width = 9;
	const size_t font_height = 16;
	const size_t font_height_pad = 19;
	
	size_t left;
	size_t top;
	size_t width;
	size_t height;
	size_t x_offset;
	size_t y_offset;
	size_t min_x;
	size_t min_y;
	size_t max_x;
	size_t max_y;
	color_t *frame;
	size_t scanline;
	color_t fg_color;
	color_t bg_color;
	const Color *hex_fg;
	
	void blit_char(size_t x, size_t y, uint8_t index, color_t color);
	void clear_frame(size_t x, size_t y, size_t width, size_t height);
	
	void update_cursor();
	void scroll();
	void newline();
	
	void put_base(size_t value, size_t base);
	void put_base_padding(size_t value, size_t base, size_t min_size);
	
	static void do_panic();
	
	static const char digits[];
};

extern Console console;

