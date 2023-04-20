#include "avlTree.h"
#include "hashtable.h"
#include <iostream>

class SortedSet{
public:
    SortedSet();
    SortedSet(uint32_t score, std::string name);
    void add(uint32_t score, std::string name); //add or update an already existing name
    void del(std::string name);
    std::pair<uint32_t, std::string> query(uint32_t score, std::string name, int64_t offset);

private:
    AvlTree* tree;
    HashTable* hmap;
    void update(uint32_t score, std::string name);
};