#include "boot.hpp"
#include "../kernel.hpp"
#include "../console.hpp"
#include "../lib.hpp"
#include "../consoles/fb.hpp"

namespace Boot
{
	Parameters parameters;
	FramebufferConsoleBackend fb_console;
};

extern "C" void boot_entry(Boot::Parameters *parameters)
{
	Runtime::initialize();

	Boot::parameters = *parameters;

	Boot::fb_console.initialize((FramebufferConsoleBackend::color_t *)parameters->frame_buffer,
		parameters->frame_buffer_width,
		parameters->frame_buffer_height,
		parameters->frame_buffer_scanline,
		parameters->frame_buffer_size);

	console.initialize(&Boot::fb_console);
	
	kernel();
}
