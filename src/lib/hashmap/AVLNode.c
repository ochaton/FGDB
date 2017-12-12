// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "AVLNode.h"

// prototypes of local methods
static int32_t __avl_get_hight(avlnode_ptr node);
static void __avl_right_move(avlnode_ptr *node);
static void __avl_left_move(avlnode_ptr *node);
static void __avl_LR_move(avlnode_ptr *node);
static void __avl_RL_move(avlnode_ptr *node);

// methods
/*void avl_test_tree_out(avlnode_ptr go) {
    if (!go) {
        return;
    }
    int mid = 0;
    if (!go->parent) {
        mid++;
    }
    avl_test_tree_out(go->left);
    avl_test_tree_out(go->right);
    printf("AVL TEST %s\n", go->key.ptr);
    if (mid) {
        printf("End of tree\n\n");
    }
}*/
int32_t avl_new_node(avlnode_ptr *new_node, str_t key, void *page) {
    /*(*new_node) = (avlnode*) malloc(sizeof(avlnode));
    if (!(*new_node)) {
        return -1;
    }
    (*new_node)->hight = 1;
    (*new_node)->key = key;
    (*new_node)->page = page;
    (*new_node)->left = (*new_node)->right =(*new_node)->parent = NULL;*/
    avlnode_ptr mid_node = (avlnode *) malloc(sizeof(avlnode));
    if (!mid_node) {
        return -1;
    }
    mid_node->hight = 1;
    mid_node->key = key;
    mid_node->page = page;
    mid_node->left = mid_node->right = mid_node->parent = NULL;
    *new_node = mid_node;

    return 1;
}

avlnode_ptr avl_search(avlnode_ptr node, str_t key) {
    if (!node) {
        return NULL;
    }
    //printf("HellO  %s\n",node->key.ptr );
    //printf("HellO  %s\n\n",key.ptr );
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
int32_t avl_insert_node(avlnode_ptr node, avlnode_ptr node_new) {
    if (key_comp(node->key, node_new->key) > 0) {
        if (node->right) {
            avl_insert_node(node->right, node_new);
        } else {
            node->right = node_new;
        }
        if (node->right) {
            node->right->parent = node;
        }
    } else if (key_comp(node->key, node_new->key) < 0) {
        if (node->left) {
            avl_insert_node(node->left, node_new);
        } else {
            node->left = node_new;
        }
        
        if (node->left) {
            node->left->parent = node;
        }
    } else {
        //printf("%s %s \n", node->key.ptr, node_new->key.ptr);
        return -1;
    }
    avl_calc_hight(node);
    avl_rebalance(&node);
    return 1;
}

// here we can delete page
int32_t avl_remove_node(avlnode_ptr *node, avlnode_ptr node_new) {
    //avl_test_tree_out(*node);
    if (!(*node)) {
        return -1;
    }
    if (key_comp((*node)->key, node_new->key) > 0) {
        avlnode_ptr mid_kek = (*node)->right;
        avl_remove_node(&(mid_kek), node_new);
    } else if (key_comp((*node)->key,node_new->key) < 0) {
        avlnode_ptr mid_kek = (*node)->left;
        avl_remove_node(&(mid_kek), node_new);
    } else {
        //printf("Deleye from tree %s\n", (*node)->key.ptr);
        if (!(*node)->left && !(*node)->right) {
            avlnode_ptr last = (*node)->parent;
            if (last && (last->left == *node)) {
                last->left == NULL;
            } else if (last) {
                last->right == NULL;
            }
            avl_calc_hight(last);
            // delete page
            // free((*node)->key);
            (*node)->key.ptr = "err";
            avlnode_ptr mid_kek = *node;
            free(mid_kek);
            (*node) = NULL;
            //free(node);
        } else if ((*node)->left && (*node)->right) {
            avlnode_ptr mid1 = *node, mid2;
            mid1 = mid1-> left;
            while (mid1->right) {
                mid1 = mid1->right;
            }
            // delete page
            // free((*node)->key);
            (*node)->key = mid1->key;
            (*node)->page = mid1->page;
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
                mid1->key.ptr = NULL;
                mid1->page = NULL;
                free(mid1);
                while (mid2 != (*node)) {
                    avl_calc_hight(mid2);
                    avl_rebalance(&mid2);
                    mid2 = mid2->parent;
                }
                avl_calc_hight(*node);
                avl_rebalance(node);
            }
            
        } else if ((*node)->left) {
            avlnode_ptr mid1 = (*node)->left;
            // delete page  
            //free((*node)->key);
            (*node)->key = mid1->key;
            (*node)->page = mid1->page;
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
            mid1->key.ptr = NULL;
            mid1->page = NULL;
            free(mid1);
            
        } else if ((*node)->right) {
            avlnode_ptr mid1 = (*node)->right;
            // delete page
            //free((*node)->key);
            (*node)->key = mid1->key;
            (*node)->page = mid1->page;
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
            mid1->key.ptr = NULL;
            mid1->page = NULL;
            free(mid1);
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
    free(node);
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
