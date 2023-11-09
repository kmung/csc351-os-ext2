#ifndef DISK_H
#define DISK_H
#include <string>
#include <ctime>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include "superblock.h"
#include "inode.h"
#include "dataBlock.h"
#include "dentry.h"
#include "bitmap.h"

using namespace std;

// Function to open the disk file
void openDisk(const string& devicePath, fstream& diskFile);

// Function to allocate memory to our disk partiton. 
int createDisk(const string& devicePath);

// Function to close the disk file
void closeDisk(fstream& diskFile);

// Initializations for the disk
void initSuperBlock(fstream& disk);
void initBlockBitmap(fstream& disk);
void initInodeBitmap(fstream& disk);
void initFirstInode(fstream& disk);
void initRootdentry(fstream& disk, int inum, int parentInum);

// Read the contents of the disk
void readSuperBlock(fstream& disk, SuperBlock& sb);
void readInodeBitmap(fstream& disk, bitmap& inodeBitmap);
void readBlockBitmap(fstream& disk, bitmap& blockBitmap);
void readInode(fstream& disk, int inum, Inode& inode);
void readDentry(fstream& disk, std::vector<dentry>& entries, int inum);

#endif