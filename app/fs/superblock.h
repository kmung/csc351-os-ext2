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


// If each inode can address 1034 blocks (10 direct and 1 single indirect pointer which can address 1024 blocks), and you have 32768 inodes,
// then you could theoretically address 32768 * 1034 = 33,882,112 blocks.
// Given a block size of 4KB, this would be 135,528,448 KB or approximately 129,000 MB, which is far larger than your 2GB memory.
// However, it's important to note that not every inode will necessarily use all of its addressable blocks. 
// The number of inodes is based on the maximum number of files or directories you expect to have, and each file or directory will use one inode. 
// Many files will be smaller than the maximum size that an inode can address, so they won't use all of their addressable blocks.
// In practice, it's common to have more addressable blocks than actual blocks in the memory, because this allows the file system to handle a large number of small files efficiently. 
// If you find that you're running out of memory, you might need to reduce the number of inodes or the number of addressable blocks per inode.
// So, having 32768 inodes for a 2GB memory with your current inode structure (10 direct pointers and 1 single indirect pointer) is reasonable.


// Super block locates on the first block of the memory
// It contains the entire information of file system
// Although the size of superblock is 1KB, it still use the entire space of first block
struct SuperBlock {
	uint64_t memory_size = MEMORY_SIZE;
	uint64_t block_size = BLOCK_SIZE;
	uint64_t nBlocks = MEMORY_SIZE / BLOCK_SIZE;
	uint64_t inode_size = INODE_SIZE;
	uint64_t nInodeBlocks = NINODE_BLOCKS;
	uint64_t nInodes = BLOCK_SIZE * NINODE_BLOCKS / INODE_SIZE;
	uint64_t first_data_block = NINODE_BLOCKS + 1;

	// List to keep track of free inode and datablock
	// If data exists in block, return true. Otherwise return false
	bool* inode_freelist;
    bool* block_freelist; 
};


#endif