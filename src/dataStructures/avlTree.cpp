#include "avlTree.h"
#include <iostream>
#include <assert.h>

AvlTree::AvlTree(uint32_t val, std::string name){
    root = new AvlNode;
    root->name = name;
    root->depth = 1;
    root->cnt = 1;
    root->left = root->right = root->parent = NULL;
}
void printNode(AvlNode* node){
    if(!node)
        return;
    printNode(node->left);
    std::cout << node->val << " " << node->cnt << " " << node->depth << std::endl;
    printNode(node->right);
}
void AvlTree::printAvl(){
    AvlNode* current = root;
    printNode(root);
}
uint32_t AvlTree::avl_depth(AvlNode* node){
    return node ? node->depth : 0;
}

uint32_t AvlTree::avl_cnt(AvlNode* node){
    return node ? node->cnt : 0;
}

void AvlTree::avl_update(AvlNode* node){
    node->cnt = 1 + avl_cnt(node->left) + avl_cnt(node->right);
    node->depth = 1 + std::max(avl_depth(node->right), avl_depth(node->left));
}

AvlNode* AvlTree::rot_left(AvlNode* node){
    AvlNode* new_node = node->right;
    if(new_node->left){
        new_node->left->parent = node;
    }
    node->right = new_node->left;
    new_node->left = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    avl_update(node);
    avl_update(new_node);
    return new_node;
}
AvlNode* AvlTree::rot_right(AvlNode* node){
    AvlNode* new_node = node->left;
    if(new_node->right){
        new_node->right->parent =  node;
    }
    node->left = new_node->right;
    new_node->right = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    avl_update(node);
    avl_update(new_node);
    return new_node;
}
//before rotating we may need to rotate the subtree so that they are leaning in the correct direction
//to do so we need to rotate the subtree first
AvlNode* AvlTree::avl_fix_left(AvlNode* node){
    if(avl_depth(node->left->left) < avl_depth(node->left->right))
        node->left = rot_left(node->left);
    return rot_right(node);
}
AvlNode* AvlTree::avl_fix_right(AvlNode* node){
    if(avl_depth(node->right->right) < avl_depth(node->right->left))
        node->right = rot_right(node->right);
    return rot_left(node);
}
//used to fix the tree after insertion/deletion
//returns the root node
AvlNode* AvlTree::avl_fix(AvlNode* node){
    while (true){
        avl_update(node);
        uint32_t l_depth = avl_depth(node->left);
        uint32_t r_depth = avl_depth(node->right);
        AvlNode** from = NULL;
        if(node->parent){
            from = (node->parent->left == node) ? &node->parent->left : &node->parent->right;
        }
        if(l_depth == r_depth + 2)
            node = avl_fix_left(node);
        if(l_depth + 2 == r_depth)
            node = avl_fix_right(node);
        if(!from)//we are at the root
            return node;
        *from = node;
        node = node->parent;
    }
}

AvlNode* AvlTree::avl_del(AvlNode* node){
    if(node->right == NULL){
        AvlNode* parent = node->parent;
        if(node->left)
            node->left->parent = parent;
        if(parent){
            (node == parent->left ? parent->left : parent->right) = node->left;
            return avl_fix(parent);
        }else{
            return node->left; //we removed the root and no right subtree existed
        }
    }else{ //both subtree are presents
        // swap the node with its next sibling
        AvlNode *victim = node->right;
        while (victim->left) { //choose the smallest possible node that is greater than the one we want to delete
            victim = victim->left; 
        }

        AvlNode* root = avl_del(victim); //remove the chosen node
        //now we have detatched victim and its subtree (if presents)
        //we need to swap this with the original node to delete
        uint32_t victim_val = victim->val;
        std::string victim_name = victim->name;
        *victim = *node;
        if (victim->left) {
            victim->left->parent = victim;
        }
        if (victim->right) {
            victim->right->parent = victim;
        }
        AvlNode* parent = node->parent;
        victim->val = victim_val;
        victim->name = victim_name;
        if (parent) {
            (parent->left == node ? parent->left : parent->right) = victim;
            return root;
        } else {
            return victim;
        }
    }
}

void AvlTree::add(uint32_t val, std::string name){
    AvlNode* new_node = new AvlNode;
    new_node->val = val;
    new_node->name = name;

    AvlNode* current = root;
    while (true)
    {
        //we need to first compare the score and eventually the name
        //if val is less then current then we go left
        //if it is not we check if they are equal
        //if they are not we go right
        //if they are equal we do the same check on the name
        //if the new name comes before then we go left otherwise we go right
        AvlNode** from = (val < current->val) ? &current->left : (val == current->val ? (name < current->name ? &current->left : &current->right) : &current->right);
        if(!*from){
            (*from) = new_node;
            new_node->parent = current;
            root = avl_fix(new_node);
            break;
        }
        current = *from;
    }
}

bool AvlTree::del(uint32_t val, std::string name){
    AvlNode* current = root;
    while(current){
        if(current->val == val && current->name == name)
            break;
        current = val < current->val ? current->left : (val == current->val ? (name < current->name ? current->left : current->right) : current->right); 
    }
    if(!current)
        return false;
    root = avl_del(current);
    delete current;
    return true;
}

bool AvlTree::avl_verify(AvlNode* node, AvlNode* parent){
    if(!node)
        return true;
    if(node->parent != parent){
        std::cout << "parent differ" << std::endl;
        std::cout << node->val << " "<< parent->val << std::endl;
        std::cout << node->parent << " "<< parent->left->val << " " << parent->right->val << std::endl;
        return false;
    }
    if(!avl_verify(node->left, node))
        return false;
    if(!avl_verify(node->right, node))
        return false;

    if(node->cnt != 1 + avl_cnt(node->left) + avl_cnt(node->right)){
        std::cout << "cnt failed" << std::endl;
        return false;
    }

    uint32_t l_count = avl_depth(node->left);
    uint32_t r_count = avl_depth(node->right);
    if(!(l_count == r_count || l_count == r_count+1 || l_count+1 == r_count)){
        std::cout << "depth failed" << std::endl;
        return false;
    }
    if(node->depth != 1 + std::max(l_count, r_count)){
        std::cout << "depth2 failed" << std::endl;
        return false;
    }
    
    if(node->left){
        if(node->left->parent != node){
            std::cout << "left parent failed" << std::endl;
            return false;
        }
        if(node->left->val > node->val){
            std::cout << "ordering val left failed" << std::endl;
            return false;
        }else if(node->left->val == node->val && node->left->name > node->name){
            std::cout << "ordering name left failed" << std::endl;
            return false;
        }
    }
    if(node->right){
        if(node->right->parent != node){
            std::cout << "right parent failed" << std::endl;
            return false;
        }
        if(node->right->val < node->val){
            std::cout << "ordering val right failed" << std::endl;
            return false;
        }else if (node->right->val == node->val && node->right->name < node->name){
            std::cout << "ordering name right failed" << std::endl;
            return false;
        }
    }
    return true;
}
