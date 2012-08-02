#include "../params.hpp"
#include "../kernel.hpp"
#include "../console.hpp"
#include "../lib.hpp"
#include "../consoles/fb.hpp"

struct Parameters
{
	size_t size;
	void *frame_buffer;
	size_t frame_buffer_size;
	size_t frame_buffer_width;
	size_t frame_buffer_height;
	size_t frame_buffer_scanline;
	Params::Info info;
};

FramebufferConsoleBackend fb_console;

extern "C" void boot_entry(Parameters *parameters)
{
	Runtime::initialize();

	fb_console.initialize((FramebufferConsoleBackend::color_t *)parameters->frame_buffer,
		parameters->frame_buffer_width,
		parameters->frame_buffer_height,
		parameters->frame_buffer_scanline,
		parameters->frame_buffer_size);

	console.initialize(&fb_console);

	Params::info = parameters->info;

	kernel();
}
