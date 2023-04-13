#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <cmath>
#include <string>

struct Node{
    Node* next = NULL;
    uint64_t hcode = 0;
    std::string key;
    std::string val;
};

//size = power of 2 so that we can use bitwise & (modulo)
//maybe start with something like 8/16 and eventually resize
//TODO: implement resize
class HashTable {
public:
    HashTable(size_t n = 8) {
        //n must be a power of 2!
        assert(n > 0 && ((n-1) & n) == 0);
        table = (Node**)calloc(n, sizeof(Node*)); 
        mask = n-1;
        size = 0;
    }

    void insert(std::string key, std::string val){
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
    
    bool del(std::string key){
        if(!table)
            return false;
        Node** tmp = lookup(key);
        if(!tmp)
            return false;    
        *tmp = (*tmp)->next;     
        size--;
        return true;
    }
    
    Node** get(std::string key){
        if(!table)
            return NULL;
        Node** tmp = lookup(key);
        if(!tmp)
            return NULL;
        return tmp;
    }

    bool has(std::string key){
        if(!table)
            return false;
        Node** tmp = lookup(key);
        if(!tmp)
            return false;
        return true;
    }

    bool is_empty(){
        return size == 0;
    }

    size_t getSize(){
        return size;
    }

private:
    Node** table = NULL;
    size_t size = 0;
    size_t mask = 0;
    const int PRIME_CONST = 31;

    //polynomial rolling, maybe to change for something more efficient
    size_t hash(std::string key){
        size_t hashcode = 0;
        for(int i=0; i<key.size();i++){
            hashcode += (size_t)(key[i] * pow(PRIME_CONST, i)) & mask;
        }
        return hashcode & mask;
    }
    Node** lookup(std::string key){
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
};
