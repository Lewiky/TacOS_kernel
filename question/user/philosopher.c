#include "philosopher.h"

pid_t philosophers[16];
char* names[16] = {"Ringo","George","Paul","John","Han","Luke","Chewy","Frodo","Sam","Pippin","Merry","Aragorn","Legolas","Gandalf","Gimli","Boromir"};
const char e[] = "E";
const char t[] = "T";

//Do nothing, running the nop instruction does nothing, but won't get optimised out
void sleep(int seconds){
    // for(int i = 0; i< seconds*0x20000000;i++){
    //     asm volatile("nop");
    // }
    yield();
}

void think(int num){
    //writes(names[num]);
    // void* mem = shrm(num+16);
    // memcpy ((char *)mem, t, sizeof(t));
    // shrd(num+16);
    //writes(" IS THINKING\n");
    sleep(1);
}

void eat(int num, int count){
    void* left = shrm(num);
    void* right = shrm((num+1)%16);
    writes(names[num]);
    writes(" IS EATING ");
    char* eatCount;
    itoa(eatCount,count);
    writes(eatCount);
    writes("\n");
    // void* mem = shrm(num+16);
    // memcpy ((char *)mem, e, sizeof(e));
    // shrd(num+16);
    sleep(2);
    shrd(num);
    shrd((num+1)%16);
}

void bePhili(int num){
    int count = 1;
    while(1){
        think(num);
        eat(num, count);
        count++;
    }
}

void watcher(){
    while(1){
        writes("\n");
        for(int i = 16; i < 32; i++){
            void* mem = shrm(i);
            write(STDOUT_FILENO,mem,1);
            shrd(i);
        }
        writes("\n");
        sleep(5);
    }
}

void main_philosopher(){
    for(int i = 0; i <16; i++){
        pid_t pid = fork();
        if(pid == 0){
            bePhili(i);
        }else{
            philosophers[i] = pid;
            writes(names[i]);
            writes(" is born \n");
            //nice(1,10);
            //watcher();
        }
    }
    exit(EXIT_SUCCESS);
}