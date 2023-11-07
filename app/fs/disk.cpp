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

//******************************************************************************
void openDisk(const string& devicePath, fstream& diskFile){
	// Open the file
    diskFile.open(devicePath, ios::binary | ios::out);
    if (!diskFile.is_open()) {
        cerr << "Error: Could not open file at " << devicePath << '\n';
    } else {
		cout << "Opened file at " << devicePath << '\n';
	}
}

//******************************************************************************
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
	initSuperBlock(disk);

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

	// Initialize the root directory inode
	initInode(disk);

	// Initialize the '.' directory entry
	initRootDentry(disk);

	return rc;
}

//******************************************************************************
void closeDisk(fstream& diskFile){
    // Close the file
    if (diskFile.is_open()) {
        diskFile.close();
    }
}

//******************************************************************************
void readSuperBlock(fstream& disk, SuperBlock& sb) {
	disk.seekg(0, ios_base::beg);
	// Read the superblock from the disk
    char buffer[BLOCK_SIZE];
	disk.read(buffer, BLOCK_SIZE);

    sb = *reinterpret_cast<SuperBlock*>(buffer);
}

//******************************************************************************
void readInode(fstream& disk, int inum, Inode& inode){
	disk.seekg(BLOCK_SIZE + (inum * INODE_SIZE), ios_base::beg);
    char buffer[INODE_SIZE];
	disk.read(buffer, INODE_SIZE);

    inode = *reinterpret_cast<Inode*>(buffer);
}

//******************************************************************************
void readDentry(fstream& disk, std::vector<dentry>& entries){
    disk.seekg(FIRST_DATA_BLOCK * BLOCK_SIZE, ios_base::beg);

    // Read the first dentry to get the number of entries
    char buffer[sizeof(dentry)];
    disk.read(buffer, sizeof(dentry));
    dentry dirEntry = *reinterpret_cast<dentry*>(buffer);

    // Now read the rest of the entries
    for (int i = 0; i < dirEntry.nEntries; ++i) {
        disk.read(buffer, sizeof(dentry));
        dentry entry = *reinterpret_cast<dentry*>(buffer);
        entries.push_back(entry);
    }
}

//******************************************************************************
void initSuperBlock(fstream& disk) {
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
    disk.seekp(0);
	disk.write(reinterpret_cast<char*>(&sb), BLOCK_SIZE);

	// for (bool b : sb.inode_freelist) {
    //     disk.write(reinterpret_cast<char*>(&b), sizeof(b));
    // }
    // for (bool b : sb.block_freelist) {
    //     disk.write(reinterpret_cast<char*>(&b), sizeof(b));
    // } 

}

//******************************************************************************
void initInode(fstream& disk) {
	// Initialize the inode
	Inode inode;

	// Set the inode's mode to read, write, and execute for all
	inode.num = 0;
	inode.mode = 00777;
	inode.nlink = 1;
	inode.uid = 0;
	inode.gid = 0;
	inode.size = 0;
	inode.atime = time(nullptr);
	inode.mtime = time(nullptr);
	inode.ctime = time(nullptr);

	// Initialize blockPointers[] to an invalid block number
    for (int i = 0; i < 11; i++) {
        inode.blockPointers[i] = 0;
    }

	// Write the inode to the file
	disk.seekp(BLOCK_SIZE);
	disk.write(reinterpret_cast<char*>(&inode), sizeof(inode));
}

//******************************************************************************
dentry initRootDentry(fstream& disk) {
    // Initialize the root directory inode
    dentry rootDentry;
    rootDentry.inode = 0; // In the root directory, '.' and '..' point to the same inode
    strcpy(rootDentry.fname, ".");
    rootDentry.nEntries = 2; // Initialize nEntries

    // Initialize the parent directory inode
    dentry parentDentry;
    parentDentry.inode = 0; // In the root directory, '.' and '..' point to the same inode
    strcpy(parentDentry.fname, "..");

    // Write the root directory entry to the disk
    disk.seekp(FIRST_DATA_BLOCK * BLOCK_SIZE, ios_base::beg);
    disk.write(reinterpret_cast<const char*>(&rootDentry), sizeof(dentry));

    // Write the parent directory entry to the disk
    disk.write(reinterpret_cast<const char*>(&parentDentry), sizeof(dentry));

	return rootDentry;
}


