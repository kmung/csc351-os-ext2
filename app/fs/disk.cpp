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
	SuperBlock sb = initSuperBlock(disk);

	// Write the superblock to the file
	disk.seekp(0);
	disk.write(reinterpret_cast<char*>(&sb), sizeof(sb));

	// Allocate blocks
	char block[block_size] = {0}; // Initialize all bytes to 0
	for (size_t i = 1; i < num_blocks; ++i) { // Start from 1, as 0 is used by superblock
		disk.write(block, block_size);
	}

	// Check for write errors
	if (!disk) {
		std::cerr << "Error: Could not write to file at " << devicePath << '\n';
		rc = -1;
	} else {
		rc = 0;
	}

	// Find a free inode for the root directory
	int rootInodeIndex = 0;

	// Initialize the root directory inode
	Inode rootInode;

	// Set the root directory inode's mode to read, write, and execute for all
	rootInode.num = rootInodeIndex;
	rootInode.mode = 0777;
	rootInode.nlink = 2;
	rootInode.uid = 0;
	rootInode.gid = 0;
	rootInode.size = 0;
	rootInode.atime = time(nullptr);
	rootInode.mtime = time(nullptr);
	rootInode.ctime = time(nullptr);
	rootInode.blockPointers[0] = sb.first_data_block;
	

	// Write the root inode to the file
	disk.seekp(rootInodeIndex * block_size);
	disk.write(reinterpret_cast<char*>(&rootInode), sizeof(rootInode));

	// Initialize the '.' directory entry
	dentry rootDentry;
	rootDentry.inode = rootInodeIndex;
	strcpy(rootDentry.fname, ".");

	// Write the '.' directory entry to the file
	disk.seekp((rootInodeIndex + 1) * block_size);
	disk.write(reinterpret_cast<char*>(&rootDentry), sizeof(rootDentry));

	// Initialize the '..' directory entry
	dentry dotrootDentry;
	dotrootDentry.inode = rootInodeIndex;
	strcpy(dotrootDentry.fname, "..");


	// Write the root directory entry to the file
	disk.seekp((rootInodeIndex + 1) * block_size);
	disk.write(reinterpret_cast<char*>(&rootDentry), sizeof(rootDentry));

	disk.close();

	return rc;
}

void closeDisk(ifstream& diskFile){
    // Close the file
    if (diskFile.is_open()) {
        diskFile.close();
    }
}

void readBlock(fstream& disk, int blockNumber, char* data, int blockSize) {
    // Calculate the position in the file
    streampos position = blockNumber * blockSize;

    // Move the read position to the start of the block
    disk.seekg(position);

    // Read the data from the block
    disk.read(data, blockSize);
}

void readSuperBlock(fstream& disk, SuperBlock& sb) {
    // Read the superblock from the file
    disk.read(reinterpret_cast<char*>(&sb), sizeof(sb));

    // // Read the vectors from the file
    // sb.inode_freelist.resize(sb.nInodes);
    // for (bool& b : sb.inode_freelist) {
    //     disk.read(reinterpret_cast<char*>(&b), sizeof(b));
    // }
    // sb.block_freelist.resize(sb.nBlocks);
    // for (bool& b : sb.block_freelist) {
    //     disk.read(reinterpret_cast<char*>(&b), sizeof(b));
    // }
}

SuperBlock initSuperBlock(fstream& disk) {
    // Initialize super block
    SuperBlock sb;

	sb.memory_size = MEMORY_SIZE;
	sb.block_size = BLOCK_SIZE;
	sb.nBlocks = MEMORY_SIZE / BLOCK_SIZE;
	sb.inode_size = INODE_SIZE;
	sb.nInodeBlocks = NINODE_BLOCKS;
	sb.nInodes = BLOCK_SIZE * NINODE_BLOCKS / INODE_SIZE;
	sb.first_data_block = NINODE_BLOCKS + 1;

	// // Initialize inode_freelist and block_freelist with all false
    // sb.inode_freelist = vector<bool>(sb.nInodes, false);
    // sb.block_freelist = vector<bool>(sb.nBlocks, false);

    // // Set block_freelist[0] to true
    // sb.block_freelist[0] = true;

    // Write the superblock to the file
    disk.write(reinterpret_cast<char*>(&sb), sizeof(sb));

	// for (bool b : sb.inode_freelist) {
    //     disk.write(reinterpret_cast<char*>(&b), sizeof(b));
    // }
    // for (bool b : sb.block_freelist) {
    //     disk.write(reinterpret_cast<char*>(&b), sizeof(b));
    // }


	std::cout << "block_size: " << sb.block_size << std::endl;
    std::cout << "inode_size: " << sb.inode_size << std::endl;
    std::cout << "num_blocks: " << sb.nBlocks << std::endl;
 

	return sb;
}
