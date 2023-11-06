#include "superblock.h"

using namespace std;

//******************************************************************************
bool Superblock::allocateInode(int &inodeNum) {
    bool rc = false;
    if (inodeNum == 2) {
        // Case 1: inode is for root directory
        inodeFreeList[2] = true;
        rc = true;
    } else {
        // Case 1: inode is for a regular file/directory
        // *inodeFreeList for regular files and directory start at index #11
        if (!firstFreeInode) {
            inodeFreeList[firstFreeInode] = true;
            inodeNum = firstFreeInode;
            for (int i = firstFreeInode + 1; i < nInodes; i++) {
                if (inodeFreeList[i] == false) {
                    firstFreeInode = i;
                    break;
                }
            }

            rc = true;
        }
    }

    return rc;
}