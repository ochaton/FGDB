// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// struct hashmap_key {
//  /* Pointer to offset inside page (stored inside headers of arena-pages) */
//  page_header_key_t * header_key_id;
//  /* Page identificator. Storing this we can find the page, where stored value */
//  page_id_t page;
// };


//#include "lib/hashmap/HashMap.h"
#include "HashMap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {

    hm_node_ptr hashmap;
    str_t key = { 3, "abc" };

    int page1 = 1;

    if (-1 == hash_new_node(&hashmap, 0)) {
        fprintf(stderr, "Some error!\n");
        exit(EXIT_FAILURE);
    }

    hash_insert(hashmap, key, (void *) &page1);
    str_t key2 = { 3, "abc" };
    avlnode_ptr node = hash_search(hashmap, key2);

    //assert(node);
    printf("%s\n", node->key.ptr);
    
    hash_delete(hashmap, key2);
    node = hash_search(hashmap, key2);
    if (node) {
        printf("%s\n", node->key.ptr);
    }
    return 0;
}