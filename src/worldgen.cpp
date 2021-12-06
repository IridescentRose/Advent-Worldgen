
#include "worldgen.hpp"
#include <ctime>

const float FREQUENCY = 2;

Worldgen::Worldgen() {
    fsl.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    fsl.SetFrequency(FREQUENCY * 0.01f);
    fsl.SetSeed(time(NULL));
}

inline auto range_map(float& input, float curr_range_min, float curr_range_max, float range_min, float range_max) -> void {
    input = (input - curr_range_min) * (range_max - range_min) / (curr_range_max - curr_range_min) + range_min;
}

/*
    uint8_t octaves;
    float amplitude;
    float frequency;
    float persistence;
    float mod_freq;
    float offset

    float range_min;
    float range_max;
*/

/*
    BIOME_OCEAN     = 0,
    BIOME_RIVER     = 1,
    BIOME_PLAINS    = 2,
    BIOME_DESERT    = 3,
    BIOME_FOREST    = 4,
    BIOME_RAINFOREST= 5,
    BIOME_TUNDRA    = 6,
    BIOME_BEACH     = 7
*/
auto Worldgen::get_settings(uint8_t biome) -> NoiseSettings*{
    static NoiseSettings noiseSettings[8] = {
    //  oct  amp  freq   per   mfrq  off  rngmin rngmax     type
        {1, 1.0f, 2.0f, 0.42f, 4.5f, 0.0f, 0.0f, 0.6f}, // OCEAN
        {1, 1.0f, 2.0f, 0.42f, 4.5f, 0.0f, 0.0f, 0.6f}, // RIVER
        {2, 1.0f, 2.0f, 0.42f, 4.5f, 0.0f, 0.0f, 0.6f}, // PLAINS
        {2, 1.0f, 2.0f, 0.42f, 4.5f, 0.0f, 0.0f, 0.6f}, // DESERT
        {3, 1.0f, 2.0f, 0.42f, 4.5f, 0.0f, 0.0f, 0.6f}, // FOREST
        {3, 1.0f, 2.0f, 0.42f, 4.5f, 0.0f, 0.0f, 0.6f}, // RAINFOREST
        {2, 1.0f, 2.0f, 0.42f, 4.5f, 0.0f, 0.0f, 0.6f}, // TUNDRA
        {2, 1.0f, 2.0f, 0.42f, 4.5f, 0.0f, 0.0f, 0.6f}, // BEACH
    };

    return &noiseSettings[biome];
}


/**
 * @brief Gets noise
 * 
 * @param x x
 * @param y y
 * @param settings Settings of the noise generator
 * @return float output
 */
auto Worldgen::get_noise(float x, float y, NoiseSettings* settings) -> float {

    float amp = settings->amplitude;
    float freq = settings->frequency;

    float sum_noise = 0.0f;
    float sum_amp = 0.0f;

    for(auto i = 0; i < settings->octaves; i++) {
        auto noise = fsl.GetNoise(x * freq, y * freq);
        
        noise *= amp;
        sum_noise += noise;
        sum_amp += amp;

        amp *= settings->persistence;
        freq *= settings->mod_freq;
    }

    auto divided = sum_noise / sum_amp;
    range_map(divided, -1.0f, 1.0f, settings->range_min, settings->range_max);

    return divided;
}

/**
 * @brief Generates noise from 0.0f -> 1.0f
 * 
 * @param x 
 * @param y 
 * @return float 
 */
auto Worldgen::generate_noise(float x, float y) -> float {
    auto noise = fsl.GetNoise(x, y);
    range_map(noise, -1.0f, 1.0f, 0.0f, 1.0f);
    return noise;
}

auto Worldgen::get_biome(float temp, float prec) -> BiomeType {

    if(temp < 0.4f) {
        return BIOME_TUNDRA;
    } else if(temp >= 0.4f && temp <= 0.65f) {
        if(prec < 0.5f) {
            return BIOME_FOREST;
        } else {
            return BIOME_RAINFOREST;
        }
    } else {
        if(prec < 0.35f) {
            return BIOME_DESERT;
        } else if(prec >= 0.35f && prec <= 0.65f) {
            return BIOME_PLAINS;
        } else {
            return BIOME_RAINFOREST;
        }
    }
    
    return BIOME_OCEAN;
} 

auto Worldgen::generate_map() -> void {

    //Base layer
    for(int x = 0; x < 128; x++){
        for(int y = 0; y < 128; y++){
            map[x*128 + y] = generate_noise(static_cast<float>(x), static_cast<float>(y));
            map[x*128 + y] += generate_noise(static_cast<float>(x) * 4.0f, static_cast<float>(y) * 4.0f) * 0.75f;
            map[x*128 + y] /= 1.75f;
        }
    }

    //Biome map
    for(int x = 0; x < 128; x++){
        for(int y =0; y < 128; y++){

            auto idx = x*128 + y;
            auto val = map[idx];
            if(val < 0.46f){
                biome_map[idx] = BiomeType::BIOME_OCEAN;
            } else if(val >= 0.46f && val <= 0.48f){
                biome_map[idx] = BiomeType::BIOME_BEACH;
            } else {
                
                //Generate temperature
                float temp = generate_noise(
                    static_cast<float>(x) / 1.0f, 
                    static_cast<float>(y) / 1.0f
                );

                //Generate precipitation
                float prec = generate_noise(
                    static_cast<float>(x) / 1.5f, 
                    static_cast<float>(y) / 1.5f
                );

                biome_map[idx] = static_cast<uint8_t>(get_biome(temp, prec));

            }
        }
    }

    //Perlin Worms for rivers

    //Add Terrain Layer
    for(int x = 0; x < 128; x++){
        for(int y = 0; y < 128; y++){
            auto idx = x*128 + y;

            auto settings = get_settings(biome_map[idx]);

            map[idx] *= 0.5f;
            map[idx] += get_noise(
                static_cast<float>(x), 
                static_cast<float>(y),
                settings
            );

        }
    }

    
}