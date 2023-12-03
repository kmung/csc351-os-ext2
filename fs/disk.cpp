#include "disk.h"
#include "bitmap.h"

#include <iostream>
#include <fstream>
#include <iostream>
#include <cstring>
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
	// fstream disk;
    // openDisk(devicePath, disk);
    fstream disk(devicePath, ios::in | ios::out);
    if (!disk.is_open()) {
        disk.open(devicePath, ios::out);
        disk.close();
        disk.open(devicePath, ios::in | ios::out);
    }

	// Initialize super block
	initSuperBlock(disk);

	// Initialize all bytes to 0
	char block[block_size] = {0}; 

	// Start from 1, as 0 is used by superblock
	for (size_t i = 1; i < num_blocks; ++i) { 
		disk.write(block, block_size);
	}
    
	// Check for write errors
	if (!disk) {
		cerr << "Error: Could not write to file at " << devicePath << '\n';
		rc = -1;
	} else {
		rc = 0;
	}

	// Initialize bitmaps, first inode, and first data block
	initInodeBitmap(disk);
	initBlockBitmap(disk);
	initFirstInode(disk);
	initRootdentry(disk, 0 , 0);

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

    // Write the superblock to the file
    disk.seekp(0);
	disk.write(reinterpret_cast<char*>(&sb), BLOCK_SIZE);
}

//******************************************************************************
void initInodeBitmap(fstream& disk){
	Bitmap myBitmap(NINODES);

	// Set the first bit to 1 to indicate that 
	// the first inode is in use for the root directory inode
	myBitmap.setBit(0, true);

	// Get the bitmap data
    string bitmapData = myBitmap.getData();

	// Write the bitmap to the file
	disk.seekp(FIRST_INODE_BITMAP * BLOCK_SIZE);
	disk.write(bitmapData.c_str(), bitmapData.size());
}

//******************************************************************************
void initBlockBitmap(fstream& disk){
	Bitmap myBitmap(NDATA_BLOCKS);

	// Set the first bit to 1 to indicate that
	// the first data block is in use for the root directory
	myBitmap.setBit(0, true);

	// Get the bitmap data
    string bitmapData = myBitmap.getData();

	// Write the bitmap to the file
	disk.seekp(FIRST_DATA_BITMAP * BLOCK_SIZE);
	disk.write(bitmapData.c_str(), bitmapData.size());
}


//******************************************************************************
void initFirstInode(fstream& disk) {
	// Initialize the inode
	Inode inode;

	// Set the inode's mode to read, write, and execute for all
	inode.num = 0;
	inode.mode = 00777;
	inode.nlink = 2;
	inode.uid = 1000;
	inode.gid = 1000;
	inode.size = 0;
	inode.atime = time(nullptr);
	inode.mtime = time(nullptr);
	inode.ctime = time(nullptr);
	inode.blockAddress = FIRST_DATA_BLOCK;

	// Write the inode to the file
	disk.seekp(FIRST_INODE * BLOCK_SIZE);
	disk.write(reinterpret_cast<char*>(&inode), INODE_SIZE);
}

//******************************************************************************
void initRootdentry(fstream& disk, int inum, int parentInum) {
    // Initialize the root directory inode
    dentry rootDentry;
    rootDentry.inode = inum; 
	// Set entry name to "."
    strcpy(rootDentry.fname, ".");
	// Set the number of entries to 2 as we have "." and ".."
    rootDentry.nEntries = 2;

    // Write the root directory entry to the disk
    disk.seekp(FIRST_DATA_BLOCK * BLOCK_SIZE, ios_base::beg);
    disk.write(reinterpret_cast<const char*>(&rootDentry), DENTRY_SIZE);

    // Initialize the parent directory inode
    dentry parentDentry;
    parentDentry.inode = parentInum; 
    strncpy(parentDentry.fname, "..", MAX_NAME_LEN);
	parentDentry.nEntries = 2;

    // Write the parent directory entry to the disk
    disk.write(reinterpret_cast<const char*>(&parentDentry), DENTRY_SIZE);
}

