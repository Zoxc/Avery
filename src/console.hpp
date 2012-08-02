#pragma once
#include "common.hpp"

class ConsoleBackend;

class Console
{
public:
	typedef uint32_t color_t;

	enum Color
	{
		Default,
		Panic,
		Strong,
		Number,
		ColorCount
	};

	Console();
	
	Console &clear();
	
	Console &panic();

	static void (*flush_line)();
	
	Console &color(Color new_color);
	
	Console &u(const unsigned long value);
	
	Console &x(const unsigned long value);
	Console &x(const void *value);
	
	Console &lb();
	Console &endl();
	
	Console &w(unsigned long count = 1);
	Console &a(unsigned long count = 8);
	
	Console &c(const char c);
	Console &s(const char *str);

	void initialize(ConsoleBackend *backend);

	void get_buffer_info(void *&buffer, size_t &buffer_size);
	void new_buffer(void *buffer);
	
private:
	ConsoleBackend *backend;

	size_t x_offset;
	size_t y_offset;

	size_t width;
	size_t height;

	Color text_color;
	Color hex_fg;

	void update_cursor();
	void scroll();
	void newline();
	
	void put_base(size_t value, size_t base);
	void put_base_padding(size_t value, size_t base, size_t min_size);
	
	static void do_panic();
	
	static const char digits[];
};

class ConsoleBackend
{
public:
	virtual void color(Console::Color text) = 0;
	virtual void print(char c) = 0;
	virtual void clear() = 0;
	virtual void get_buffer_info(void *&buffer, size_t &buffer_size) = 0;
	virtual void new_buffer(void *buffer) = 0;
};

extern Console console;

