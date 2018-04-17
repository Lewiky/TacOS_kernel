#include "diskTest.h"

 void main_disk(){
     char* data = "Hello!";
     disk_wr(000000001,(uint8_t*)data ,BLOCK_LEN);
     uint8_t* result[BLOCK_LEN];
     disk_rd(000000001,result,BLOCK_LEN);
     writes(result);
     exit(EXIT_SUCCESS);
}