#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <cmath>
#include <string>
#include "hashtable.h"
#include <vector>

//size = power of 2 so that we can use bitwise & (modulo)
//maybe start with something like 8/16 and eventually resize
//TODO: implement resize
HashTable::HashTable (size_t n): mask(n-1) {
    //n must be a power of 2!
    assert(n > 0 && ((n-1) & n) == 0);
    table = (Node**)calloc(n, sizeof(Node*)); 
    size = 0;
}

void HashTable::insert(std::string key, std::string val){
    size_t index = hash(key); //hash our key
    Node* tmp = new Node(); //create a new node to be added and fill it with key, val and hashval
    tmp->hcode = index;
    tmp->key = key;
    tmp->val = val;
    Node* next = table[index];
    tmp->next = next;
    table[index] = tmp;
    size++;
}
    
bool HashTable::del(std::string key){
    if(!table)
        return false;
    Node** tmp = lookup(key);
    if(!tmp)
        return false;
    Node* toDelete = *tmp;
    *tmp = (*tmp)->next;   
    delete toDelete; //TODO, check correctness of this delete, is this the correct way to deallocate memory?  
    size--;
    return true;
}

Node** HashTable::get(std::string key){
    if(!table)
        return NULL;
    Node** tmp = lookup(key);
    if(!tmp)
        return NULL;
    return tmp;
}

bool HashTable::has(std::string key){
    if(!table)
        return false;
    Node** tmp = lookup(key);
    if(!tmp)
        return false;
    return true;
}

bool HashTable::is_empty(){
    return size == 0;
}

size_t HashTable::getSize(){
    return size;
}

std::vector<std::string> HashTable::keys(){
    std::vector<std::string> keys;
    if(!table)
        return keys;
    for(int i=0; i<mask+1; i++){
        Node* tmp = table[i];
        while(tmp){
            keys.push_back(tmp->key);
            tmp = tmp->next;
        }
    }
    return keys;
}

Node** table = NULL;
size_t size = 0;
size_t mask = 0;

//polynomial rolling, maybe to change for something more efficient
size_t HashTable::hash(std::string key){
    size_t hashcode = 0;
    for(int i=0; i<key.size();i++){
        hashcode += (size_t)(key[i] * pow(PRIME_CONST, i)) & mask;
    }
    return hashcode & mask;
}
Node** HashTable::lookup(std::string key){
    if(!table){
        return NULL;
    }
    size_t index = hash(key);
    Node** tmp = &table[index];//list of nodes colliding
    while(*tmp){
        if((*tmp)->key == key){
            return tmp;
        }
        tmp = &(*tmp)->next;
    }
    return NULL;
}

