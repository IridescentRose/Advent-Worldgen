#include "gfx.hpp"
#include <pspge.h>
#include <pspdisplay.h>
#include <psputils.h>

namespace GFX {
    uint32_t* draw_buffer;

    auto init() -> void {
        draw_buffer = static_cast<uint32_t*>(sceGeEdramGetAddr());

        sceDisplaySetMode(0, 480, 272);
        sceDisplaySetFrameBuf(draw_buffer, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, 1);
    }

    auto clear(uint32_t c) -> void {
        for(int i = 0; i < 512 * 272; i++)
            draw_buffer[i] = c;
        
        sceKernelDcacheWritebackInvalidateAll();
    }

    auto draw_pixel(int x, int y, uint32_t c) -> void {
        draw_buffer[x + y * 512] = c;
    }

    auto draw_img(uint32_t* buffer) -> void {
        for(int x = 0; x < 256; x++) {
            for(int y = 0; y < 256; y++) {
                int idx = (x + 112) + (y + 8) * 512;
                draw_buffer[idx] = buffer[x + y * 256];
            }
        }
        sceKernelDcacheWritebackInvalidateAll();
    }

}