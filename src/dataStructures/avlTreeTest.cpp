#include "avlTree.h" 
#include <set>
#include <iostream>

static void extract(AvlNode* node, std::multiset<uint32_t>& extracted){
    if(!node)
        return;
    extract(node->left, extracted);
    extracted.insert(node->val);
    extract(node->right, extracted);
}

static bool verify(AvlTree& tree, std::multiset<uint32_t>& ref){
    if(!tree.avl_verify(tree.root, NULL)){
        std::cout << "Avl verify failed" << std::endl;
        return false;
    }
    if(tree.avl_cnt(tree.root) != ref.size()){
        std::cout << "Avl cnt failed" << std::endl;
        return false;
    }
    std::multiset<uint32_t> extracted;
    extract(tree.root, extracted);
    if(extracted != ref)
        return false;
    return true;
}

int main(){
    AvlTree tree = AvlTree();
    std::multiset<uint32_t> ref;
    ref.insert(0); //to match the item creted with the constructor (creates root node with val = 0)

    //sequential insertion
    for (uint32_t i = 0; i < 10; i ++) {
        tree.add(i);
        ref.insert(i);
        if(!verify(tree, ref)){
            std::cout << "Invalid AVL tree" << std::endl;
            abort();
        }
    }

    // random insertion
    for (uint32_t i = 0; i < 1000; i++) {
        uint32_t val = (uint32_t)rand() % INT32_MAX;
        tree.add(val);
        ref.insert(val);
        if(!verify(tree, ref)){
            std::cout << "Invalid AVL tree" << std::endl;
            abort();
        }
    }

    // random deletion
    for (uint32_t i = 0; i < 500; i++) {
        uint32_t val = (uint32_t)rand() % INT32_MAX;
        auto it = ref.find(val);
        if (it == ref.end()) {
            assert(!tree.del(val));
        } else {
            assert(tree.del(val));
            ref.erase(it);
        }
        if(!verify(tree, ref)){
            std::cout << "Invalid AVL tree" << std::endl;
            abort();
        }
    }

    std::cout<< "Test completed succesfully" << std::endl;
}