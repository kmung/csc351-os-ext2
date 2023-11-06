#include "disk.h"

#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <stdlib.h>
#include <string.h>

using namespace std;

void openDisk(const string& devicePath, fstream& diskFile){
	// Open the file
    diskFile.open(devicePath, ios::binary | ios::out);
    if (!diskFile.is_open()) {
        cerr << "Error: Could not open file at " << devicePath << '\n';
    }
}

int createDisk(const string& devicePath){
	int rc = -1;

	// 2GB memory size
	const uint32_t memory_size = 2ULL * 1024 * 1024 * 1024;

	// 4KB block size
	const size_t block_size = 4 * 1024;

	// Calculate the number of blocks
	size_t num_blocks = memory_size / block_size;



    // Open the file
	fstream disk;
    openDisk(devicePath, disk);

	// Initialize super block
	// SuperBlock sb;
	SuperBlock* sb = new SuperBlock(memory_size, block_size);

	// Allocate blocks
    char block[block_size] = {0}; // Initialize all bytes to 0
    for (size_t i = 0; i < num_blocks; ++i) {
        disk.write(block, block_size);
    }

    // Check for write errors
    if (!disk) {
        std::cerr << "Error: Could not write to file at " << devicePath << '\n';
        rc = -1;
    } else {
        rc = 0;
    }

	
	// Mark first block to be used by superblock
	sb->block_freelist[0] = true;

	// Write the superblock to the file
	disk.seekp(0);
	disk.write(reinterpret_cast<char*>(sb), sizeof(*sb));

	// Find a free inode for the root directory
	int rootInodeIndex = 1;

	// Initialize the root directory inode
	Inode rootInode;

	// Set the root directory inode's mode to read, write, and execute for all
	rootInode.mode = 0777;

	// Write the root inode to the file
	disk.seekp(rootInodeIndex * block_size);
	disk.write(reinterpret_cast<char*>(&rootInode), sizeof(rootInode));

	// Initialize the root directory entry
	dentry rootDentry;
	rootDentry.inode = rootInodeIndex;
	strcpy(rootDentry.fname, "/");

	// Write the root directory entry to the file
	disk.seekp((rootInodeIndex + 1) * block_size);
	disk.write(reinterpret_cast<char*>(&rootDentry), sizeof(rootDentry));

	disk.close();

	return rc;
}
