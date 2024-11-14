#include "common.h"

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
    a = a % mod;
    uint64_t result = 0;
    while (b > 0) {
        if (b % 2 == 1)
            result = (result + a) % mod;
        a = (a * 2) % mod;
        b /= 2;
    }
    return result;
}