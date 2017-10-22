// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "AVLnode.h"

//prototipes local methods

int32_t __avl_get_hight(avlnode_ptr node);
void __avl_calc_hight(avlnode_ptr node);
int32_t __avl_calc_balance(avlnode_ptr node);
void __avl_right_move(avlnode_ptr *node);
void __avl_left_move(avlnode_ptr *node);
void __avl_LR_move(avlnode_ptr *node);
void __avl_RL_move(avlnode_ptr *node);
void __avl_rebalance(avlnode_ptr *node);
void __avl_erase(avlnode_ptr node); // her we can delete page
//methods

int32_t avl_new_node(avlnode_ptr *new_node, str_t key, void *page) {
    (*new_node) = (avlnode_ptr) malloc(sizeof(avlnode));
    if (!(*new_node)) {
        return -1;
    }
    (*new_node)->key = key;
    (*new_node)->page = page;
    (*new_node)->left = (*new_node)->right =(*new_node)->parent = NULL;
    return 1;
}
avlnode_ptr avl_search(avlnode_ptr node, str_t key) {
    if (!node) {
        return NULL;
    } 
    if (!key_comp(node->key, key)) {
        return node;
    } else {
        if (key_comp(node->key, key) < 0) {
            return avl_search(node->right, key);
        } else {
            return avl_search(node->left, key);
        }
    }
}
int32_t avl_insert_node(avlnode_ptr *node, avlnode_ptr node_new) {
    if (!(*node)) {
        *node = node_new;
        return 1;
    }
    if (key_comp((*node)->key, node_new->key) > 0) {
        avl_insert_node(&(*node)->right, node_new);
        if ((*node)->right) {
            (*node)->right->parent = *node;
        }
    } else if (key_comp((*node)->key, node_new->key) < 0) {
        avl_insert_node(&(*node)->left, node_new);
        if ((*node)->left) {
            (*node)->left->parent = *node;
        }
    } else {
        return -1;
    }
    __avl_calc_hight(*node);
    __avl_rebalance(node);
    return 1;
}
//here we can delete page
int32_t avl_remove_node(avlnode_ptr *node, avlnode_ptr node_new) {
    if (!(*node)) {
        return -1;
    }
    if (key_comp((*node)->key, node_new->key) > 0) {
        return avl_remove_node(&(*node)->right, node_new);
    } else if (key_comp((*node)->key,node_new->key) < 0) {
        return avl_remove_node(&(*node)->left, node_new);
    } else {
        if (!(*node)->left && !(*node)->right) {
            avlnode_ptr last = (*node)->parent;
            if (last && (last->left == *node)) {
                last->left == NULL;
            } else if (last) {
                last->right == NULL;
            }
            __avl_calc_hight(last);
            //delete key
            //delete page
            //???????????
            free((*node));
            
        } else if ((*node)->left && (*node)->right) {
            avlnode_ptr mid1 = *node, mid2;
            mid1 = mid1-> left;
            while (mid1->right) {
                mid1 = mid1->right;
            }
            //delete key
            //delete page
            //???????????
            (*node)->key = mid1->key;
            (*node)->page = mid1->page;
            if (mid1 == (*node)->left) {
                (*node)->left = mid1->left;
                if (mid1->left) {
                    mid1->left->parent = *node;
                }
                __avl_calc_hight(*node);
                __avl_rebalance(node);
            } else {
                if (mid1->left) {
                    mid1->left->parent = mid1->parent;
                }
                mid1->parent->right = mid1->left;
                mid2 = mid1->parent;
                mid1->key.ptr = NULL;
                mid1->page = NULL;
                free(mid1);
                while (mid2 != (*node)) {
                    __avl_calc_hight(mid2);
                    __avl_rebalance(&mid2);
                    mid2 = mid2->parent;
                }
                __avl_calc_hight(*node);
                __avl_rebalance(node);
            }
        } else if ((*node)->left) {
            avlnode_ptr mid1 = (*node)->left;
            //delete key
            //delete page
            //???????????
            (*node)->key = mid1->key;
            (*node)->page = mid1->page;
            (*node)->left = mid1->left;
            (*node)->right = mid1->right;
            __avl_calc_hight(*node);
            mid1->key.ptr = NULL;
            mid1->page = NULL;
            free(mid1);
        } else if ((*node)->right) {
            avlnode_ptr mid1 = (*node)->right;
            //delete key
            //delete page
            //???????????
            (*node)->key = mid1->key;
            (*node)->page = mid1->page;
            (*node)->left = mid1->left;
            (*node)->right = mid1->right;
            __avl_calc_hight(*node);
            mid1->key.ptr = NULL;
            mid1->page = NULL;
            free(mid1);
        }
    }
    __avl_calc_hight(*node);
    __avl_rebalance(node);
    return 1;
}

//local metohdes
void __avl_erase(avlnode_ptr node) {
    if (!node) return;
    if (node->left) {
        __avl_erase(node->left);
    }
    if (node->right) {
        __avl_erase(node->right);
    }
    //delete key
    //delete page
    //???????????
    free(node);
}
int32_t __avl_get_hight(avlnode_ptr node) {
    return (node) ? node->hight : 0;
}
void __avl_calc_hight(avlnode_ptr node) {
    if (node) {
        node->hight = max_32t(__avl_get_hight(node->left), __avl_get_hight(node->right)) + 1;
    }
}
int32_t __avl_calc_balance(avlnode_ptr node) {
    if (node) {
        return __avl_get_hight(node->right) - __avl_get_hight(node->left);
    }
    return 0;
}
void __avl_right_move(avlnode_ptr *node) {
    avlnode_ptr mid = (*node)->left;
    (*node)->left = mid->right;
    if (mid->right) {
        mid->right->parent = (*node);
    }
    mid->parent = (*node)->parent;
    (*node)->parent = mid;
    mid->right = (*node);
    (*node) = mid;
    __avl_calc_hight((*node)->right);
    __avl_calc_hight(*node);
}
void __avl_left_move(avlnode_ptr *node) {
    avlnode_ptr mid = (*node)->right;
    (*node)->right = mid->left;
    if (mid->left) {
        mid->left->parent = (*node);
    }
    mid->parent = (*node)->parent;
    (*node)->parent = mid;
    mid->left = (*node);
    (*node) = mid;
    __avl_calc_hight((*node)->left);
    __avl_calc_hight((*node));
}
void __avl_LR_move(avlnode_ptr *node) {
    __avl_left_move(&(*node)->left);
    __avl_right_move(node);
}
void __avl_RL_move(avlnode_ptr *node) {
    __avl_right_move(&(*node)->right);
    __avl_left_move(node);
}
void __avl_rebalance(avlnode_ptr *node) {
    if ((__avl_calc_balance(*node) == -2) && (__avl_calc_balance((*node)->left) == -1)) {
        __avl_right_move(node);
    } else if ((__avl_calc_balance(*node) == 2) && (__avl_calc_balance((*node)->right) == 1)) {
        __avl_left_move(node);
    } else if ((__avl_calc_balance(*node) == -2) && (__avl_calc_balance((*node)->left) == 1)) {
        __avl_LR_move(node);
    } else if ((__avl_calc_balance(*node) == 2) && (__avl_calc_balance((*node)->left) == -1)) {
        __avl_RL_move(node);
    }
}