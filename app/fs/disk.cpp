#include "disk.h"

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
	int rc = -1;

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

	// Create a vector to store block addresses
	std::vector<void*> block_addresses(num_blocks);

	// Store each block address
	for (int i = 0; i < num_blocks; i++) {
		block_addresses[i] = reinterpret_cast<uint8_t*>(memory + (i * block_size));
	}

	// Initialize super block
	struct SuperBlock* sb = reinterpret_cast<SuperBlock*>(block_addresses[0]);

	// Initialize arrays to keep track of free data block and inode
	sb->block_freelist = new bool[sb->nBlocks];
	sb->inode_freelist = new bool[sb->nInodes];

    // Marking all data bloks as free
    for (int i = 0; i < sb->nBlocks; i++)
        sb->block_freelist[i] = false; 

    // Marking all inodes as free 
    for (int i = 0; i < sb->nInodes; i++)
        sb->inode_freelist[i] = false;

	// Mark first block to be used by superblock
	sb->block_freelist[0] = true;


	// Find a free inode for the root directory
	int rootInodeIndex = -1;
	for (int i = 0; i < sb->nInodes; i++) {
		if (!sb->inode_freelist[i]) {
			rootInodeIndex = i;
			sb->inode_freelist[i] = true;
			break;
		}
	}

	// Check if a free inode was found
	if (rootInodeIndex == -1) {
		// Print error code
	}

	// Initialize the root directory inode
	Inode* rootInode = reinterpret_cast<Inode*>(block_addresses[rootInodeIndex]);
	// Set the root directory inode's mode to read, write, and execute for all
	rootInode->i_mode = 00777;
	
	rootInode->i_num = rootInodeIndex;

	// Set the root directory inode's number of hard links to 2
	// One for the root directory itself (.), and one for the parent directory (..)
	// Both '.' and '..' point to the root directory itself
	rootInode->i_nlink = 2;

	// Set the root directory inode's owner and group to root (0)
	rootInode->i_uid = 0;
	rootInode->i_gid = 0;

	// Theoretically, the root directory inode's size should be 0 when the memory is empty?
	// But we might need to account for the size of the directory entry
	rootInode->i_size = 0;

	// One block is allocated for the root directory entry
	rootInode->i_nblocks = 1;

	// Set the root directory inode's access, modification, and creation times to the current time
	rootInode->i_atime = rootInode->i_mtime = rootInode->i_ctime = time(NULL);

	// Mark the first data block as used
	int rootDirBlockIndex = sb->first_data_block;
	sb->block_freelist[rootDirBlockIndex] = true;

	// Get the address of the block
	char* rootDirBlock = reinterpret_cast<char*>(block_addresses[rootDirBlockIndex]);

	// Update the inode of the root directory
	rootInode->i_blockPointers[0] = rootDirBlockIndex;

	// Cast the block address to a dentry pointer
	dentry* rootDirEntries = reinterpret_cast<dentry*>(rootDirBlock);

	// Initialize the "." entry
	rootDirEntries[0].inode = rootInodeIndex;
	rootDirEntries[0].ftype = DIRECTORY_TYPE;  
	rootDirEntries[0].den_size = sizeof(dentry);
	strcpy(rootDirEntries[0].fname, ".");

	// Initialize the ".." entry
	rootDirEntries[1].inode = rootInodeIndex;
	rootDirEntries[1].ftype = DIRECTORY_TYPE;
	rootDirEntries[1].den_size = sizeof(dentry);
	strcpy(rootDirEntries[1].fname, "..");

	return rc;
}



int main(int argc, char **argv) {
	return 0;
}