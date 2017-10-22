// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#ifndef AVLNODE_H
#define AVLNODE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum {
    MAX_KEY_LEN = 1024,
};

typedef struct {
    uint32_t size;
    char *ptr;
} str_t;

typedef struct AVLNode
{
    str_t key;
    void *page;
    struct AVLNode *left ,*right,*parent;
    int32_t hight;
} avlnode;

typedef avlnode *avlnode_ptr;

int32_t key_comp(str_t key_first, str_t key_second);

// methods:
int32_t avl_new_node(avlnode_ptr *new_node, str_t key, void *page);
avlnode_ptr avl_search(avlnode_ptr node, str_t key);
int32_t avl_insert_node(avlnode_ptr *node, avlnode_ptr node_new);
int32_t avl_remove_node(avlnode_ptr *node, avlnode_ptr node_new);

// extern metodes for HashMap.h:
void __avl_rebalance(avlnode_ptr *node);
void __avl_erase(avlnode_ptr node);
void __avl_calc_hight(avlnode_ptr node);

#endif // AVLNODE_H
