#ifndef INODE_H
#define INODE_H

#include <string>
#include <sys/stat.h>
#include <stdio.h>
#include <ctime>

using namespace std;

// special inode numbers
#define FS_BAD_INO 1         // bad blocks inode
#define FS_ROOT_INO 2        // root inode
#define FS_BOOT_LOADER_INO 5 // boot loader inode
#define FS_UNDEL_DIR_INO 6   // undelete directory inode

// structure of inode
struct Inode {
	// file mode
	mode_t i_mode;
	// unique identifier
	int i_num;
	// number of hard links count
	int i_nlink;
	// 16 bits of Owner Uid
	short i_uid;
	// 16 bits of Group Id
	short i_gid;
	// size in bytes
	int i_size;
	// number of blocks allocated
	int i_nblocks;
	// access time
	time_t i_atime;
	// modification time
	time_t i_mtime;
	// creation time
	time_t i_ctime;
	// Address of direct blocks
	// 10 direct blocks and 1 single indirect block
	uint32_t i_blockPointers[11];
};

#endif