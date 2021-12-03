#include "setup.hpp"
#include "gfx.hpp" 
#include "fastnoise.hpp"
#include "benchmark.hpp"
#include <pspgu.h>

PSP_MODULE_INFO("Worldgen", 0, 1, 0);

FastNoiseLite fsl;

float map[128 * 128];
const float FREQUENCY = 1;

struct NoiseSettings {
    uint8_t octaves;
    float frequency;
    float persistence;
    float mod_freq;
};

auto get_noise(float x, float y, NoiseSettings settings) -> float {

    float amp = 1.0f;
    float freq = settings.frequency;

    float sum_noise = 0.0f;
    float sum_amp = 0.0f;

    for(auto i = 0; i < settings.octaves; i++) {
        auto normalized_noise = (fsl.GetNoise(x * freq, y * freq) + 1.0f) / 2.0f * amp;
        sum_noise += normalized_noise;
        sum_amp += amp;

        amp *= settings.persistence;
        freq *= settings.mod_freq;
    }

    return sum_noise / sum_amp;
}

auto generate_map() -> void {
    NoiseSettings settings = {4, 1.0f, 0.38f, 5.0f};

    for(int x = 0; x < 128; x++){
        for(int y = 0; y < 128; y++){
            map[x*128 + y] = get_noise(static_cast<float>(x), static_cast<float>(y), settings);
        }
    }
}

uint32_t image[256 * 256];

auto generate_image() -> void {
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
    
    fsl.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    fsl.SetFrequency(FREQUENCY * 0.01f);

    fsl.SetSeed(-1);

    BENCHMARK(generate_map(), "Map Gen");

    generate_image();
    GFX::draw_img(image);

	while(true) {}
	return 0;
}