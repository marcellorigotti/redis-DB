#include <stdint.h>
#include <string>

struct AvlNode{
    uint32_t depth = 0;
    uint32_t cnt = 0;
    uint32_t val = 0;
    Node* left = NULL;
    Node* rigth = NULL;
    Node* parent = NULL;
};

class AvlTree{
public:
    AvlTree(uint32_t val);
private:

};