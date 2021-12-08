
#include "worldgen.hpp"
#include "benchmark.hpp"
#include "me.h"
#include <pspkernel.h>
#include <cstdio>
#include <malloc.h>

const float FREQUENCY = 2;

Worldgen::Worldgen() {
    fsl.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    fsl.SetFrequency(FREQUENCY * 0.01f);

    seed = time(NULL);
    fsl.SetSeed(time(NULL));

}


volatile struct me_struct* mei;
void* malloc_64(int size)
{
	int mod_64 = size & 0x3f;
	if (mod_64 != 0) size += 64 - mod_64;
	return((void*)memalign(64, size));
}

volatile int x = 0;

int generate_ME(int wptr) {
    Worldgen* wgen = (Worldgen*)wptr;
    wgen->generate_map();

    return (int)wgen;
}

auto Worldgen::init() -> void {
    data = (uint16_t*)calloc(1, sizeof(uint16_t) * 128 * 128 * 128);


    mei = (volatile struct me_struct*)malloc_64(sizeof(struct me_struct));
	mei = (volatile struct me_struct*)( ((u32)(mei)) | 0x40000000);
	sceKernelDcacheWritebackInvalidateAll();
    
    InitME(mei);
}

inline auto range_map(float& input, float curr_range_min, float curr_range_max, float range_min, float range_max) -> void {
    input = (input - curr_range_min) * (range_max - range_min) / (curr_range_max - curr_range_min) + range_min;
}

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

auto Worldgen::gen_base_layer(int cX, int cY) -> void {
    for(int x = 0; x < 16; x++){
        for(int y = 0; y < 16; y++){
            auto cIdx = cX * 8 + cY;
            chunks[cIdx].height_map[x*16 + y] = generate_noise(static_cast<float>(x + cX*16), static_cast<float>(y + cY*16));
            chunks[cIdx].height_map[x*16 + y] += generate_noise(static_cast<float>(x + cX*16) * 4.0f, static_cast<float>(y + cY*16) * 4.0f) * 0.75f;
            chunks[cIdx].height_map[x*16 + y] /= 1.75f;
        }
    }
}

auto Worldgen::gen_biome_layer(int cX, int cY) -> void {
    for(int x = 0; x < 16; x++){
        for(int y = 0; y < 16; y++){
            auto cIdx = cX * 8 + cY;
            auto idx = x*16 + y;

            auto settings = get_settings(chunks[cIdx].biome_map[idx]);

            chunks[cIdx].height_map[idx] *= 0.5f;
            chunks[cIdx].height_map[idx] += get_noise(
                static_cast<float>(x + cX*16), 
                static_cast<float>(y + cY*16),
                settings
            );
        }
    }
}

auto Worldgen::gen_biome_map(int cX, int cY) -> void {
    //Biome map
    for(int x = 0; x < 16; x++){
        for(int y =0; y < 16; y++){

            auto idx = x*16 + y;
            auto cIdx = cX * 8 + cY;
            auto val = chunks[cIdx].height_map[idx];

            if(val < 0.46f){
                chunks[cIdx].biome_map[idx] = BiomeType::BIOME_OCEAN;
            } else if(val >= 0.46f && val <= 0.48f){
                chunks[cIdx].biome_map[idx] = BiomeType::BIOME_BEACH;
            } else {
                
                //Generate temperature
                float temp = generate_noise(
                    static_cast<float>(x + cX*16) / 3.0f, 
                    static_cast<float>(y + cY*16) / 3.0f
                );

                //Generate precipitation
                float prec = generate_noise(
                    static_cast<float>(y + cY*16) / 1.5f,
                    static_cast<float>(x + cX*16) / 1.5f
                );

                chunks[cIdx].biome_map[idx] = static_cast<uint8_t>(get_biome(temp, prec));

            }
        }
    }
}

int gen_biome_map_ME(int genME) {
    GenerateME* gen = (GenerateME*)genME;

    volatile Worldgen* wgen = gen->gen;
    ((Worldgen*)wgen)->gen_biome_map(gen->cX, gen->cY);

    return (int)wgen;
}

int gen_biome_layer_ME(int genME) {
    GenerateME* gen = (GenerateME*)genME;

    volatile Worldgen* wgen = gen->gen;
    ((Worldgen*)wgen)->gen_biome_layer(gen->cX, gen->cY);

    return (int)wgen;
}


int gen_base_layer_ME(int genME) {
    volatile GenerateME* gen = (volatile GenerateME*)genME;

    volatile Worldgen* wgen = gen->gen;
    ((Worldgen*) wgen)->gen_base_layer(gen->cX, gen->cY);

    return gen->cX;
}


