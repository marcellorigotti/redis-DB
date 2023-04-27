#include "sortedSet.h"
#include <iostream>
#include <assert.h>

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

    SortedSet set = SortedSet(0, "a");
    for(int i=1; i<=100; i++){
        set.add(i, randStr(10));
    }
    std::cout << set.cnt() << std::endl;

    for(int i=0; i<100; i++){
        AvlNode* res = set.query(0, "a", i);
        if(!res){
            std::cout << "Node not present" << std::endl;
            continue;
        }
        else if(res->val != i)
            std::cout << "FAILED" << std::endl;
        
        for(int j=0; j<100; j++){
            int offset = j - i;
            AvlNode* res2 = set.query(res->val, res->name, offset);
            if(!res2){
                std::cout << "Node not present2" << std::endl;
                continue;
            }
            else if(res2->val != j)
                std::cout << "FAILED2" << std::endl;
        }
    }

    std::cout << "Test completed succesfully!" << std::endl;

}