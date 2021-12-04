#include "setup.hpp"
#include "gfx.hpp" 
#include "benchmark.hpp"
#include "worldgen.hpp"
#include <pspgu.h>

PSP_MODULE_INFO("Worldgen", 0, 1, 0);

uint32_t image[256 * 256];
auto generate_image(float* map) -> void {
    for(auto x = 0; x < 256; x++) {
        for(auto y = 0; y < 256; y++) {
            auto idx = x / 2 * 128 + y/2;
            float map_val = map[idx];

            uint32_t genColor;

            if(map_val < 0.5f) {
                genColor = GU_COLOR(0.0f, 0.0f, map_val, 1.0f);
            } else if (map_val > 0.5f && map_val < 0.52f) {
                genColor = GU_COLOR(1.0f, map_val + 0.2f, 0.0f, 1.0f);
            } else if(map_val > 0.66f && map_val < 0.7f) {
                genColor = GU_COLOR(map_val, map_val, map_val, 1.0f);
            } else if (map_val > 0.7f) {
                genColor = GU_COLOR(map_val + 0.2f, map_val + 0.2f, map_val + 0.2f, 1.0f);
            } else {
                genColor = GU_COLOR(0.0f, 1.0f - map_val + 0.4f, 0.0f, 1.0f);
            }

            image[x * 256 + y] = genColor;

        }
    }
}

auto main() -> int {
	SetupCallbacks();
	
    GFX::init();
    GFX::clear(0xFF333333);

    Worldgen gen;

    BENCHMARK(gen.generate_map(), "Map Gen");

    generate_image(gen.get_map());
    GFX::draw_img(image);

	while(true) {}
	return 0;
}