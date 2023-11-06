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

    fstream disk;
    openDisk(devicePath, disk);

    SuperBlock sb;
    char buffer[sizeof(sb)];
    readBlock(disk, 0, buffer, sizeof(sb)); // Read into the buffer

    memcpy(&sb, buffer, sizeof(sb)); // Copy the data from the buffer to the SuperBlock object

    cout << sb.block_size << endl; // Access the member directly, not through a pointer


    disk.seekg(sb.first_data_block * sb.block_size); // Seek to the position of the directory entry

    dentry dir;
    disk.read(reinterpret_cast<char*>(&dir), sizeof(dir)); // Read the directory entry
    cout << dir.inode << endl;

    disk.close();

    return 0;
}