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

// Save iNode with file names
// Save the entry to subdirectory if exists

// Ex)
// Directory Entries in /home/userA:
// - Entry for ".": Inode Number 12345 (refers to /home/userA itself)
// - Entry for "..": Inode Number 56789 (refers to the parent directory of /home/userA)
// - Entry for "file1.txt": Inode Number 23456 (refers to a file named "file1.txt" in /home/userA)
// - Entry for "subdir": Inode Number 78901 (refers to a subdirectory named "subdir" in /home/userA)

// Direcotry Entries in /home/userA/file1.txt
// Entry for "file1.txt": Inode Number 23456



// In our file system, each dentry takes one block.
// As each entry size is 128 (120 of file name, and two integers)
// 32 entries can be allocated on each dentry.
struct dentry{
	int inode;
	char fname[MAX_NAME_LEN];
	int nEntries;
};

#endif