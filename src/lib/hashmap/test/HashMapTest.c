// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "HashMap.h"
#include "AVLTest.h"

int32_t hash_test(str_t key, int32_t dep) {
    return key.size;
}

int32_t hash_test_print(hm_node_ptr node) {
    for (uint32_t i = 0; i <  MAX_HASH_NODE; i++) {
        if (node->len_of_list[i] != 0) {
            avl_test_print(node->top[i], 1);
        }
    }
    return 1;
}

int32_t hash_test_insert(hm_node_ptr node, str_t key, void *page) {
    if (!node) {
        return -1;
    }
    int32_t mid_key = hash_test(key, node->dep);
    while (node->len_of_list[mid_key] > MAX_HASH_DEP) {
        node = node->top[mid_key];
        mid_key = hash_test(key, node->dep);
    }
    node->len_of_list[mid_key]++;

    avlnode_ptr new_avl_node;
    avl_new_node(&new_avl_node, key, page);
    //avl_test_print(new_avl_node, 1);
    avl_insert_node(&(node->top[mid_key]), new_avl_node);
    __hash_remake(node, mid_key);
    return 1;
}

avlnode_ptr hash_test_search(hm_node_ptr node, str_t key) {
    if (!node) {
        return NULL;
    }
    int32_t mid_key = hash_test(key, node->dep);
    while (1) {
        if (node->len_of_list[mid_key] < MAX_HASH_DEP) {
            return avl_test_search(node->top[mid_key], key);
        } else {
            node = node->top[mid_key];
            mid_key = hash_test(key, node->dep);
        }
    }
}

int32_t hash_test_delete(hm_node_ptr node, str_t key) {
    if (!node) {
        return 0;
    }
    int32_t mid_key = hash_test(key, node->dep);
    while (1) {
        if (node->len_of_list[mid_key] < MAX_HASH_DEP) {
            printf("kek\n");
            avlnode_ptr new_avl_node;
            avl_new_node(&new_avl_node, key, NULL);
            avl_test_remove_node(&(node->top[mid_key]), new_avl_node);
            free(new_avl_node);
            node->len_of_list[mid_key]--;
            break;
        } else {
            node = node->top[mid_key];
            mid_key = hash_test(key, node->dep);
        }
    }
    return 1;
}
