#include "setup.hpp"
#include "gfx.hpp" 

PSP_MODULE_INFO("Worldgen", 0, 1, 0);

auto main() -> int {
	SetupCallbacks();
	
    GFX::init();

    GFX::clear(0xFF0000FF);

	while(true) {}
	return 0;
}