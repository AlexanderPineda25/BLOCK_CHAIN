#include "hash.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define PRIME_A 1000000007ULL
#define PRIME_B 1000000009ULL
#define PRIME_C 999999937ULL
#define COMBO_CAP 1000000000000ULL

static const char SYMBOLS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const int SYMBOLS_LEN = 62;

void compute_basic_hash(const char* text, char* hex_out) {
    const uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
    const uint64_t FNV_PRIME        = 1099511628211ULL;

    uint64_t hash = FNV_OFFSET_BASIS;
    size_t len = strlen(text);

    for (size_t i = 0; i < len; ++i) {
        unsigned char byte = (unsigned char)text[i];
        hash ^= (uint64_t)byte;
        hash *= FNV_PRIME;
    }

    hash ^= (uint64_t)len;
    hash *= FNV_PRIME;

    sprintf(hex_out, "%llx", (unsigned long long)hash);
}

int hash_has_trailing_zeros(const char* hex_in, int trailing_zeros) {
    if (trailing_zeros <= 0) return 1;
    int n = (int)strlen(hex_in);
    if (n < trailing_zeros) return 0;
    int start = n - trailing_zeros;
    for (int i = 0; i < trailing_zeros; i++) {
        if (hex_in[start + i] != '0') return 0;
    }
    return 1;
}

uint64_t count_total_variants(int pad_len) {
    uint64_t total = 1ULL;
    for (int i = 0; i < pad_len; i++) {
        total *= (uint64_t)SYMBOLS_LEN;
        if (total > COMBO_CAP) return COMBO_CAP;
    }
    return total;
}

void index_to_string_variant(uint64_t index, int length, char* out) {
    for (int pos = length - 1; pos >= 0; pos--) {
        out[pos] = SYMBOLS[index % (uint64_t)SYMBOLS_LEN];
        index /= (uint64_t)SYMBOLS_LEN;
    }
    out[length] = '\0';
}
