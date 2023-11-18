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

    int fd = -1;
    // Create a new file
    fd = filesystem.my_creat("testdir", 0644);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir created with fd " << fd << endl << endl << endl;
    }

    fd = filesystem.my_creat("testdir2", 0644);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir2 created with fd " << fd << endl << endl << endl;
    }

    fd = filesystem.my_creat("testdir\\test.txt", 0644);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir\\test.txt created with fd " << fd << endl << endl << endl;
    }

    fd = filesystem.my_creat("testdir2\\test3dir", 0644);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir2\\test3dir created with fd " << fd << endl << endl << endl;
    }

    fd = filesystem.my_creat("testdir2\\test3dir\\test2.txt", 0644);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir2\\test3dir\\test2.txt created with fd " << fd << endl << endl << endl;
    }

        fd = filesystem.my_creat("testdir4\\test", 0644);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir4\\test created with fd " << fd << endl << endl << endl;
    }

    return 0;
}