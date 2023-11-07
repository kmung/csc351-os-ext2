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
    // // Specify the device path
    // string devicePath = "C:/Users/ssyak/Downloads/disk.vhd";

    // // Create a filesystem object
    // createDisk(devicePath);

    // // Variable to use as a disk image file
    // std::fstream disk(devicePath, std::ios::in | std::ios::out | std::ios::binary);
    // if (!disk) {
    //     std::cerr << "Failed to open device: " << devicePath << std::endl;
    // }

    // // Before use contents in the disk, initialize the structures
    // // Then user can access to data in each content ex) sb.blockSize
    // SuperBlock sb;
    // readSuperBlock(disk, sb);

    // Inode inode;
    // readInode(disk, 0, inode);

    // std::vector<dentry> entries;
    // readDentry(disk, entries);

    // closeDisk(disk);

    
    fs filesystem;

    // Create a new file
    int fd = filesystem.my_creat("testfile", 0644);
    if (fd == -1) {
        std::cerr << "Failed to create file\n";
        return 1;
    } else {
        std::cout << "File created with fd " << fd << '\n';
    }

    return 0;
}