// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "HashMap.h"


int32_t hash (str_t key,int dep) { 
// dep if parametr for change const in hash f
    int32_t mid = 0, p, m;
    switch (dep) {
        case 1: p = 57; m = 1031; break;
        case 2: p = 97; m = 3089; break;
        case 3: p = 71; m = 2029; break;
        case 4: p = 107; m = 4019; break;
        default: p = 89; m = 6079; break;
    }
    for (uint32_t i = 0; i < key.size; i++){
        mid = (mid + p * ((int)(key.ptr[i]))) % m;
    }
    return mid;
}