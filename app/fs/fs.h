#ifndef FS_H
#define FS_H
#include <string>
#include <ctime>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>


#include "bitmap.h"
#include "mm.h"
#include "memory.h"

using namespace std;

#define MEMORY_SIZE = 2ULL * 1024 * 1024 * 1024;
#define BLOCK_SIZE = 4 * 1024;
#define INODE_SIZE = 128;
#define NINODES = 524288;



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





// Group descriptor contains information of each block group ex)superblock, bitmap
// It should locate next to super block
// But I am not sure we are using it
struct GroupDesc {
	// Describe where bitmaps are located
	uint32_t block_bitmap;
	uint32_t inode_bitmap;

	// Describe first index of inode_table
	uint32_t inode_table;

	// Count free blocks and inodes
	int nFreeBlocks;
	int nFreeInodes;	
};


// Each inode points certain data block
// Inode contains information about data block
struct Inode {
	// File mode and access permission
	uint16_t f_mode;

	// Number of hard links to the file
	uint16_t Nlink;

	// File owner id
	uint16_t Uid;

	// Group the file belongs to
	uint16_t Gid;

	// File size in bytes
	uint32_t FileSize;

	// Creation time
	time_t Ctime;

	// Time of last access
	time_t Atime;

	// Time of last modification
	time_t Mtime;

	// Include the index where file locates within data blocks
	// If we are not making a huge file system, we can just use direct blocks
	uint32_t DirectBlocks[12];
};




// I created the array to keep track on data block and inodes
// I am not sure we still need to use bitmap

// Each bitmap is one block long
// Explanation about bitmap is included on bitmap.h file
struct BlockBitmap {
	const size_t bitmapSize = 1 * 1024 * 1024;
	BlockBitmap();
};




struct InodeBitmap{
	const size_t bitmapSize = 1 * 1024 * 1024;
	InodeBitmap();
};




// Save iNode with file name
// Save the entry to subdirectory if exists

// Ex)
// Directory Entries in /home/userA:
// - Entry for ".": Inode Number 12345 (refers to /home/userA itself)
// - Entry for "..": Inode Number 56789 (refers to the parent directory of /home/userA)
// - Entry for "file1.txt": Inode Number 23456 (refers to a file named "file1.txt" in /home/userA)
// - Entry for "subdir": Inode Number 78901 (refers to a subdirectory named "subdir" in /home/userA)

// Direcotry Entries in /home/userA/file1.txt
// Entry for "file1.txt": Inode Number 23456

// Define max length of name
#define EXT2_NAME_LEN 256

struct dentry{
	int inode;
	size_t entrySize;
	int ftype;	// ex) file or directory
	char fname[EXT2_NAME_LEN];
};


// Functiont to allocate memory to our disk partiton. 
int create_disk();



class fs {
	private:
		mode_t fileType : 3;
		unsigned short OwnerPermissions : 3;
		unsigned short GroupPermissions : 3;
		unsigned short OthersPermissions : 3;
		unsigned short SpecialPermissions : 3;
};





#endif