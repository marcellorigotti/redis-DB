#include "avlTree.h";

AvlTree::AvlTree(uint32_t val){
    AvlNode* node = new AvlNode;
    node->depth = 1;
    node->cnt = 1;
    node->left = node->rigth = node->parent = NULL;
}

