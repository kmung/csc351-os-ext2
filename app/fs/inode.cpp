#include <iostream>
#include "inode.h"

using namespace std;

//******************************************************************************
// constructor
Inode::Inode(int index) {
    this->index = index;
    atime = time(0);
    mtime = time(0);
    ctime = time(0);
    size = 0;
}

//******************************************************************************
// set inode's atime, mtime ,and ctime in that order
void Inode::setTimes(time_t atime, time_t mtime, time_t ctime) {
    this->atime = atime;
    this->mtime = mtime;
    this->ctime = ctime;
}

//******************************************************************************
// set inode's size
void Inode::setSize(int size) {
    this->size = size;
    nblocks = size % 4096 ? ((size / 4096) + 1) : (size / 4096);
}

//******************************************************************************
// display inode's info
void Inode::displayInfo() const {
    cout << "File Mode: " << mode << endl;
    cout << "Number of Hard Links: " << nlink << endl;
    cout << "Owner UID: " << uid << endl;
    cout << "Group GID: " << gid << endl;
    cout << "Size: " << size << " bytes" << endl;
    cout << "Start Address: " << index << endl;
    cout << "Access Time: " << atime << endl;;
    cout << "Modification Time: " << mtime << endl;
    cout << "Creation Time: " << ctime << endl;
    cout << "Blocks Occupied: " << size / (4 * 1024) << endl;
}



