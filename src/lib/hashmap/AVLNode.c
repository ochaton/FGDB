// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "AVLNode.h"

#include <stdio.h>

// prototypes of local methods
static int32_t __avl_get_hight(avlnode_ptr node);
static void __avl_right_move(avlnode_ptr *node);
static void __avl_left_move(avlnode_ptr *node);
static void __avl_LR_move(avlnode_ptr *node);
static void __avl_RL_move(avlnode_ptr *node);


void avl_test_print(avlnode_ptr go) {
    if (!go) {
        return;
    }
    avl_test_print(go->left);
    avl_test_print(go->right);
    printf("A_Node is %s\n", go->key.ptr);
}
int32_t avl_new_node(avlnode_ptr *new_node, str_t key, void *meta) {

    avlnode_ptr mid_node = (avlnode *) malloc(sizeof(avlnode));
    if (!mid_node) {
        return -1;
    }
    mid_node->hight = 1;
    mid_node->key.ptr = (char *) malloc(key.size);
    mid_node->key.size = key.size;
    memcpy(mid_node->key.ptr, key.ptr, key.size);
    mid_node->meta = meta;
    mid_node->left = mid_node->right = mid_node->parent = NULL;
    *new_node = mid_node;

    return 1;
}

avlnode_ptr avl_search(avlnode_ptr node, str_t key) {
    if (!node) {
        return NULL;
    }
    if (!key_comp(node->key, key)) {
        return node;
    } else {
        if (key_comp(node->key, key) > 0) {
            return avl_search(node->right, key);
        } else {
            return avl_search(node->left, key);
        }
    }
}

avlnode_ptr avl_search_line(avlnode_ptr node, str_t key) {
    if (!node) {
        return NULL;
    }
    /*if (key.ptr[0] == 'R') {
        //printf("A_i %s\n", key.ptr);
        printf("+++++++++\n");
        avl_test_print(node);
        printf("+++++++++\n");
    }*/
    int32_t mid = key_comp(node->key, key);
    while ((node) && (mid != 0)) {
        if (mid > 0) {
            node = node->right;
        } else {
            node = node->left;
        }
        if (node) {
            mid = key_comp(node->key, key);
        }
    }
    return node;
}
int32_t avl_insert_node(avlnode_ptr *node, avlnode_ptr node_new) {
    if (key_comp((*node)->key, node_new->key) > 0) {
        if ((*node)->right) {
            avl_insert_node(&((*node)->right), node_new);
        } else {
            (*node)->right = node_new;
        }
        if ((*node)->right) {
            (*node)->right->parent = *node;
        }
    } else if (key_comp((*node)->key, node_new->key) < 0) {
        if ((*node)->left) {
            avl_insert_node(&((*node)->left), node_new);
        } else {
            (*node)->left = node_new;
        }

        if ((*node)->left) {
            (*node)->left->parent = *node;
        }
    } else {
        //printf("WTF!!!!!!!!  %s\n",node_new->key.ptr );
        return -1;
    }
    avl_calc_hight(*node);
    avl_rebalance(node);
    return 1;
}

int32_t avl_delete_node(avlnode_ptr *node) {
    if (!(*node)) {
        return 1;
    }
    if ((*node)->key.ptr) {
        free((*node)->key.ptr);
        (*node)->key.ptr = NULL;
    }
    free(*node);
    *node = NULL;
    return 1;
}