int simulate_worm_ME(int genME) {
    volatile GenerateME* gen = (volatile GenerateME*)genME;

    volatile Worldgen* wgen = gen->gen;
    ((Worldgen*) wgen)->simulate_worm(gen->hash, gen->cX, gen->cY);

    return gen->cX;
}

auto Worldgen::simulate_worm(int hash_result, int x, int y) -> void {
    //Spawn @ Location with Direction
    int16_t worm_head_x = x * 16 + (hash_result % 16);
    int16_t worm_head_y = y * 16 + (hash_result * y) % 16;
    float worm_head_rotation = generate_noise(worm_head_x, worm_head_y) * 360.0f; //Map 0 -> 360 degree
    //Execute Worm
    const uint8_t MAX_RUNS = 64;
    for(uint8_t runs = 0; runs < MAX_RUNS; runs++) {
        auto cX = worm_head_x / 16;
        auto cY = worm_head_y / 16;
        auto cIdx = cX * 8 + cY;
        auto wX = worm_head_x % 16;
        auto wY = worm_head_y % 16;
        auto idx = wX*16 + wY;
        auto bio_val = chunks[cIdx].biome_map[idx];

        if(bio_val == BIOME_OCEAN || bio_val == BIOME_RIVER) break;
        if(worm_head_x < 0 || worm_head_y < 0 || worm_head_x >= 128 || worm_head_y >= 128) break;
        
        chunks[cIdx].biome_map[idx] = BIOME_RIVER;
        float temp_x = static_cast<float>(worm_head_x) * 8.0f;
        float temp_y = static_cast<float>(worm_head_y) * 8.0f;
        worm_head_rotation += (generate_noise(temp_x, temp_y) - 0.5f) * 120.0f;
        step_direction(worm_head_x, worm_head_y, worm_head_rotation);
    }
}

auto Worldgen::generate_map() -> void {
    //Base layer
    for(int cX = 0; cX < 8; cX++){
        for(int cY = 0; cY < 8; cY++) {
            GenerateME genME2;
            genME2.cX = cX;
            genME2.cY = cY;
            genME2.gen = this;

            sceKernelDcacheWritebackInvalidateRange((void*)&genME2, sizeof(GenerateME));
            
            BeginME(mei, (int)gen_base_layer_ME, (int)&genME2, -1, NULL, -1, NULL);
            
            cY++;
            gen_base_layer(cX, cY);
            WaitME(mei);
        }
    }
    

    for(int cX = 0; cX < 8; cX++){
        for(int cY = 0; cY < 8; cY++) {
            GenerateME genME2;
            genME2.cX = cX;
            genME2.cY = cY;
            genME2.gen = this;

            sceKernelDcacheWritebackInvalidateRange((void*)&genME2, sizeof(GenerateME));
            
            BeginME(mei, (int)gen_biome_map_ME, (int)&genME2, -1, NULL, -1, NULL);
            
            cY++;
            gen_biome_map(cX, cY);
            WaitME(mei);
        }
    }

    //Perlin Worms for rivers
    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 8; y++) {
            //Determine if spawns
            uint8_t hash_result = static_cast<uint8_t>(x + ~seed << y);
            if(hash_result >= 128) continue;

            
            GenerateME genME2;
            genME2.cX = x;
            genME2.cY = y;
            genME2.gen = this;
            genME2.hash = hash_result;

            if(mei->done){
                sceKernelDcacheWritebackInvalidateRange((void*)&genME2, sizeof(GenerateME));
                BeginME(mei, (int)simulate_worm_ME, (int)&genME2, -1, NULL, -1, NULL);
            } else {
                simulate_worm(hash_result, x, y);
            }
        }
    }

    WaitME(mei);
    
    //Add Terrain Layer
    for(int cX = 0; cX < 8; cX++){
        for(int cY = 0; cY < 8; cY++) {
            GenerateME genME2;
            genME2.cX = cX;
            genME2.cY = cY;
            genME2.gen = this;

            sceKernelDcacheWritebackInvalidateRange((void*)&genME2, sizeof(GenerateME));
            
            BeginME(mei, (int)gen_biome_layer_ME, (int)&genME2, -1, NULL, -1, NULL);
            
            cY++;
            gen_biome_layer(cX, cY);
            WaitME(mei);
        }
    }

}

/*
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
}*/

auto Worldgen::data_fill() -> void {
    //while(!mei->done){}
    //BENCHMARK(data_fill_5(), "Data Fill");
}