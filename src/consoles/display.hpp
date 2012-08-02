#pragma once
#include "../console.hpp"

class DisplayConsoleBackend:
	public ConsoleBackend
{
public:
	void initialize(size_t width, size_t height);

	virtual void color [[override]](Console::Color text);
	virtual void print [[override]](char c);
	virtual void clear [[override]]();

	virtual void put_char(size_t x, size_t y, char c, Console::Color text) = 0;
	virtual void scroll() = 0;
private:
	Console::Color text_color;

	size_t x_offset;
	size_t y_offset;

	size_t width;
	size_t height;

	void newline();
};

