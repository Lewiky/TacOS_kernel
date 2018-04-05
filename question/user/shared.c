#include "shared.h"

void main_shared(){
    const char data[] = "some useful stuff";
    pid_t pid = fork();
    if(0 == pid){
        void* mem = shrm(1);
        write(STDOUT_FILENO,mem,sizeof(data));
        shrd(1);
        write(STDOUT_FILENO,"\nHello from Child!\n",19);
    }else{
        void* mem = shrm(1);
        memcpy ((char *)mem, data, sizeof (data));
        shrd(1);
        write(STDOUT_FILENO,"\nHello from Adult!\n",19);
    }
    exit( EXIT_SUCCESS );
}