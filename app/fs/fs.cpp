#include "fs.h"

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


int create_disk(){
	// 2GB memory size
	const uint32_t memory_size = 2ULL * 1024 * 1024 * 1024;

	// 4KB block size
	const size_t block_size = 4 * 1024;

	// Calculate the number of blocks
	size_t num_blocks = memory_size / block_size;

	// Allocate 2GB memory
	uint8_t *memory = static_cast<uint8_t*>(malloc(memory_size));

	if (memory == NULL) {
		printf("Memory allocation failed.\n");
	}

	// Create an array to store block addresses
	void* block_addresses[num_blocks];

	// Store each block address
    for (int i = 0; i < num_blocks; i++) {
		block_addresses[i] = reinterpret_cast<uint8_t*>(memory + (i * (block_size + 1)));
    }

	// Initialize super block
	struct SuperBlock* sb = reinterpret_cast<SuperBlock*>(block_addresses[0]);

	// Initialize arrays to keep track of free data block and inode
	sb->block_freelist = new bool[sb->nBlocks];
	sb->inode_freelist = new bool[sb->nInodes];

	// Mark first block to true
	sb->block_freelist[0] = true;

    // Marking all data bloks as free except the first block
    for (int i = 1; i < sb->nBlocks; i++)
        sb->block_freelist[i] = false; 

    // Marking all inodes as free 
    for (int i = 0; i < sb->nInodes; i++)
        sb->inode_freelist[i] = false;


	// Todo list
	// Allocate Super block on the first block
	// Initialize Inode
	// Mark inode of superblock
}



int main(int argc, char **argv) {
	return 0;
}