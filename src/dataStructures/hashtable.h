#include <stdint.h>
#include <string>

const int PRIME_CONST = 31;

struct Node{
    Node* next = NULL;
    uint64_t hcode = 0;
    std::string key;
    std::string val;
};

class HashTable {
public:
    HashTable(size_t n = 16);

    void insert(std::string key, std::string val);
    bool del(std::string key);
    Node** get(std::string key);
    bool has(std::string key);
    bool is_empty();
    size_t getSize();

private:
    Node** table = NULL;
    size_t size = 0;
    size_t mask;
    const int PRIME_CONST = 31;

    //polynomial rolling, maybe to change for something more efficient
    size_t hash(std::string key);
    Node** lookup(std::string key);
};

