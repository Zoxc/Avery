#include "vga.hpp"
#include "../lib.hpp"
#include "../arch/common.hpp"

uint8_t VGAConsoleBackend::color_map[Console::ColorCount] = {
	7, // Default
	12, // Panic
	15, // Strong
	10 // Number
};

void VGAConsoleBackend::initialize()
{
	fb = (uint16_t *)0xb8000;

	clear();

	// Move the cursor outside the screen

	size_t loc = char_count;

	Arch::outb(0x3D4, 14);
	Arch::outb(0x3D5, loc >> 8);
	Arch::outb(0x3D4, 15);
	Arch::outb(0x3D5, loc);

	DisplayConsoleBackend::initialize(char_width - border * 2, char_height - border * 2);
}

void VGAConsoleBackend::scroll()
{
	for (size_t i = 0; i < char_width * (char_height - 1); i++)
		fb[i] = fb[i + 80];

	for (size_t x = 0; x < char_width; x++)
		*(fb + (char_height - 1) * char_width + x) = ' ' | (bg_color << 8);
}

void VGAConsoleBackend::clear()
{
	DisplayConsoleBackend::clear();

	for(auto pos = fb; pos < fb + char_count; pos++)
		*pos = ' ' | (bg_color << 8);
}

void VGAConsoleBackend::put_char(size_t x, size_t y, char c, Console::Color text)
{
	*(fb + (y + border) * char_width + x + border) = (uint8_t)c | (color_map[text] << 8);
}

void VGAConsoleBackend::get_buffer_info(addr_t &buffer, size_t &buffer_size)
{
	buffer = (addr_t)fb;
	buffer_size = char_count * sizeof(uint16_t);
}

void VGAConsoleBackend::new_buffer(void *buffer)
{
	fb = (uint16_t *)buffer;
}
