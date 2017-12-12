// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "HashMap.h"

int main(void) {
    
    hm_node_ptr hashmap;
    str_t key = { 3, "abc" };
    str_t key1 = {3, "aaa" };
    str_t w = {1,"a"};
    int page1 = 1;
    int page2 = 2;
    if (-1 == hash_new_node(&hashmap, 0)) {
        fprintf(stderr, "Some error!\n");
        exit(EXIT_FAILURE);
    }

    hash_insert(hashmap, key, (void *) &page1);
    

    str_t key2 = { 3, "abc" };
    avlnode_ptr node = hash_search(hashmap, key2);

    assert(node);

    printf("%u\n", node->key.size);

    hash_delete(hashmap, key2);
    hash_erase(hashmap);
    return 0;
}
