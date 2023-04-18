#include "avlTree.h" 
#include <set>
#include <iostream>

static void extract(AvlNode* node, std::multiset<std::pair<uint32_t, std::string>>& extracted){
    if(!node)
        return;
    extract(node->left, extracted);
    extracted.emplace(node->val, node->name);
    extract(node->right, extracted);
}

static bool verify(AvlTree& tree, std::multiset<std::pair<uint32_t, std::string>>& ref){
    if(!tree.avl_verify(tree.root, NULL)){
        std::cout << "Avl verify failed" << std::endl;
        return false;
    }
    if(tree.avl_cnt(tree.root) != ref.size()){
        std::cout << "Avl cnt failed" << std::endl;
        return false;
    }
    std::multiset<std::pair<uint32_t, std::string>> extracted;
    extract(tree.root, extracted);
    if(extracted != ref){
        std::cout << "Sets are not the same" << std::endl;
        return false;
    }
    return true;
}

std::string randStr(int ch)
{
    const int ch_MAX = 26; //english alphabet
    char alpha[ch_MAX] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                          'h', 'i', 'j', 'k', 'l', 'm', 'n',
                          'o', 'p', 'q', 'r', 's', 't', 'u',
                          'v', 'w', 'x', 'y', 'z' };
    std::string result = "";
    for (int i = 0; i<ch; i++)
        result = result + alpha[rand() % ch_MAX];

    return result;
}

int main(){
    AvlTree tree = AvlTree();
    std::multiset<std::pair<uint32_t, std::string>> ref;
    ref.emplace(0, "a"); //to match the item creted with the constructor (creates root node with val = 0 and name = "a")

    std::cout << "First insertions" << std::endl;
    //sequential insertion
    for (uint32_t i = 0; i < 10; i ++) {
        std::string tmp = randStr(1);
        tree.add(i, tmp);
        ref.emplace(i,tmp);
        if(!verify(tree, ref)){
            std::cout << "Invalid AVL tree" << std::endl;
            abort();
        }
    }

    std::cout << "Second insertions" << std::endl;
    // random insertion
    for (uint32_t i = 0; i < 1000; i++) {
        uint32_t val = (uint32_t)rand() % 100;
        std::string tmp = randStr(1);
        tree.add(val, tmp);
        ref.emplace(val, tmp);
        if(!verify(tree, ref)){
            std::cout << "Invalid AVL tree" << std::endl;
            abort();
        }
    }

    std::cout << "First deletion" << std::endl;
    // random deletion
    for (uint32_t i = 0; i < 500; i++) {
        uint32_t val = (uint32_t)rand() % 100;
        std::string tmp = randStr(1);
        std::pair<uint32_t, std::string> pairTmp {val, tmp};
        auto it = ref.find(pairTmp);
        if (it == ref.end()) {
            assert(!tree.del(val, tmp));
        } else {
            assert(tree.del(val, tmp));
            ref.erase(it);
        }
        if(!verify(tree, ref)){
            std::cout << "Invalid AVL tree" << std::endl;
            abort();
        }
    }

    std::cout<< "Test completed succesfully" << std::endl;
}