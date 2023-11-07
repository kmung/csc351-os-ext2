#ifndef INODE_H
#define INODE_H

#include <string>
#include <sys/stat.h>
#include <stdio.h>
#include <ctime>

using namespace std;

// special inode numbers
// #define FS_BAD_INO 1         // bad blocks inode
// #define FS_ROOT_INO 2        // root inode
// #define FS_BOOT_LOADER_INO 5 // boot loader inode
// #define FS_UNDEL_DIR_INO 6   // undelete directory inode

// structure of inode
struct Inode {
	// file mode
	mode_t mode = S_IRUSR;
	// unique identifier
	int num;
	// number of hard links count
	int nlink;
	// 16 bits of Owner Uid
	short uid;
	// 16 bits of Group Id
	short gid;
	// size in bytes
	int size;
	// access time
	time_t atime;
	// modification time
	time_t mtime;
	// creation time
	time_t ctime;
	// Address of direct blocks
	// 10 direct blocks and 1 single indirect block
	uint32_t blockPointers[11];
};

#endif