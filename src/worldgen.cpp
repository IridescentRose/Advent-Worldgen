
#include "worldgen.hpp"
#include "benchmark.hpp"

const float FREQUENCY = 2;

Worldgen::Worldgen() {
    fsl.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    fsl.SetFrequency(FREQUENCY * 0.01f);

    seed = time(NULL);
    fsl.SetSeed(time(NULL));

}

auto Worldgen::init() -> void {
    data = (uint16_t*)calloc(1, sizeof(uint16_t) * 128 * 128 * 128);
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

    if(temp < 0.35f) {
        return BIOME_TUNDRA;
    } else if(temp >= 0.35f && temp <= 0.65f) {
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

auto step_direction(int16_t& x, int16_t& y, float rot) {
    if(rot >= 0.0f && rot <= 45.0f) {
        x += 1;
    } else if (rot >= 45.0f && rot <= 90.0f) {
        x += 1;
        y += 1;
    } else if (rot >= 90.0f && rot <= 135.0f){
        y += 1;
    } else if (rot >= 135.0f && rot <= 180.0f){
        x -= 1;
        y += 1;
    } else if (rot >= 180.0f && rot <= 225.0f){
        x -= 1;
    } else if (rot >= 225.0f && rot <= 270.0f){
        x -= 1;
        y -= 1;
    } else if (rot >= 270.0f && rot <= 335.0f){
        y -= 1;
    } else if (rot >= 335.0f && rot <= 360.0f){
        x += 1;
        y -= 1;
    }
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
                    static_cast<float>(x) / 3.0f, 
                    static_cast<float>(y) / 3.0f
                );

                //Generate precipitation
                float prec = generate_noise(
                    static_cast<float>(y) / 1.5f,
                    static_cast<float>(x) / 1.5f
                );

                biome_map[idx] = static_cast<uint8_t>(get_biome(temp, prec));

            }
        }
    }

    //Perlin Worms for rivers
    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            //Determine if spawns
            uint8_t hash_result = static_cast<uint8_t>(x + ~seed << y);
            if(hash_result >= 128) continue;

            //Spawn @ Location with Direction
            int16_t worm_head_x = x * 16 + (hash_result % 16);
            int16_t worm_head_y = y * 16 + (hash_result * y) % 16;

            float worm_head_rotation = generate_noise(worm_head_x, worm_head_y) * 360.0f; //Map 0 -> 360 degrees

            //Execute Worm
            const uint8_t MAX_RUNS = 64;

            for(uint8_t runs = 0; runs < MAX_RUNS; runs++) {
                auto idx = worm_head_x*128 + worm_head_y;
                auto bio_val = biome_map[idx];

                if(bio_val == BIOME_OCEAN || bio_val == BIOME_RIVER) break;
                if(worm_head_x < 0 || worm_head_y < 0 || worm_head_x >= 128 || worm_head_y >= 128) break;

                biome_map[idx] = BIOME_RIVER;

                float temp_x = static_cast<float>(worm_head_x) * 8.0f;
                float temp_y = static_cast<float>(worm_head_y) * 8.0f;

                worm_head_rotation += (generate_noise(temp_x, temp_y) -0.5f) * 120.0f;

                step_direction(worm_head_x, worm_head_y, worm_head_rotation);
            }
        }
    }

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

auto Worldgen::data_fill_1() -> void {
    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 128; y++) {
            for(int z = 0; z < 16; z++) {
                data[(z * 128 * 16) + (x * 128) + y] = (y < (map[x * 128 + z] * 128.0f)) ? 0xCAFE : 0x0000;
            }
        }
    }
}


auto Worldgen::data_fill_2() -> void {
    for(int x = 0; x < 16; x++) {
        for(int z = 0; z < 16; z++) {
            for(int y = 0; y < 128; y++) {
                data[(z * 128 * 16) + (x * 128) + y] = (y < (map[x * 128 + z] * 128.0f)) ? 0xCAFE : 0x0000;
            }
        }
    }
}

auto Worldgen::data_fill_3() -> void {
    for(uint8_t x = 0; x < 16; x++) {
        for(uint8_t z = 0; z < 16; z++) {

            uint16_t val = map[x * 128 + z] * 128.0f;

            for(uint16_t y = 0; y < val; y++) {
                data[(z * 128 * 16) + (x * 128) + y] = 0xCAFE;
            }
        }
    }
}

auto Worldgen::data_fill_4() -> void {
    for(uint8_t x = 0; x < 128; x++) {
        for(uint8_t z = 0; z < 128; z++) {
            uint16_t val = map[x * 128 + z] * 128.0f;

            for(uint16_t y = 0; y < val; y++) {
                if(y < 32){
                    data[(z * 128 * 128) + (x * 128) + y] = 0xCAFE;
                } else {
                    data[(z * 128 * 128) + (x * 128) + y] = 0xDEAD;
                }
            }
        }
    }
}

auto Worldgen::write_chunk(int offset_map, int offset_data) -> void {
    for(uint8_t x = 0; x < 16; x++) {
        for(uint8_t z = 0; z < 16; z++) {
            uint16_t val = map[x * 128 + z + offset_map] * 128.0f;

            for(uint16_t y = 0; y < val; y++) {
                if(y < 32){
                    data[(z * 128 * 16) + (x * 16) + y + offset_data] = 0xCAFE;
                } else {
                    data[(z * 128 * 16) + (x * 16) + y + offset_data] = 0xDEAD;
                }
            }
        }
    }
}

auto Worldgen::data_fill_5() -> void {
    for(uint8_t x = 0; x < 8; x++) {
        for(uint8_t y = 0; y < 8; y++){
            write_chunk( ((x*128)+y) * 16, ((y * 128 * 16) + (x * 16)) * 16);
        }
    }
}

auto Worldgen::data_fill() -> void {
    
    BENCHMARK(data_fill_1(), "Data Fill");
    BENCHMARK(data_fill_2(), "Data Fill");
    BENCHMARK(data_fill_3(), "Data Fill");
    BENCHMARK(data_fill_4(), "Data Fill");
    BENCHMARK(data_fill_5(), "Data Fill");

}