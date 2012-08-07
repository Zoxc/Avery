#pragma once
#include "display.hpp"

class VGAConsoleBackend:
	public DisplayConsoleBackend
{
public:
	typedef uint32_t color_t;

	void get_buffer_info [[override]](addr_t &buffer, size_t &buffer_size);
	void new_buffer [[override]](void *buffer);
	void put_char [[override]](size_t x, size_t y, char c, Console::Color text);
	void scroll [[override]]();
	void clear [[override]]();

	void initialize();

private:
	uint16_t *fb;

	static const uint8_t bg_color = 0;
	static const size_t border = 1;
	static const size_t char_width = 80;
	static const size_t char_height = 25;

	static const size_t char_count = char_width * char_height;

	static uint8_t color_map[Console::ColorCount];
};

extern Console console;

