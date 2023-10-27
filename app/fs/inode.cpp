#include "inode.h"

using namespace std;

//******************************************************************************
inode::inode(mode_t i_mode, int i_address) {
    this->i_mode = i_mode;
    i_nlink = 1;
    // i_uid? 
    // i_gid?
    // i_size
    // i_address
    // i_ctime
    // i_blocks
}