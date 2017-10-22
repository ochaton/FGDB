// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#ifndef AVL_HASHMAP_H
#define AVL_HASHMAP_H

#include <stdint.h>
#include <stdlib.h>
#include "AVLnode.h"

enum {
    MAX_HASH_NODE = 1024,
    MAX_HASH_DEP = 1024,
};

typedef struct HashMap  //create here hast table
{
    // if len_of_list[index]>MAX_LEN then top[index] is a ptr on new hash_table, else it is ptr on list
    // dont forget mix hash after create new child table!!!!
    int32_t dep;
    uint32_t  len_of_list[MAX_HASH_NODE]; //here i save the length of each list
    void  *top[MAX_HASH_NODE];  //here is my ptr on list or on hash table
} hm_node;

typedef hm_node *hm_node_ptr;

int32_t hash (str_t key,int32_t dep);

//metodes
int32_t hash_new_node(hm_node_ptr *node, uint32_t dep);
avlnode_ptr hash_search(hm_node_ptr node, str_t key);
int32_t hash_insert(hm_node_ptr node, str_t key, void *page);
int32_t hash_delete(hm_node_ptr node, str_t key);

#endif // AVL_HASHMAP_H
