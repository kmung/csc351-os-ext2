#ifndef FS_H
#define FS_H
#include <string>
#include <ctime>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include "superblock.h"
#include "inode.h"
#include "mm.h"
#include "dentry.h"

using namespace std;

#define MEMORY_SIZE = 2ULL * 1024 * 1024 * 1024 * 8;
#define BLOCK_SIZE = 4 * 1024 * 8;
#define INODE_SIZE = 128 * 8;
#define NINODES = 524288;
#define ADDRESS_SIZE = 4;


// Function to allocate memory to our disk partiton. 
int create_disk();


#endif