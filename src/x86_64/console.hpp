#pragma once
#include "../common.hpp"

class Console
{
public:
	class Color
	{
		public:
			uint8_t value;
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
	
private:
	uint8_t x_offset;
	uint8_t y_offset;
	uint8_t color;
	const Color *hex_fg;
	
	void update_cursor();
	void scroll();
	void newline();
	
	void put_base(size_t value, size_t base);
	void put_base_padding(size_t value, size_t base, size_t min_size);
	
	static void do_panic();
	
	static const char digits[];

	static const unsigned max_chars;
	static const unsigned size_x;
	static const unsigned size_y;
	
	static const unsigned min_x;
	static const unsigned min_y;
	
	static const unsigned max_x;
	static const unsigned max_y;
	
	static uint16_t *const vga;
};

extern Console console;
