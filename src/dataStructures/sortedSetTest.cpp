#include "sortedSet.h"
#include <iostream>
#include <assert.h>

int main(){

    SortedSet set = SortedSet(0, "aa");
    for(int i=1; i<=100; i++){
        set.add(i, "aa");
    }

    for(int i=0; i<100; i++){
        AvlNode* res = set.query(0, "aa", i);
        if(res->val != i)
            std::cout << "FAILED" << std::endl;
    }

}