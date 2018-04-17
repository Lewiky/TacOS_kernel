#include "filesystem.h"

int diskWriteFileHeader(file_t* file){
    return disk_wr((uint32_t)file->start,(uint8_t*)file,BLOCK_LEN);
}

file_t diskReadFileHeader(uint32_t pointer){
    file_t file = {0};
    disk_rd(pointer,(uint8_t*)&file,BLOCK_LEN);
    return((file_t)file);
}

int validFile(file_t file){
    file_t emptyFile = {0};
    if(strcmp(emptyFile.name,file.name) == 0){
        return false;
    }
    return true;
}

//Find the next region of disk that is unoccupied
uint8_t findSpace(){
    //return 2;
    uint8_t address = 00000000;
    file_t file = diskReadFileHeader(address);
    while(validFile(file)){
        //writes(file.name);
        file = diskReadFileHeader(file.end+1);
        address += file.length;
    }
    return address;
}

uint8_t addFile(uint8_t parentAddress, char* name, int type){
    uint8_t start = findSpace();
    file_t parent = diskReadFileHeader(parentAddress);
    if(parent.type == FILETYPE){
        return -1;
    }
    parent.length++;
    parent.end++;
    file_t file = {.name = name, .type = type, .parent = parentAddress, .start = start, .end = start, .length = 1};
    disk_wr((uint32_t)parent.end,(uint8_t*)&file.start,BLOCK_LEN);
    diskWriteFileHeader(&parent);
    diskWriteFileHeader(&file);
    return file.start;
}

file_t getFile(uint32_t address){
    return diskReadFileHeader(address);
}

file_t getRoot(){
    return diskReadFileHeader(00000000);
}

//Init the file system, create the root node (and some test files)
void initFS(){
    file_t root = {.name = ".",.type = DIRECTORYTYPE, .parent = 00000000, .start =00000000, .end = 00000000, .length = 1 };
    diskWriteFileHeader(&root);
    uint8_t fileID = addFile(root.start,"test_file",FILETYPE);
    diskReadFileHeader(fileID);
    file_t file = diskReadFileHeader(fileID);
    //writes(file.name);
    // uint8_t dirID = addFile(root.start,"test_dir",DIRECTORYTYPE);
    // file_t dir = diskReadFileHeader(dirID);
    // uint8_t subFileID = addFile(dir.start,"subFile",FILETYPE);
}