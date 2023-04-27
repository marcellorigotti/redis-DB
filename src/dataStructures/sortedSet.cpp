#include "sortedSet.h"
#include <vector>
#include <typeinfo>


SortedSet::SortedSet(){
    tree = new AvlTree();
    hmap = new HashTable();
    hmap->insert("a", 0); //Default node for AvlTree, modify it to create an empty tree
}

SortedSet::SortedSet(uint32_t score, std::string name){
    tree = new AvlTree(score, name);
    hmap = new HashTable();
    hmap->insert(name, score);
}

void SortedSet::update(uint32_t score, std::string name){
    if(tree->del(score, name)){
        tree->add(score, name);
        (*hmap->get(name))->val = score;
    }
    return;
}
void SortedSet::add(uint32_t score, std::string name){
    if(hmap->has(name)){ //hashtable lookup to see if we already have an entry with that name
        if((*hmap->get(name))->val == score) //score is the same, no need to do anything
            return;
        update(score, name);
        return;
    }
    tree->add(score, name);
    hmap->insert(name, score);

    return;
}

bool SortedSet::del(std::string name){
    if(hmap->has(name)){
        uint32_t score = (*hmap->get(name))->val;
        tree->del(score, name);
        hmap->del(name);
        return true;
    }
    return false;
}

bool SortedSet::has(std::string name){
    return hmap->has(name);
}

Node** SortedSet::get(std::string name){
    return hmap->get(name);
}

std::vector<std::string> SortedSet::keys(){
    return hmap->keys();
}

uint32_t SortedSet::cnt(){
    return tree->root->cnt;
}


AvlNode* SortedSet::query(uint32_t score, std::string name, int64_t offset){
    AvlNode* current = tree->root;
    AvlNode* candidate = NULL;
    while (current){
        if(current->val < score || (current->val == score && current->name < name)){
            current = current->right;
        }else{
            candidate = current;
            current = current->left;
        }
    }
    if(candidate){
        candidate = offsetNode(candidate, offset);
    }
    return candidate ? candidate : NULL;
}

AvlNode* SortedSet::offsetNode(AvlNode* node, int64_t offset){
    int64_t pos = 0;
    while(pos != offset){
        if(pos < offset && pos + AvlTree::avl_cnt(node->right) >= offset){
            node = node->right;
            pos += AvlTree::avl_cnt(node->left) + 1;
        }else 
        if(pos > offset && pos - AvlTree::avl_cnt(node->left) <= offset){
            node = node->left;
            pos -= AvlTree::avl_cnt(node->right) + 1;
        }
        else{
            AvlNode* parent = node->parent;
            if(!parent)
                return NULL;
            if(parent->right == node)
                pos -= AvlTree::avl_cnt(node->left) + 1;
            else
                pos += AvlTree::avl_cnt(node->right) + 1;
            node = parent;
        }
    }
    return node;
}