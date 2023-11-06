#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>


#include "disk.h"
#include "superblock.h"
#include "inode.h"
#include "dataBlock.h"
#include "dentry.h"
#include "filesystem.h"


using namespace std;

int main(int argc, char **argv) {
    
    // Specify the device path
    string devicePath = "C:/Users/ssyak/OneDrive/Desktop/class/2023fall/CSC351/disk.vhd";

    createDisk(devicePath);

    return 0;
}