#include "boot.hpp"
#include "../kernel.hpp"

namespace Boot
{
	Parameters *parameters;
};

extern "C" void entry(void *, void *, Boot::Parameters *parameters)
{
	Boot::parameters = parameters;
	
	kernel();
}
