#ifndef FS_H
#define FS_H

//ext2 based fs
struct fs {
  unsigned long inodes_per_block; // number of inodes per block
  unsigned long blocks_per_group; // number of blocks in a group
  unsigned long inodes_per_group; // number of inodes in a group
  unsigned long group_desc_count; // number of group descriptor blocks
  unsigned long desc_per_block; // number of group descriptors per block
  unsigned long groups_count; // number of groups in the fs
};

// FS date
#define FS_DATE "10/24/2023"

// special inode numbers
#define FS_BAD_INO 1 // bad blocks inode
#define FS_ROOT_INO 2 // Root inode
#define FS_BOOT_LOADER_INO 5 // boot loader inode
#define FS_UNDEL_DIR_INO 6 // undelete directory inode

// structure of the super block
struct fs_super_block
{
  // inodes count
  // blocks count
  // free blocks count
  // free inodes count
  // mount time
  // write time
  // default uid for reserved blocks
  // default gid for reserved blocks
};

// structure of directory entry
struct fs_dir_entry {
  // inode number
  // directory entry length
  // name length
  // file type
  // file name
};


#endif