#ifndef INODE_H
#define INODE_H

#include <sys/stat.h>
#include <ctime>

// special inode numbers
#define FS_BAD_INO 1         // bad blocks inode
#define FS_ROOT_INO 2        // root inode
#define FS_BOOT_LOADER_INO 5 // boot loader inode
#define FS_UNDEL_DIR_INO 6   // undelete directory inode

// structure of inode
class inode {
	private:
		// file mode
		mode_t i_mode;
		// number of hard links count
		int i_nlink;
		// 16 bits of Owner Uid
		short i_uid;
		// 16 bits of Group Id
		short i_gid;
		// size in bytes
		int i_size;
		// index of file's starting location
		int i_address;
		// access time
		clock_t i_atime;
		// modification time
		clock_t i_mtime;
		// creation time
		clock_t i_ctime;
		// number of blocks occupied
		int i_blocks;

	public:
		inode(mode_t i_mode, int i_address);
		~inode();
};

#endif