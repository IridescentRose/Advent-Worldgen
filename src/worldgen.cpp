
#include "worldgen.hpp"
#include <ctime>

const float FREQUENCY = 1;

Worldgen::Worldgen() {
    fsl.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    fsl.SetFrequency(FREQUENCY * 0.01f);
    fsl.SetSeed(time(NULL));
}

auto Worldgen::get_noise(float x, float y, NoiseSettings settings) -> float {

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

auto Worldgen::generate_map() -> void {
    NoiseSettings settings = {4, 1.0f, 0.38f, 5.0f};

    for(int x = 0; x < 128; x++){
        for(int y = 0; y < 128; y++){
            map[x*128 + y] = get_noise(static_cast<float>(x), static_cast<float>(y), settings);
        }
    }
}