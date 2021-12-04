#pragma once
#include <cstdint>
#include "fastnoise.hpp"

struct NoiseSettings {
    uint8_t octaves;
    float frequency;
    float persistence;
    float mod_freq;
};

class Worldgen {
    public:
        Worldgen();

        Worldgen(const Worldgen&) = delete;
        Worldgen(const Worldgen&&) = delete;

        void operator=(const Worldgen&) = delete;
        void operator=(const Worldgen&&) = delete;

        auto generate_map() -> void;

        auto get_map() -> float* {
            return map;
        }

    private:
        auto get_noise(float x, float y, NoiseSettings settings) -> float;
        
        FastNoiseLite fsl;
        float map[128 * 128];
};