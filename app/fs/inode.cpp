#include <iostream>
#include "inode.h"

using namespace std;

//******************************************************************************
// constructor
Inode::Inode(mode_t mode, int id) {
    i_mode = mode;
    i_id = id;
    i_atime = time(0);
    i_mtime = time(0);
    i_ctime = time(0);
    i_blocks = 0;
}

//******************************************************************************
void Inode::setTimes(time_t atime, time_t mtime, time_t ctime) {
    i_atime = atime;
    i_mtime = mtime;
    i_ctime = ctime;
}

//******************************************************************************
void Inode::setBlocks(int blocks) {
    i_blocks = blocks;
}

//******************************************************************************
void Inode::displayInfo() const {
    cout << "File Mode: " << i_mode << endl;
    cout << "Number of Hard Links: " << i_nlink << endl;
    cout << "Owner UID: " << i_uid << endl;
    cout << "Group GID: " << i_gid << endl;
    cout << "Size: " << i_size << " bytes" << endl;
    cout << "Start Address: " << i_id << endl;
    cout << "Access Time: " << ctime(&i_atime);
    cout << "Modification Time: " << ctime(&i_mtime);
    cout << "Creation Time: " << ctime(&i_ctime);
    cout << "Blocks Occupied: " << i_blocks << endl;
}



