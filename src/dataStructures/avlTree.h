#include <stdint.h>
#include <string>

struct AvlNode{
    uint32_t depth = 0;
    uint32_t cnt = 0; //used to implement rank-based query!
    uint32_t val = 0;
    AvlNode* left = NULL;
    AvlNode* right = NULL;
    AvlNode* parent = NULL;
};

class AvlTree{
public:
    AvlTree(uint32_t val = 0);
    void add(uint32_t val);
    bool del(uint32_t val);
    bool avl_verify(AvlNode* node, AvlNode* parent = NULL);
    void printAvl();
    uint32_t avl_cnt(AvlNode* node);

    AvlNode* root = NULL;
private:
    uint32_t avl_depth(AvlNode* node);
    void avl_update(AvlNode* node);
    AvlNode* rot_left(AvlNode* node);
    AvlNode* rot_right(AvlNode* node);
    AvlNode* avl_fix_left(AvlNode* node);
    AvlNode* avl_fix_right(AvlNode* node);
    AvlNode* avl_fix(AvlNode* node);
    AvlNode* avl_del(AvlNode* node);
};