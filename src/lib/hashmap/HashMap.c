// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "HashMap.h"
/*void hash_test_tree_print(avlnode_ptr go) {
    if (!go) {
        return;
    } 
    int test_mid = 0;
    if (!go->parent) {
        test_mid++;
    } 
    hash_test_tree_print(go->left);
    hash_test_tree_print(go->right);
    printf("Node is %s\n",go->key.ptr);
    if (test_mid) {
        printf("End of  tree \n\n");
    }
}*/
int32_t hash_new_node(hm_node_ptr *node, uint32_t dep) {
    //create new hash table
    if (!node) {
        return -1;
    }
    hm_node_ptr new_node = (hm_node*) malloc(sizeof(hm_node));
    if (!new_node) {
        return -1;
    }
    for (int32_t i = 0; i < MAX_HASH_NODE; i++) {
        new_node->top[i] = NULL;
        new_node->len_of_list[i] = 0;
    }
    new_node->dep = dep;
    *node = new_node;
    return 0;
}

avlnode_ptr hash_search(hm_node_ptr node, str_t key) {
    if (!node) {
        return NULL;
    }
    int32_t mid_key = hash(key, node->dep);
    while (1) {
        if (node->len_of_list[mid_key] < MAX_HASH_DEP) {
            return avl_search(node->top[mid_key], key);
        } else {
            node = node->top[mid_key];
            mid_key = hash(key, node->dep);
        }
    }
}

int32_t hash_insert(hm_node_ptr node, str_t key, void *page) {
    if (!node) {
        return -1;
    }

    int32_t mid_key = hash(key, node->dep);
    //printf("mid_key %d\n",mid_key);
    while (node->len_of_list[mid_key] > MAX_HASH_DEP) {
        
        node = node->top[mid_key];
        mid_key = hash(key, node->dep);
    }
    
    node->len_of_list[mid_key]++;
    
    avlnode_ptr new_avl_node;
    avl_new_node(&new_avl_node, key, page);
    
    if (!node->top[mid_key]) {
        node->top[mid_key] = new_avl_node;
    } else {
        avlnode_ptr midd = node->top[mid_key];
        int32_t ans = avl_insert_node(midd, new_avl_node);
        if (ans == -1) {
            return -1;
        }
    }
    
    __hash_remake(node, mid_key);
    return 1;
}

int32_t hash_delete(hm_node_ptr node, str_t key) {
    if (!node) {
        return 0;
    }

    int32_t mid_key = hash(key, node->dep);
    while (1) {
        
        if (node->len_of_list[mid_key] < MAX_HASH_DEP) {
            //hash_test_tree_print(node->top[mid_key]);
            avlnode_ptr new_avl_node;
            avl_new_node(&new_avl_node, key, NULL);
            avlnode_ptr mid_node = node->top[mid_key];
            avl_remove_node(&mid_node, new_avl_node);
            if (new_avl_node) {
                free(new_avl_node);
            }
            //if (node->top[mid_key]) {
            //    hash_test_tree_print(node->top[mid_key]);
            //}
            node->top[mid_key] = mid_node;
            node->len_of_list[mid_key]--;
            break;
        } else {
            node = node->top[mid_key];
            mid_key = hash(key, node->dep);
        }
    }
    //hash_test_tree_print(node->top[mid_key]);
    return 1;
}

int32_t hash_erase(hm_node_ptr node) {
    if (!node) {
        return 0;
    }
    for (uint32_t i = 0; i < MAX_HASH_NODE; i++) {
        if (node->len_of_list[i] > MAX_HASH_DEP) {
            hash_erase(node->top[i]);
        } else {
            if (node->len_of_list[i] > 0) {
                avl_erase(node->top[i]);
            }
            
        }
    }
    free(node);
    return 1;
}

int32_t __hash_remake(hm_node_ptr node, uint32_t index) {
    //if == then it is flag that make new hash, if > that it is hash
    if (node->len_of_list[index] != MAX_HASH_DEP) {
        return 0;
    }
    hm_node_ptr new_node;
    hash_new_node(&new_node, node->dep+1);
    avlnode_ptr go = node->top[index];
    __hash_insert_avl_in_hash(new_node, go);
    node->top[index] = new_node;
    node->len_of_list[index]++;
    return 1;
}

int32_t __hash_insert_avl_in_hash(hm_node_ptr node, avlnode_ptr go) {
    if (!go) {
        return 0;
    }
    if (go->left) {
        __hash_insert_avl_in_hash(node, go->left);
    }
    if (go->right) {
        __hash_insert_avl_in_hash(node, go->right);
    }
    /*if (go->parent) {
        if (go->parent->left == go) {
            go->parent->left = NULL;
        } else {
            go->parent->right = NULL;
        }
    }*/
    go->left = go->right = go->parent = NULL;
    go->hight = 1;
    //avl_calc_hight(go->parent);
    //avl_rebalance(&go->parent);
    //go->parent = NULL;
    //avl_calc_hight(go);
    //avl_rebalance(&go);
    int32_t mid_key = hash(go->key, node->dep);
    node->len_of_list[mid_key]++;
    if (!node->top[mid_key]) {
        node->top[mid_key] = go;
    } else {
        avlnode_ptr midd = node->top[mid_key];
        avl_insert_node(midd, go);
    }
    
    return 1;
}
