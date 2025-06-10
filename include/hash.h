#ifndef HASH_H
#define HASH_H

#include <stdint.h>

// Hash functions for generating preference lists
uint32_t hash_offset(const char *str, uint32_t table_size);
uint32_t hash_skip(const char *str, uint32_t table_size);

// General hash functions
uint32_t djb2_hash(const char *str);
uint32_t sdbm_hash(const char *str);

#endif // HASH_H