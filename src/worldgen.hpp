#pragma once
#include <cstdint>
#include "fastnoise.hpp"

struct NoiseSettings {
    uint8_t octaves;
    float amplitude;
    float frequency;
    float persistence;
    float mod_freq;
    float offset;

    float range_min;
    float range_max;
};


enum BiomeType {
    BIOME_OCEAN     = 0,
    BIOME_RIVER     = 1,
    BIOME_PLAINS    = 2,
    BIOME_DESERT    = 3,
    BIOME_FOREST    = 4,
    BIOME_RAINFOREST= 5,
    BIOME_TUNDRA    = 6,
    BIOME_BEACH     = 7
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

        auto get_biome_map() -> uint8_t* {
            return biome_map;
        }

    private:
        auto get_noise(float x, float y, NoiseSettings* settings) -> float;
        auto get_biome(float temp, float prec) -> BiomeType;
        auto generate_noise(float x, float y) -> float;
        auto get_settings(uint8_t biome) -> NoiseSettings*;
        
        FastNoiseLite fsl;
        float map[128 * 128];
        uint8_t biome_map[128 * 128];
};