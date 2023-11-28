#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>


#include "disk.h"
#include "superblock.h"
#include "inode.h"
#include "dentry.h"
#include "filesystem.h"


using namespace std;

int main(int argc, char **argv) {
    // Create a filesystem object
    fs filesystem;

    // Import a test png file
    filesystem.my_lcp("/home/ckmung/Documents/csc351/csc351-os-ext2/test.png", "testfile");

    // Create directories and check they are created by ls (without parameter) command
    filesystem.my_mkdir("testdir", 0755);
    filesystem.my_mkdir("testdir2", 0600);
    filesystem.my_mkdir("testdir3", 0544);
    cout << filesystem.my_ls() << endl << endl;

    // Create a copy of testfile in first test directory
    // Change the owner of second test directory
    // Remove third test directory
    // Check all changes on root directory by ls (without parameter)
    // Check my_cp created the copy under first test directory by ls (with absolute path)
    filesystem.my_cp("testfile", "testdir/cpfile");
    filesystem.my_chown("testdir2", 10, 10);
    filesystem.my_rmdir("testdir3");
    cout << filesystem.my_ls() << endl;
    cout << filesystem.my_ls("testdir") << endl << endl;

    // Remove the original testfile
    // As the testfile was copied, we still can get information from cpfile
    // Move cpfile to second test directory
    // This time, use cd to move current directory and use ls (without command) to check
    // my_mv correctly created the file
    // Check ls with absolute path still work after cwd has been changed
    // If it works properly, it will show cpfile has been deleted
    filesystem.my_rm("testfile");
    filesystem.my_mv("testdir/cpfile", "testdir2/mvfile");
    cout << filesystem.my_cd("testdir2");
    cout << "testdir2: " << filesystem.my_ls() << endl;
    cout << "testdir: " << filesystem.my_ls("testdir") << endl << endl;

    // Import a new text file
    // Create a hard link to the new text file
    // Use cat command to check hard link correctly points the original file
    filesystem.my_lcp("/home/ckmung/Documents/csc351/csc351-os-ext2/test.txt", "testfile2");
    filesystem.my_ln("testfile2", "lnfile");
    cout << filesystem.my_cat("testfile2") << endl;
    cout << filesystem.my_cat("lnfile") << endl << endl;

    // Export the png file and text file
    filesystem.my_Lcp("testdir2/mvfile", "/home/ckmung/Documents/csc351/csc351-os-ext2/testexport.png");
    filesystem.my_Lcp("testfile2", "/home/ckmung/Documents/csc351/csc351-os-ext2/testexport.txt");





    return 0;
}