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




// Function to allocate memory to our disk partiton. 
int create_disk();


#endif