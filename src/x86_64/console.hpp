#pragma once
#include "../common.hpp"

class Console
{
public:
	enum Color
	{
		Black,
		Blue,
		Green,
		Cyan,
		Red,
		Magenta,
		Brown,
		LightGray,
		DarkGray,
		LightBlue,
		LightGreen,
		LightCyan,
		LightRed,
		LightMagenta,
		Yellow,
		White
	};
	
	Console();
	
	Console &clear();
	
	Console &fg(Color new_fg);
	Console &bg(Color new_bg);
	
	Color get_fg();
	Color get_bg();
	
	Console &putc(const char c);
	Console &puts(const char *str);
	
private:
	uint8_t x;
	uint8_t y;
	uint8_t color;
	
	void update_cursor();
	void scroll();
	void newline();

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
