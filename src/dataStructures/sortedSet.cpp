#include "sortedSet.h"

SortedSet::SortedSet(){
    tree = new AvlTree();
    hmap = new HashTable();
}

SortedSet::SortedSet(uint32_t score, std::string name){
    tree = new AvlTree(score, name);
    hmap = new HashTable();
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

void SortedSet::del(std::string name){
    if(hmap->has(name)){
        uint32_t score = (*hmap->get(name))->val;
        tree->del(score, name);
        hmap->del(name);
    }
    return;
}