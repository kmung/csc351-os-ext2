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
#include "datablock.h"

using namespace std;

//******************************************************************************
// ext2 based fs
class fs {
	public:
		fs();
		~fs();

		// refer to https://man7.org/linux/man-pages/man2/syscalls.2.html for more
		
		int my_read(int fd, char* buffer, int nbytes);
		int my_write(int fd, const char* buffer, int nbytes);
		int my_creat(const string& name, int mode);
		int my_Iseek(int fd, off_t offset, int when);
		int my_fstat(int fd, struct stat* buf);
		int my_open(const char *pathname, mode_t mode);
		int my_close(int fd);
		int my_stat(const string& name, struct stat& buf);

		int fd;


};
#endif