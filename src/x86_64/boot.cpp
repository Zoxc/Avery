#include "boot.hpp"
#include "../kernel.hpp"

namespace Boot
{
	Parameters parameters;
};

extern "C" void boot_entry(Boot::Parameters *parameters)
{
	Boot::parameters = *parameters;
	
	kernel();
}
