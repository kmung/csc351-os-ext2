#include <cstdio>
#include <cstdlib>
#include <cstring>
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
    string devicePath = "C:/Users/ssyak/Downloads/disk.vhd";

    // Create a filesystem object
    createDisk(devicePath);

    std::fstream disk(devicePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!disk) {
        std::cerr << "Failed to open device: " << devicePath << std::endl;
    }

    SuperBlock sb;
    readSuperBlock(disk, sb);

    Inode inode;
    readInode(disk, 0, inode);

    std::vector<dentry> entries;
    readDentry(disk, entries);

    // // Print the dentries
    // for (const auto& dentry : entries) {
    //     std::cout << "Filename: " << dentry.fname << std::endl;
    //     std::cout << "Inode: " << dentry.inode << std::endl;
    // }

    closeDisk(disk);

    return 0;
}