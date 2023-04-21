#include "avlTree.h"
#include "hashtable.h"
#include <iostream>

class SortedSet{
public:
    SortedSet();
    SortedSet(uint32_t score, std::string name);
    void add(uint32_t score, std::string name); //add or update an already existing name
    bool del(std::string name);
    bool has(std::string name);
    Node** get(std::string name);
    std::vector<std::string> keys();

    //look for a particular node and return the offset node relative to it
    AvlNode* query(uint32_t score, std::string name, int64_t offset);

private:
    AvlTree* tree;
    HashTable* hmap;
    void update(uint32_t score, std::string name);
    AvlNode* offsetNode(AvlNode*, int64_t offset);
};