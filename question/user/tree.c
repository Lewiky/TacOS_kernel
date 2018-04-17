#include "tree.h"

void recursionTree(int process){
    int numKids = numChild(process);
    //writes(toString(numKids));
    if(numKids == 0){
        return;
    }
    for(int i = 0; i <numKids; i++){
        writes("|\n");
        writes("--");
        writes(getChildName(process,i));
        writes("\n");
        recursionTree(getChildAddress(process,i));
    }
}

void main_tree(){
    writes("\n");
    writes("--Tree--\n");
    writes(".\n");
    recursionTree(00000000);
    exit(EXIT_SUCCESS);
}