// here we can delete meta
int32_t avl_remove_node(avlnode_ptr *node, avlnode_ptr node_new) {

    if (!(*node)) {
        return -1;
    }
    if (key_comp((*node)->key, node_new->key) > 0) {
        avlnode_ptr mid = (*node)->right;
        avl_remove_node(&(mid), node_new);
    } else if (key_comp((*node)->key, node_new->key) < 0) {
        avlnode_ptr mid = (*node)->left;
        avl_remove_node(&(mid), node_new);
    } else {
        if (!(*node)->left && !(*node)->right) {
            avlnode_ptr last = (*node)->parent;
            if (last) {
                if (last->left == *node) {
                    last->left = NULL;
                } else if (last->right == *node) {
                    last->right = NULL;
                }
            }
            avl_calc_hight(last);
            avl_delete_node(node);
            *node = NULL;
        } else if ((*node)->left && (*node)->right) {
            /*if ((*node)->key.ptr[0] == 'W') {
                printf("AA_Node is ");
                for (int32_t i = 0; i < (*node)->key.size; i++) {
                    printf("%c", (*node)->key.ptr[i]);
                }
                printf("\n");
            }*/
            avlnode_ptr mid1 = *node, mid2;
            mid1 = mid1->left;
            while (mid1->right) {
                mid1 = mid1->right;
            }
            str_t kkey = (*node)->key;
            (*node)->key = mid1->key;
            mid1->key = kkey;

            void * mmeta = (*node)->meta;
            (*node)->meta = mid1->meta;
            mid1->meta = mmeta;
            if (mid1 == (*node)->left) {
                (*node)->left = mid1->left;
                if (mid1->left) {
                    mid1->left->parent = *node;
                }
                avl_calc_hight(*node);
                avl_rebalance(node);
            } else {
                if (mid1->left) {
                    mid1->left->parent = mid1->parent;
                }
                mid1->parent->right = mid1->left;
                mid2 = mid1->parent;
                avl_delete_node(&mid1);
                while (mid2 != (*node)) {
                    avl_calc_hight(mid2);
                    avl_rebalance(&mid2);
                    mid2 = mid2->parent;
                }
                avl_calc_hight(*node);
                avl_rebalance(node);
            }
            avl_delete_node(&mid1);
        } else if ((*node)->left) {
            avlnode_ptr mid1 = (*node)->left;

            str_t kkey = (*node)->key;
            (*node)->key = mid1->key;
            mid1->key = kkey;

            void * mmeta = (*node)->meta;
            (*node)->meta = mid1->meta;
            mid1->meta = mmeta;


            (*node)->left = mid1->left;
            if ((*node)->left) {
                (*node)->left->parent = (*node);
            }
            (*node)->right = mid1->right;
            if ((*node)->right) {
                (*node)->right->parent = (*node);
            }
            avl_calc_hight(*node);
            avl_rebalance(node);
            avl_delete_node(&mid1);

        } else if ((*node)->right) {
            int ww = 0;
            if ((*node)->key.ptr[0] == 'W') {
                ww = 1;
                printf("AA_Node is ");
                for (int32_t i = 0; i < (*node)->key.size; i++) {
                    printf("%c", (*node)->key.ptr[i]);
                }
                printf("\n");
            }
            avlnode_ptr mid1 = (*node)->right;
            ////////////////////////////
            //free((*node)->key.ptr);
            //(*node)->key.size = 0;
            ////////////////////////////
            str_t kkey = (*node)->key;
            (*node)->key = mid1->key;
            mid1->key = kkey;
            void * mmeta = (*node)->meta;
            (*node)->meta = mid1->meta;
            mid1->meta = mmeta;
            (*node)->left = mid1->left;
            if ((*node)->left) {
                (*node)->left->parent = (*node);
            }
            (*node)->right = mid1->right;
            if ((*node)->right) {
                (*node)->right->parent = (*node);
            }
            avl_calc_hight(*node);
            avl_rebalance(node);
            avl_delete_node(&mid1);
            /*if (ww) {
                printf("AA_Node is ");
                for (int32_t i = 0; i < (*node)->key.size; i++) {
                    printf("%c", (*node)->key.ptr[i]);
                }
                printf("\n");
            }*/
        }
        return 1;
    }

    avl_calc_hight(*node);
    avl_rebalance(node);
    return 1;
}

void avl_erase(avlnode_ptr node) {
    if (!node) return;
    if (node->left) {
        avl_erase(node->left);
    }
    if (node->right) {
        avl_erase(node->right);
    }
    node->left = node->right = NULL;
    avl_delete_node(&node);
}

void avl_calc_hight(avlnode_ptr node) {
    if (node) {
        node->hight = max_32t(__avl_get_hight(node->left), __avl_get_hight(node->right)) + 1;
    }
}

int32_t avl_calc_balance(avlnode_ptr node) {
    if (node) {
        return __avl_get_hight(node->right) - __avl_get_hight(node->left);
    }
    return 0;
}

void avl_rebalance(avlnode_ptr *node) {
    if ((avl_calc_balance(*node) == -2) && (avl_calc_balance((*node)->left) == -1)) {
        __avl_right_move(node);
    } else if ((avl_calc_balance(*node) == 2) && (avl_calc_balance((*node)->right) == 1)) {
        __avl_left_move(node);
    } else if ((avl_calc_balance(*node) == -2) && (avl_calc_balance((*node)->left) == 1)) {
        __avl_LR_move(node);
    } else if ((avl_calc_balance(*node) == 2) && (avl_calc_balance((*node)->left) == -1)) {
        __avl_RL_move(node);
    }
}

static int32_t __avl_get_hight(avlnode_ptr node) {
    return (node) ? node->hight : 0;
}

static void __avl_right_move(avlnode_ptr *node) {
    avlnode_ptr mid = (*node)->left;
    (*node)->left = mid->right;
    if (mid->right) {
        mid->right->parent = (*node);
    }
    mid->parent = (*node)->parent;
    (*node)->parent = mid;
    mid->right = (*node);
    (*node) = mid;
    avl_calc_hight((*node)->right);
    avl_calc_hight(*node);
}

static void __avl_left_move(avlnode_ptr *node) {
    avlnode_ptr mid = (*node)->right;
    (*node)->right = mid->left;
    if (mid->left) {
        mid->left->parent = (*node);
    }
    mid->parent = (*node)->parent;
    (*node)->parent = mid;
    mid->left = (*node);
    (*node) = mid;
    avl_calc_hight((*node)->left);
    avl_calc_hight((*node));
}

static void __avl_LR_move(avlnode_ptr *node) {
    __avl_left_move(&(*node)->left);
    __avl_right_move(node);
}

static void __avl_RL_move(avlnode_ptr *node) {
    __avl_right_move(&(*node)->right);
    __avl_left_move(node);
}