//******************************************************************************
void readSuperBlock(fstream& disk, SuperBlock& sb) {
	// Make sure current position is the beginning of the disk
	disk.seekg(0, ios_base::beg);
	// Read the superblock from the disk
    char buffer[BLOCK_SIZE];
	disk.read(buffer, BLOCK_SIZE);

    sb = *reinterpret_cast<SuperBlock*>(buffer);
}

//******************************************************************************
void readInodeBitmap(fstream& disk, Bitmap& inodeBitmap){
    // Calculate the size of the bitmap in bytes
    int bitmapSize = (NINODES + 7) / 8;

    // Resize the bitmap to the correct size
    inodeBitmap.resize(bitmapSize * 8);

    // Read the bitmap from the file
    disk.seekg(FIRST_INODE_BITMAP * BLOCK_SIZE);
    for (int i = 0; i < bitmapSize; ++i) {
        char byte;
        disk.read(&byte, 1);
        for (int j = 0; j < 8; ++j) {
            bool bit = (byte >> j) & 1;
            inodeBitmap.setBit(i * 8 + j, bit);
        }
    }
}

//******************************************************************************
void readBlockBitmap(fstream& disk, Bitmap& blockBitmap){
    // Calculate the size of the bitmap in bytes
    int bitmapSize = (NBLOCKS + 7) / 8;

    // Resize the bitmap to the correct size
    blockBitmap.resize(bitmapSize * 8);

    // Read the bitmap from the file
    disk.seekg(FIRST_DATA_BITMAP * BLOCK_SIZE);
    for (int i = 0; i < bitmapSize; ++i) {
        char byte;
        disk.read(&byte, 1);
        for (int j = 0; j < 8; ++j) {
            bool bit = (byte >> j) & 1;
            blockBitmap.setBit(i * 8 + j, bit);
        }
    }
}

//******************************************************************************
void readInode(fstream& disk, int inum, Inode& inode){
    streampos curPosition = disk.tellg();

    // Check if the disk file is open
    if (!disk.is_open()) {
		cout << "Disk file is not open" << endl;
        throw invalid_argument("Disk file is not open");

    }

    // Seek to the inode
    disk.seekg(FIRST_INODE * BLOCK_SIZE + (inum * INODE_SIZE), ios_base::beg);

    // Check if the seek failed
    if (disk.fail()) {
		cout << "Failed to seek to inode" << endl;
        throw invalid_argument("Failed to seek to inode");
    }

    // Read the inode
    char buffer[INODE_SIZE];
    disk.read(buffer, INODE_SIZE);

    // Check if the read failed
    if (disk.fail()) {
		cout << "Failed to read inode" << endl;
        throw invalid_argument("Failed to read inode");
    }

    // Convert the buffer to an Inode structure
    inode = *reinterpret_cast<Inode*>(buffer);

    disk.seekg(curPosition);
}

//******************************************************************************
void readDentry(fstream& disk, vector<dentry>& entries, int inum){
	// Read the inode
	Inode node;
	readInode(disk, inum, node);

	// Move pointer to where the dentries are stored 
    disk.seekg(node.blockAddress * BLOCK_SIZE, ios_base::beg);

    // Read the first dentry to get the number of entries
	// The reason read the first dentry seperately is beause
	// the number of entries is stored in the first dentry
    char buffer[DENTRY_SIZE];
    disk.read(buffer, DENTRY_SIZE);
    dentry dirEntry = *reinterpret_cast<dentry*>(buffer);
	entries.push_back(dirEntry);

    // Now read the rest of the entries
    for (int i = 1; i < dirEntry.nEntries; ++i) {
        disk.read(buffer, DENTRY_SIZE);
        dentry entry = *reinterpret_cast<dentry*>(buffer);
        entries.push_back(entry);
    }

}


