#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H
#include <string>
#include <ctime>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

using namespace std;

#define MEMORY_SIZE = 2ULL * 1024 * 1024 * 1024 * 8;
#define BLOCK_SIZE = 4 * 1024 * 8;
#define INODE_SIZE = 128 * 8;
#define NINODES = 524288;
#define ADDRESS_SIZE = 4;



// Super block locates on the first block of the memory
// It contains the entire information of file system
// Although the size of superblock is 1KB, it still use the entire space of first block
struct SuperBlock {
	// 2GB Memory Size
	const uint32_t memory_size = 2ULL * 1024 * 1024 * 1024;

	// 4KB block size
	const size_t block_size = 4 * 1024;

	// Total number of disk blocks
	const int nBlocks = memory_size / block_size;

	// Number of Inode blocks, we should decide it
	// I put half of nblocks for now
	const int nInodes = 524288;

	// Size of inode strcuture
	const size_t inode_size = 128;

	


	// List to keep track of free inode and datablock
	// If data exists in block, return true. Otherwise return false
	bool* inode_freelist;
    bool* block_freelist; 
};


// Function to allocate memory to our disk partiton. 
int create_disk();


#endif