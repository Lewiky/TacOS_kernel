#include "tree.h"

void recursionTree(int process, int depth, int bars){
    int numKids = numChild(process);
    if(numKids == 0){
        return;
    }
    for(int i = 0; i < numKids; i++){
        uint8_t childAddress = getChildAddress(process,i);
        for(int j = 0; j < depth; j++){
            bars?writes("│   "):writes("    ");
        }
        writes("│\n");
        for(int j = 0; j < depth; j++){
            bars?writes("│   "):writes("    ");
        }
        if(i == numKids-1){
            writes("└─ ");
        }else{
           writes("├─ "); 
        }
        writes(getName(childAddress));
        writes("\n");
        if(numChild(childAddress) > 0){
            depth++;
            recursionTree(childAddress,depth,(i != numKids-1));
            depth--;
        }
    }
}

void main_tree(){
    writes("\n");
    writes("--Tree--\n");
    writes(getName(0));
    writes("\n");
    recursionTree(00000000, 0, 0);
    exit(EXIT_SUCCESS);
}