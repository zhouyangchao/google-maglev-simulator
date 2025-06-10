#include "hash.h"
#include <stdio.h>

// DJB2 hash algorithm
uint32_t djb2_hash(const char *str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

// SDBM hash algorithm
uint32_t sdbm_hash(const char *str) {
    uint32_t hash = 0;
    int c;

    while ((c = *str++)) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

// FNV-1a hash algorithm (newly added for better distribution)
uint32_t fnv1a_hash(const char *str) {
    uint32_t hash = 2166136261u;
    int c;

    while ((c = *str++)) {
        hash ^= c;
        hash *= 16777619u;
    }

    return hash;
}

// Calculate offset (used for generating starting position of preference list)
uint32_t hash_offset(const char *str, uint32_t table_size) {
    // Use combined hash to increase randomness
    uint32_t h1 = djb2_hash(str);
    uint32_t h2 = fnv1a_hash(str);
    uint32_t combined = h1 ^ (h2 << 16) ^ (h2 >> 16);
    return combined % table_size;
}

// Calculate jump step size (used for generating step size of preference list)
uint32_t hash_skip(const char *str, uint32_t table_size) {
    // Use different combination method to calculate skip
    uint32_t h1 = sdbm_hash(str);
    uint32_t h2 = fnv1a_hash(str);
    uint32_t combined = h1 ^ (h2 << 8) ^ (h2 >> 24);
    uint32_t skip = combined % (table_size - 1) + 1;
    return skip;
}