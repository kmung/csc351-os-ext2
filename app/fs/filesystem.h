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

		// Store fd of opened files
		vector<int> openFdTable;

		// Write new Inode
		void writeInode(fstream& disk, int inum, Inode& inode);

		// Write a single entry on current dentry vector
		void writeDentry(fstream& disk, const vector<dentry>& entries, int inum);

		// Write a new dentry that contains '.' and '..' entries
		void writeStartDentry(fstream& disk, int inum, int parentInum);

		// Check all parents in fileName exist
		// If exist, save last parent's dentry vector and inum
		// Otherwise, save -1 to parentInum
		void findParent(fstream& disk, string fileName, vector<dentry>& parentDentries, int& parentInum);

		// Update parent's dentry vector to include new dentry
		void updateParentDentry(fstream& disk, string fileName, int inum, vector<dentry> parentDentries, int parentInum);

	public:
		fs();
		~fs();

		// refer to https://man7.org/linux/man-pages/man2/syscalls.2.html for more
		int my_creat(const string& name, mode_t mode);
		int my_open(const char *pathname, mode_t mode);
		bool my_close(int fd);
		bool my_stat(const string& name, struct stat& buf);
		bool my_fstat(int fd, struct stat& buf);
		int my_lseek(int fd, off_t offset, int whenence);

		// int my_read(int fd, char* buffer, int nbytes);
		// int my_write(int fd, const char* buffer, int nbytes);
		
		
		
		
		

		int fd;


};

#endif