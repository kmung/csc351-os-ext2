#ifndef FS_H
#define FS_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

#include "inode.h"

// FS date
#define FS_DATE "10/24/2023

using namespace std;

//******************************************************************************
// ext2 based fs
class fs {
	private:
		
	public:
		fs(string pathname="");
		~fs();

		// refer to https://man7.org/linux/man-pages/man2/syscalls.2.html for more
		int my_creat(const char *pathname, mode_t mode);
		int my_open(const char *pathname);
		ssize_t read(int fd, void buf[.count], size_t count);
		ssize_t write(int fd, const void buf[.count], size_t count);

};

#endif