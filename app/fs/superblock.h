#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H
#include <string>
#include <ctime>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

using namespace std;

// 2GB memory size
#define MEMORY_SIZE 2ULL * 1024 * 1024 * 1024
// 4KB block size
#define BLOCK_SIZE 4 * 1024
// 128B inode size
#define INODE_SIZE 128
// Total number of inode blocks
#define NINODE_BLOCKS 1024
// Total number of inodes
#define NINODES 1024 * 32

class Superblock {
	private:
		uint64_t memorySize = MEMORY_SIZE;
		uint64_t blockSize = BLOCK_SIZE;
		uint64_t nBlocks = MEMORY_SIZE / BLOCK_SIZE;
		uint64_t inodeSize = INODE_SIZE;
		uint64_t nInodeBlocks = NINODE_BLOCKS;
		uint64_t nInodes = BLOCK_SIZE * NINODE_BLOCKS / INODE_SIZE;
		uint64_t firstDataBlock = NINODE_BLOCKS + 1;
		uint64_t firstFreeInode = 11;

		// List to keep track of free inode blocks and free data blocks
		// Return true if block is free. Otherwise return false.
		bool *inodeFreeList;
		bool *blockFreeList; 
	public:
		Superblock();
		bool allocateInode(int &inodeNum);
};


#endif