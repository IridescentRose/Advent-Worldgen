#include "setup.hpp"
#include "gfx.hpp" 
#include "benchmark.hpp"
#include "worldgen.hpp"
#include <pspgu.h>

PSP_MODULE_INFO("Worldgen", 0, 1, 0);

uint32_t image[256 * 256];

union Color {
    uint32_t c;
    struct RGBA {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } rgba;
};

auto generate_image(float* map, float* biome) -> void {
    for(auto x = 0; x < 256; x++) {
        for(auto y = 0; y < 256; y++) {
            auto idx = x / 2 * 128 + y/2;
            float map_val = map[idx];
            float bio_val = biome[idx];

            Color genColor;

            if(map_val < 0.5f) {
                genColor.c = GU_COLOR(0.0f, 0.0f, map_val, 1.0f);
            } else if (map_val > 0.5f && map_val < 0.52f) {
                genColor.c = GU_COLOR(1.0f, map_val + 0.2f, 0.0f, 1.0f);
            } else if(map_val > 0.66f && map_val < 0.7f) {
                genColor.c = GU_COLOR(map_val, map_val, map_val, 1.0f);
            } else if (map_val > 0.7f) {
                genColor.c = GU_COLOR(map_val + 0.2f, map_val + 0.2f, map_val + 0.2f, 1.0f);
            } else {
                genColor.c = GU_COLOR(0.0f, 1.0f - map_val + 0.4f, 0.0f, 1.0f);
            }

            auto temp = static_cast<int16_t>(bio_val * 128.0f);
            if(temp > 255) {
                temp = 255;
            } else if (temp < 0) {
                temp = 0;
            }

            if(!(map_val > 0.5f && map_val < 0.52f)) {
                genColor.rgba.r = static_cast<uint8_t>(temp);
            }

            image[x * 256 + y] = genColor.c;
        }
    }
}

auto main() -> int {
	SetupCallbacks();
	
    GFX::init();
    GFX::clear(0xFF333333);

    Worldgen gen;

    BENCHMARK(gen.generate_map(), "Map Gen");

    generate_image(gen.get_map(), gen.get_biome_map());
    GFX::draw_img(image);

	while(true) {}
	return 0;
}