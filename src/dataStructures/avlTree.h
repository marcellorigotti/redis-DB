#include <stdint.h>
#include <string>

struct AvlNode{
    uint32_t depth = 1;
    uint32_t cnt = 1; //used to implement rank-based query!
    uint32_t val = 0; //considered score
    std::string name; //we'll keep a corresponding name for the sortedSet
    AvlNode* left = NULL;
    AvlNode* right = NULL;
    AvlNode* parent = NULL;
};

class AvlTree{
public:
    AvlTree(uint32_t val = 0, std::string name = "a");
    void add(uint32_t val, std::string name);
    bool del(uint32_t val, std::string name);
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