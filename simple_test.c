#include <stdint.h>

uint32_t process(uint32_t x) {
    uint32_t a = x & 0x7;
    uint32_t b = a << 2;
    uint32_t c = b | 0x3;
    uint32_t d = c ^ 0x5;
    uint32_t e = ~d;

    return e;
}

