#ifndef _FILESYS_H
#define _FILESYS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "disk.h"
#include "libc.h"

#define DIRECTORYTYPE 0
#define FILETYPE 1


typedef struct file {
    char* name; //Name of directory
    uint8_t type; // DIRECTORY or FILE
    uint8_t parent; //Address of parent file
    uint8_t start; //Address of data start
    uint8_t end; //Address of last block of data
    uint8_t length; //Total number of blocks occupied
} file_t;

void initFS();
file_t getRoot();
file_t getFile(uint32_t address);
uint8_t addFile(uint8_t parentAddress, char* name, int type);
int validFile(file_t file);

#endif