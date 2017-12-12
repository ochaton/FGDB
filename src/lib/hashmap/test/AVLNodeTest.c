// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "AVLNode.h"

int32_t avl_test_print(avlnode_ptr node, int32_t dep) {
    if (!node) {
        return 0;
    }
    if (node->left) {
        avl_test_print(node->left,dep + 1);
    }
    if (node->right) {
        avl_test_print(node->right,dep +1 );
    }
    printf("%d\n", node->key.size);
    if (node->parent) {
        printf("the paren is %d\n",node->parent->key.size);
    }
    return 1;
}

avlnode_ptr avl_test_search(avlnode_ptr node, str_t key){
    if (!node) {
        return NULL;
    }

    if (node->key.size == key.size) {
        return node;
    } else {
        if (node->key.size < key.size) {
            return avl_test_search(node->right, key);
        } else {
            return avl_test_search(node->left, key);
        }
    }
}

int32_t avl_test_remove_node(avlnode_ptr *node, avlnode_ptr node_new) {

    if (!(*node)) {
        printf("NO\n");
        return -1;
    }
    printf("top is %d\n",(*node)->key.size);
    if ((*node)->key.size < node_new->key.size) {
        return avl_test_remove_node(&(*node)->right, node_new);
    } else if ((*node)->key.size > node_new->key.size ) {
        return avl_test_remove_node(&(*node)->left, node_new);
    } else {
        if (!(*node)->left && !(*node)->right) {
            printf("No child\n");

            avlnode_ptr last = (*node)->parent;
            if (last && (last->left == *node)) {
                last->left = NULL;
                printf("left\n");
            } else if (last) {
                last->right = NULL;
                printf("right\n");
            }
            avl_calc_hight(last);
            //delete key
            //delete page
            //???????????
            free((*node));

        } else if ((*node)->left && (*node)->right) {
            printf("All child\n");
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
            printf("left child\n");
            avlnode_ptr mid1 = (*node)->left;
            //delete key
            //delete page
            //???????????
            (*node)->key = mid1->key;
            (*node)->page = mid1->page;
            (*node)->left = mid1->left;
            (*node)->right = mid1->right;
            avl_calc_hight(*node);
            mid1->key.ptr = NULL;
            mid1->page = NULL;
            free(mid1);
        } else if ((*node)->right) {
            printf("right child\n");
            avlnode_ptr mid1 = (*node)->right;
            //delete key
            //delete page
            //???????????
            (*node)->key = mid1->key;
            (*node)->page = mid1->page;
            (*node)->left = mid1->left;
            (*node)->right = mid1->right;
            avl_calc_hight(*node);
            mid1->key.ptr = NULL;
            mid1->page = NULL;
            free(mid1);
        }
    }
    avl_calc_hight(*node);
    avl_rebalance(node);
    return 1;
}
