#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H
#include <string>
#include <ctime>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <vector>

using namespace std;

// 2GB memory size
#define MEMORY_SIZE 2ULL * 1024 * 1024 * 1024
// 4KB block size
#define BLOCK_SIZE 4 * 1024
// 128B inode size
#define INODE_SIZE 128
// 128B dentry size
#define DENTRY_SIZE 128
// Total number of blocks
#define NBLOCKS 524288
// Total number of inode blocks
#define NINODE_BLOCKS 1024
// Total number of inodes
#define NINODES 1024 * 32
// Total number of data blocks
#define NDATA_BLOCKS 523264

// Location of bitmaps, first inode, and first data block
#define FIRST_INODE_BITMAP 1
#define FIRST_DATA_BITMAP 2
#define FIRST_INODE 18
#define FIRST_DATA_BLOCK 1042


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

};


#endif