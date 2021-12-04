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

        auto get_biome_map() -> float* {
            return biome_map;
        }

    private:
        auto get_noise(float x, float y, NoiseSettings settings) -> float;
        auto generate_biomes() -> void;
        
        FastNoiseLite fsl;
        float map[128 * 128];
        float biome_map[128 * 128];
};