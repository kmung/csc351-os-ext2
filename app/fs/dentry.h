#ifndef DENTRY_H
#define DENTRY_H
#include <sys/stat.h>
#include <stdlib.h>
#define EXT2_NAME_LEN 256

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

struct dentry{
	int inode;
	size_t entrySize;
	int ftype;	// ex) file or directory
	char fname[EXT2_NAME_LEN];
};

#endif