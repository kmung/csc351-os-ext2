#ifndef INODE_H
#define INODE_H

#include <string>
#include <sys/stat.h>
#include <stdio.h>
#include <ctime>

using namespace std;

// special inode numbers
#define FS_BAD_INO 1         // bad blocks inode
#define FS_ROOT_INO 2        // root inode
#define FS_BOOT_LOADER_INO 5 // boot loader inode
#define FS_UNDEL_DIR_INO 6   // undelete directory inode

// structure of inode
class Inode {
	private:
		// file mode
		mode_t mode;
		// unique identifier
		int index;
		// number of hard links count
		int nlink;
		// 16 bits of Owner Uid
		short uid;
		// 16 bits of Group Id
		short gid;
		// size in bytes
		int size;
		// number of blocks allocated
		int nblocks;
		// access time
		time_t atime;
		// modification time
		time_t mtime;
		// creation time
		time_t ctime;
		// Include the index where file locates within data blocks
		// If we are not making a huge file system, we can just use direct blocks
		uint32_t blockPointers[11];

	public:
		// constructor
		Inode(int index=NULL);

		// set inode's atime, mtime, and ctime in that order
		void setTimes(time_t atime, time_t mtime, time_t ctime);

		// set inode's size
		void setSize(int size);

		// display inode's info
		void displayInfo() const;
};

#endif