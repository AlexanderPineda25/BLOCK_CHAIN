#ifndef HASH_H
#define HASH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void compute_basic_hash(const char* text, char* hex_out);
int  hash_has_trailing_zeros(const char* hex_in, int trailing_zeros);
uint64_t count_total_variants(int pad_len);
void index_to_string_variant(uint64_t index, int length, char* out);

#ifdef __cplusplus
}
#endif

#endif
