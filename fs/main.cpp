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
        cout << "File created with fd " << fd << '\n' << endl;
    }

    fd = filesystem.my_creat("testdir\\test.txt", 0644);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "File created with fd " << fd << '\n' << endl;
    }

    return 0;
}