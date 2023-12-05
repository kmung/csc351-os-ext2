#ifndef DENTRY_H
#define DENTRY_H
#include <sys/stat.h>
#include <stdlib.h>
// Define the maximum length of file name
#define MAX_NAME_LEN 120

// Define the type of file
#define FILE_TYPE 0
#define DIRECTORY_TYPE 1

// Mark the file as symbolic link (soft link)
#define SYMLINK_TYPE 2


// In our file system, each dentry takes one block.
// As each entry size is 128 (120 of file name, and two integers)
// 32 entries can be allocated on each dentry.
struct dentry{
	int inode;
	char fname[MAX_NAME_LEN];
	int nEntries;
};

#endif