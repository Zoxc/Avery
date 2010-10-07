#pragma once

enum console_color
{
	console_black,
	console_blue,
	console_green,
	console_cyan,
	console_red,
	console_magenta,
	console_brown,
	console_light_gray,
	console_dark_gray,
	console_light_blue,
	console_light_green,
	console_light_cyan,
	console_light_red,
	console_light_magenta,
	console_yellow,
	console_white
};

void console_cls(void);

enum console_color console_get_fg(void);
enum console_color console_get_bg(void);
void console_fg(enum console_color fg);
void console_bg(enum console_color bg);

void console_put_base_padding(size_t value, size_t base, size_t min_size);

void console_putc(char c);
void console_puts(const char *str);

void kprint(const char * format, ...);
