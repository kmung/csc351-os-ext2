#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>

#include "disk.h"
#include "superblock.h"
#include "dentry.h"
#include "inode.h"
#include "bitmap.h"

using namespace std;

//******************************************************************************
// ext2 based fs
class fs {
	private:
		fstream disk;
		SuperBlock sb;
		Inode inode;
		vector<dentry> rootEntry;
		bitmap inodeBitmap;
		bitmap blockBitmap;

		// Write new Inode
		void writeInode(fstream& disk, int inum, Inode& inode);

		// Write a single Dentry
		void writeDentry(fstream& disk, const vector<dentry>& entries, int inum);

		// Write a start point of Dentry
		void writeStartDentry(fstream& disk, int inum, int parentInum);

		// Update parent directory entry when creating a new file
		void updateDentry(fstream& disk, string fileName, int& parentInum, int currnetInum);

	public:
		fs();
		~fs();

		

		// refer to https://man7.org/linux/man-pages/man2/syscalls.2.html for more
		int my_creat(const string& name, mode_t mode);

		// int my_read(int fd, char* buffer, int nbytes);
		// int my_write(int fd, const char* buffer, int nbytes);
		// int my_Iseek(int fd, off_t offset, int when);
		// int my_fstat(int fd, struct stat* buf);
		// int my_open(const char *pathname, mode_t mode);
		// int my_close(int fd);
		// int my_stat(const string& name, struct stat& buf);

		int fd;


};

#endif