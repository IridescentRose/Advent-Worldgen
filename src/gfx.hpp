#include <cstdint>

namespace GFX {
    auto init() -> void;
    auto clear(uint32_t c) -> void;
    auto draw_pixel(int x, int y, uint32_t c) -> void;

    auto draw_img(uint32_t* buffer) -> void;
}