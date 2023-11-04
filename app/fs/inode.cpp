#include <iostream>
#include "inode.h"

using namespace std;

//******************************************************************************
// constructor
Inode::Inode(mode_t mode, int num) {
    i_mode = mode;
    i_num = num;
    i_atime = time(0);
    i_mtime = time(0);
    i_ctime = time(0);
    i_size = 0;
}

//******************************************************************************
// set inode's atime, mtime ,and ctime in that order
void Inode::setTimes(time_t atime, time_t mtime, time_t ctime) {
    i_atime = atime;
    i_mtime = mtime;
    i_ctime = ctime;
}

//******************************************************************************
// set inode's size
void Inode::setSize(int size) {
    i_size = size;
}

//******************************************************************************
// display inode's info
void Inode::displayInfo() const {
    cout << "File Mode: " << i_mode << endl;
    cout << "Number of Hard Links: " << i_nlink << endl;
    cout << "Owner UID: " << i_uid << endl;
    cout << "Group GID: " << i_gid << endl;
    cout << "Size: " << i_size << " bytes" << endl;
    cout << "Start Address: " << i_num << endl;
    cout << "Access Time: " << ctime(&i_atime);
    cout << "Modification Time: " << ctime(&i_mtime);
    cout << "Creation Time: " << ctime(&i_ctime);
    cout << "Blocks Occupied: " << i_size / (4 * 1024) << endl;
}



