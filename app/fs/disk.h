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

using namespace std;

void openDisk(const string& devicePath, fstream& diskFile);
// Function to allocate memory to our disk partiton. 
int createDisk(const string& devicePath);

void closeDisk(fstream& diskFile);

void readSuperBlock(fstream& disk, SuperBlock& sb);

void readInode(fstream& disk, int inum, Inode& inode);

void readDentry(fstream& disk, std::vector<dentry>& entries);

void initSuperBlock(fstream& disk);

void initInode(fstream& disk);

dentry initRootDentry(fstream& disk);

#endif