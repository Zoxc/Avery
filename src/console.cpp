#include "console.hpp"
#include "lib.hpp"
#include "arch.hpp"

Console console;

void (*Console::flush_line)() = 0;

Console::Console() : text_color(Default), hex_fg(Value)
{
}

void Console::initialize(ConsoleBackend *backend)
{
	this->backend = backend;
}

void Console::get_buffer_info(void *&buffer, size_t &buffer_size)
{
	backend->get_buffer_info(buffer, buffer_size);
}

void Console::new_buffer(void *buffer)
{
	backend->new_buffer(buffer);
}

void Console::do_panic()
{
	Arch::panic();
}

Console &Console::Console::panic()
{
	flush_line = do_panic;
	newline(); newline();
	
	color(Panic).s("Panic").color(Strong).s(": ");
	
	return *this;
}

Console &Console::endl()
{
	newline();
	
	if(flush_line != 0)
		flush_line();
	
	return *this;
}

Console &Console::color(Color new_color)
{
	text_color = new_color;

	backend->color(new_color);
	
	return *this;
}

Console &Console::lb()
{
	newline();
	
	return *this;
}

Console &Console::a(unsigned long count)
{
	c(' ');
	
	while(x_offset % count)
		c(' ');
	
	return *this;
}

Console &Console::w(unsigned long count)
{
	while(count--)
		c(' ');
	
	return *this;
}

Console &Console::u(const unsigned long value)
{
	put_base(value, 10);
	
	return *this;
	
}

Console &Console::x(const unsigned long value)
{
	auto temp = text_color;

	color(hex_fg);
	
	c('0').c('x');
	
	put_base_padding(value, 16, sizeof(value) * 2);
	
	color(temp);
	
	return *this;
}

Console &Console::x(const void *value)
{
	x((unsigned long)value);
	
	return *this;
}

const char Console::digits[] = "0123456789abcdef";

void Console::put_base(size_t value, size_t base)
{
	size_t temp = value / base;
	
	if(temp)
		put_base(temp, base);
	
	c(digits[value % base]);
}

void Console::put_base_padding(size_t value, size_t base, size_t min_size)
{
	min_size--;

	size_t temp = value / base;
	
	if(min_size || temp)
		put_base_padding(temp, base, min_size);
	
	c(digits[value % base]);
}

void Console::newline()
{
	backend->print('\n');
}

Console &Console::c(const char c)
{
	backend->print(c);
	
	return *this;
}

Console &Console::s(const char *str)
{
	if(!str)
		return *this;
	
	while(*str)
	{
		c(*str);
		str++;
	}
	
	return *this;
}

Console &Console::clear(void)
{
	backend->clear();

	return *this;
}
