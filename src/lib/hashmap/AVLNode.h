// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#ifndef AVLNODE_H
#define AVLNODE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct AVLNode * avlnode_ptr;

#include "common.h"

enum {
    MAX_KEY_LEN = 1024,
};

typedef struct AVLNode
{
    str_t key;
    void *meta;
    struct AVLNode *left ,*right,*parent;
    int32_t hight;
} avlnode;

int32_t key_comp(str_t key_first, str_t key_second);
int32_t max_32t(int32_t first, int32_t second);

// methods
int32_t avl_new_node(avlnode_ptr *new_node, str_t key, void *meta);
//    avlnode_ptr avl_search(avlnode_ptr node, str_t key);
int32_t avl_insert_node(avlnode_ptr *node, avlnode_ptr node_new);
int32_t avl_remove_node(avlnode_ptr *node, avlnode_ptr node_new);
int32_t avl_delete_node(avlnode_ptr *node);
void avl_erase(avlnode_ptr node);

int32_t avl_calc_balance(avlnode_ptr node);
void avl_calc_hight(avlnode_ptr node);
void avl_rebalance(avlnode_ptr *node);

avlnode_ptr avl_search_line(avlnode_ptr node, str_t key);

#endif
