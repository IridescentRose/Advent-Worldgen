#include "setup.hpp"
#include "gfx.hpp" 
#include "benchmark.hpp"
#include "worldgen.hpp"
#include <pspgu.h>
#include <pspctrl.h>
#include <psppower.h>
#include "me.h"
#include <stdio.h>

PSP_MODULE_INFO("Worldgen", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU | THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-1024);

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

auto get_color(uint8_t b) -> uint32_t {
    switch(static_cast<BiomeType>(b)){
        case BIOME_OCEAN: return GU_RGBA(0, 0, 255, 255);
        case BIOME_BEACH: return GU_RGBA(255, 255, 0, 255);
        case BIOME_RIVER: return GU_RGBA(0, 128, 255, 255);
        case BIOME_DESERT: return GU_RGBA(255, 192, 0, 255);
        case BIOME_PLAINS: return GU_RGBA(128, 255, 128, 255);
        case BIOME_FOREST: return GU_RGBA(96, 192, 96, 255);
        case BIOME_RAINFOREST: return GU_RGBA(96, 255, 96, 255);
        case BIOME_TUNDRA: return GU_RGBA(192, 192, 255, 255);
        default: return GU_RGBA(255, 0, 0, 255);
    }
}

auto generate_image(Worldgen* gen) -> void {
    for(auto x = 0; x < 256; x++) {
        for(auto y = 0; y < 256; y++) {
            auto cX = x / 32;
            auto cY = y / 32;

            auto vX = (x % 32) / 2;
            auto vY = (y % 32) / 2;

            auto idx = vX * 16 + vY;

            uint8_t bio_val = gen->get_biome_map(cX, cY)[idx];
            float map_val = gen->get_map(cX, cY)[idx];

            Color genColor;
            genColor.c = get_color(bio_val);

            genColor.rgba.r *= map_val;
            genColor.rgba.g *= map_val;
            genColor.rgba.b *= map_val;

            image[x * 256 + y] = genColor.c;
        }
    }
}

auto main() -> int {
	SetupCallbacks();

    scePowerSetClockFrequency(333, 333, 166);
    
    int ret = pspSdkLoadStartModule("./mediaengine.prx", PSP_MEMORY_PARTITION_KERNEL);

	if (ret < 0) {
		printf("ME Module Not Loaded!\n");
        sceKernelExitGame();
	}
	else {
		printf("ME Module Loaded\n");
	}


    GFX::init();
    GFX::clear(0xFF333333);

    Worldgen gen;
    gen.init();

    BENCHMARK(gen.generate_map(), "Map Gen");
    gen.data_fill();

    generate_image(&gen);
    GFX::draw_img(image);

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    struct SceCtrlData padData;

	while(true) {
        sceCtrlReadBufferPositive(&padData, 1);

        if(padData.Buttons & PSP_CTRL_CROSS){
            gen.reseed();
            BENCHMARK(gen.generate_map(), "Map Gen");


            GFX::clear(0xFF333333);
            generate_image(&gen);
            GFX::draw_img(image);
        }
    }
	return 0;
}