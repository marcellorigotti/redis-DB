#include "avlTree.h";

AvlTree::AvlTree(uint32_t val){
    AvlNode* node = new AvlNode;
    node->depth = 1;
    node->cnt = 1;
    node->left = node->right = node->parent = NULL;
    root = node;
}
uint32_t AvlTree::avl_depth(AvlNode* node){
    return node ? node->depth : 0;
}

uint32_t AvlTree::avl_cnt(AvlNode* node){
    return node ? node->cnt : 0;
}

void AvlTree::avl_update(AvlNode* node){
    node->cnt = 1 + avl_cnt(node->left) + avl_cnt(node->right);
    node->depth = 1 + std::max(avl_depth(node->right), avl_depth(node->left));
}

AvlNode* AvlTree::rot_left(AvlNode* node){
    AvlNode* new_node = node->right;
    if(new_node->left){
        new_node->left->parent =  node;
    }
    node->right = new_node->left;
    new_node->left = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    avl_update(node);
    avl_update(new_node);
    return new_node;
}
AvlNode* AvlTree::rot_right(AvlNode* node){
    AvlNode* new_node = node->left;
    if(new_node->right){
        new_node->right->parent =  node;
    }
    node->left = new_node->right;
    new_node->right = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    avl_update(node);
    avl_update(new_node);
    return new_node;
}
//before rotating we may need to rotate the subtree so that they are leaning in the correct direction
//to do so we need to rotate the subtree first
AvlNode* AvlTree::avl_fix_left(AvlNode* node){
    if(avl_depth(node->left->left) < avl_depth(node->left->right))
        node->left = rot_left(node->left);
    return rot_right(node);
}
AvlNode* AvlTree::avl_fix_right(AvlNode* node){
    if(avl_depth(node->right->right) < avl_depth(node->right->left))
        node->right = rot_right(node->right);
    return rot_left(node);
}
//used to fix the tree after insertion/deletion
//returns the root node
AvlNode* AvlTree::avl_fix(AvlNode* node){
    while (true){
        avl_update(node);
        uint32_t l_depth = avl_depth(node->left);
        uint32_t r_depth = avl_depth(node->right);
        AvlNode* from = NULL;
        if(node->parent){
            from = (node->parent->left == node) ? node : node->parent->right;
        }
        if(l_depth == r_depth + 2)
            node = avl_fix_left(node);
        if(l_depth + 2 == r_depth)
            node = avl_fix_right(node);
        if(!from)//we are at the root
            return node;
        node = node->parent;
    }
}

AvlNode* AvlTree::avl_del(AvlNode* node){
    if(node->right == NULL){
        AvlNode* parent = node->parent;
        if(node->left)
            node->left->parent = parent;
        if(parent){
            (node == parent->left? parent->left : parent->right) = node->left;
            delete node;
            return avl_fix(parent);
        }else{
            AvlNode* tmp = node->left;
            //delete node;
            return tmp; //we removed the root and no right subtree existed
        }
    }else{
        AvlNode* victim = node->right;
        while (victim->left){
            victim = victim->left;
        }
        AvlNode* root = avl_del(victim);
        *victim = *node;

        AvlNode* parent = node->parent;
        if(parent){
            (node == parent->left ? parent->left : parent->right) = victim;
            return root;
        }else{
            return victim;
        }
    }
}

void AvlTree::add(uint32_t val){
    if(!root)
        AvlTree(val);
    else{
        AvlNode* new_node = new AvlNode;
        new_node->val = val;

        AvlNode* current = root;
        while (true)
        {
            AvlNode** from = (val < current->val) ? &current->left : &current->right;
            if(!from){
                (*from) = new_node;
                new_node->parent = current;
                root = avl_fix(new_node);
                break;
            }
            current = *from;
        }
    }
}

bool AvlTree::del(uint32_t val){
    AvlNode* current = root;
    while(current){
        if(current->val == val)
            break;
        current = val < current->val ? current->left : current->right; 
    }
    if(!current)
        return false;
    root = avl_del(current);
    delete current;
    return true;
}

void AvlTree::avl_verify(AvlNode* parent, AvlNode* node){
    if(!node)
        return;
    assert(node->parent == parent);
    avl_verify(node, node->left);
    avl_verify(node, node->right);

    assert(node->cnt == 1 + avl_cnt(node->left) + avl_cnt(node->right));

    uint32_t l_count = avl_depth(node->left);
    uint32_t r_count = avl_depth(node->right);
    assert(l_count == r_count || l_count == r_count+1 || l_count+1 == r_count);
    assert(node->depth == 1 + std::max(l_count, r_count));
    
    if(node->left){
        assert(node->left->parent == node);
        assert(node->left->val <= node->val);
    }
    if(node->right){
        assert(node->right->parent == node);
        assert(node->right->val >= node->val);
    }
}
