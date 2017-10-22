// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "AVLnode.h"

int32_t key_comp(str_t key_first, str_t key_second) {
    if ((key_first.size - key_second.size) != 0) {
        return key_second.size - key_first.size;
    }
    return strcmp(key_second.ptr,key_first.ptr);
}
int32_t max_32t(int32_t first, int32_t second) {
    return (first > second) ? first : second;
}