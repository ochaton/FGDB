// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "HashMap.h"

int32_t hash (str_t key,int dep) {
    // `dep` changes constant in hashf
    int32_t mid = 0, p, m;
    switch (dep) {
        case 1: p = 57; m = MAX_HASH_NODE; break;
        case 2: p = 97; m = MAX_HASH_NODE; break;
        case 3: p = 71; m = MAX_HASH_NODE; break;
        case 4: p = 107; m = MAX_HASH_NODE; break;
        default: p = 89; m = MAX_HASH_NODE; break;
    }
    for (uint32_t i = 0; i < key.size; i++){
        mid = (mid + p * ((int)(key.ptr[i]))) % m;
    }
    return mid;
}