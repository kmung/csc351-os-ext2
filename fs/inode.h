#ifndef INODE_H
#define INODE_H

#include <string>
#include <sys/stat.h>
#include <stdio.h>
#include <ctime>

using namespace std;

// structure of inode
struct Inode {
	// file mode
	mode_t mode = S_IRUSR;
	// unique identifier
	int num;
	// number of hard links count
	int nlink;
	// 16 bits of Owner Uid
	int uid;
	// 16 bits of Group Id
	int gid;
	// size in bytes
	int size;
	// access time
	time_t atime;
	// modification time
	time_t mtime;
	// creation time
	time_t ctime;
	// Address of direct blocks
	int blockAddress;
};

#endif