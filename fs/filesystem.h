#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/types.h>
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
		Bitmap inodeBitmap;
		Bitmap blockBitmap;

		// Store fd of opened files
		vector<int> openFdTable;

		string curPath;
		int curInum;

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

		int allocateMem(int allocateSize, int inum, int& curMaxBlocks);

	public:
		fs(string vhd_path);
		~fs();

		// refer to https://man7.org/linux/man-pages/man2/syscalls.2.html for more
		int my_creat(const string& path, mode_t mode);
		int my_open(const char *pathname, mode_t mode);
		bool my_close(int fd);
		bool my_stat(const string& path, struct stat& buf);
		bool my_fstat(int fd, struct stat& buf);
		int my_lseek(int fd, off_t offset, int whenence);

		int my_read(int fd, char* buffer, int nbytes);
		int my_write(int fd, const char* buffer, int nbytes);
		
		// Need absolute path
		string my_ls(const string& name);
		// If parameter is empty, list current directory
		string my_ls();
		// It can be absolute path or relative path
		string my_cd(const string& name);
		// Need absolute path
		int my_mkdir(const string& name, mode_t mode);
		// If path is not given, remove current directory 
		int my_mkdir(mode_t mode);
		//keep track of current directory
		string my_getcwd();


		int my_rmdir(const string& name);
		int my_chown(const string& name, int owner, int group);
		int my_cp(const string& srcPath, const string& destPath);
		int my_mv(const string& srcPath, const string& destPath);
		int my_rm(const string& name);
		int my_ln(const string& srcPath, const string& destPath);
		string my_cat(const string& srcPath);
		int my_Lcp(const string& srcPath, const string& destPath);
		int my_lcp(const string& srcPath, const string& destPath);
};

#endif