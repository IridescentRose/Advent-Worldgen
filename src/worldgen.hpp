#pragma once
#include <ctime>
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

struct Chunk {
    float height_map[16 * 16];
    uint8_t biome_map[16 * 16];
};

class Worldgen {
    public:
        Worldgen();

        Worldgen(const Worldgen&) = delete;
        Worldgen(const Worldgen&&) = delete;

        void operator=(const Worldgen&) = delete;
        void operator=(const Worldgen&&) = delete;

        auto init() -> void;
        auto generate_map() -> void;
        auto data_fill() -> void;

        auto get_map(int x, int y) -> float* {
            return chunks[x*8 + y].height_map;
        }

        auto reseed() -> void {
            seed = time(NULL);
            fsl.SetSeed(time(NULL));
        }

        auto get_biome_map(int x, int y) -> uint8_t* {
            return chunks[x*8 + y].biome_map;
        }

    private:
        auto get_noise(float x, float y, NoiseSettings* settings) -> float;
        auto get_biome(float temp, float prec) -> BiomeType;
        auto generate_noise(float x, float y) -> float;
        auto get_settings(uint8_t biome) -> NoiseSettings*;
        friend int generate_ME(int wgen);

        auto gen_base_layer(int cX, int cY) -> void;
        auto gen_biome_map(int cX, int cY) -> void;
        auto gen_biome_layer(int cX, int cY) -> void;

        /*
        auto data_fill_5() -> void;
        auto write_chunk(int offset_map, int offset_data) -> void;
        */

        FastNoiseLite fsl;
        Chunk chunks[8 * 8];
        uint64_t seed;

        uint16_t* data;
};