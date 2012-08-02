#include "display.hpp"

void DisplayConsoleBackend::initialize(size_t width, size_t height)
{
	this->width = width;
	this->height = height;

	text_color = Console::Default;

}

void DisplayConsoleBackend::newline()
{
	y_offset++;
	x_offset = 0;

	if(y_offset >= height)
	{
		scroll();
		y_offset = height - 1;
	}
}

void DisplayConsoleBackend::color(Console::Color text)
{
	text_color = text;
}

void DisplayConsoleBackend::print(const char c)
{
	switch((uint8_t)c)
	{
		case '\n':
			newline();
			break;

		case '\t':
			x_offset = (x_offset + 4) & ~(4 - 1);
			
			if(x_offset >= width)
				newline();
			break;
			
		default:
			if(x_offset >= width)
				newline();
			
			put_char(x_offset++, y_offset, c, text_color);
			break;
	}
}

void DisplayConsoleBackend::clear()
{
	x_offset = 0;
	y_offset = 0;
}
