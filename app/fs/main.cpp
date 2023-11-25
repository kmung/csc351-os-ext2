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

    // Creat files and directories
    filesystem.my_creat("testdir", 0644 | S_IFDIR);
    filesystem.my_creat("testdir\\test.txt", 0700 | S_IFREG);
    filesystem.my_creat("testdir\\test2.txt", 0755 | S_IFREG);
    filesystem.my_creat("testdir\\test3.txt", 0644 | S_IFREG);
    filesystem.my_creat("testdir2", 0644 | S_IFDIR);
    filesystem.my_creat("testdir2\\test3dir", 0644 | S_IFDIR);
    filesystem.my_creat("testdir2\\test3dir\\tt.txt", 0600 | S_IFREG);
    filesystem.my_creat("testdir2\\test3dir\\tt2.txt", 0644 | S_IFREG);
    
    // Test close and open
    filesystem.my_close(2);
    filesystem.my_open("testdir\\test.txt", 0644);

    // Test write and read, make sure lseek before read and write
    cout << endl << "Test write and read" << endl;
    filesystem.my_lseek(1, 0, SEEK_SET);
    filesystem.my_write(1, "hello", 5);
    filesystem.my_write(1, " world", 6);
    char buffer[15];
    filesystem.my_lseek(1, 0, SEEK_SET);
    filesystem.my_read(1, buffer, 15);

    // Test overwrite
    cout << endl << "Test overwrite" << endl;
    filesystem.my_lseek(1, 5, SEEK_SET);
    filesystem.my_write(1, " jos", 4);
    filesystem.my_lseek(1, 0, SEEK_SET);
    filesystem.my_read(1, buffer, 15);

    // Test mkdir and ls
    cout << endl << "Test first ls" << endl;
    filesystem.my_mkdir("testdir\\testdir4", 0644 | S_IFDIR);
    filesystem.my_ls("testdir");

    // Test rmdir, chown, cd and ls
    cout << endl << "Test second ls" << endl;
    filesystem.my_rmdir("testdir\\testdir4");
    filesystem.my_chown("testdir\\test2.txt", 10, 10);
    filesystem.my_cd("testdir");
    filesystem.my_ls();

    // Test mv
    cout << endl << "Test mv" << endl;
    filesystem.my_ls("testdir2");
    filesystem.my_mv("testdir", "testdir2\\testdir.txt");
    filesystem.my_ls("testdir2");

    // Test ln
    cout << endl << "Test ln" << endl;
    int rc = filesystem.my_ln("testdir2\\testdir.txt", "testdir2\\testdir2.txt");
    int fd = filesystem.my_open("testdir2\\testdir2.txt", 0644);
    filesystem.my_lseek(fd, 0, SEEK_SET);
    filesystem.my_read(fd, buffer, 15);


    // Result
    // Test write and read
    // hello world

    // Test overwrite
    // hello josld

    // Test first ls
    // -rwx------ 2 1000 1000 0 Sat Nov 25 12:51:14 2023 test.txt
    // -rwxr-xr-x 2 1000 1000 0 Sat Nov 25 12:51:14 2023 test2.txt
    // -rw-r--r-- 2 1000 1000 0 Sat Nov 25 12:51:14 2023 test3.txt
    // drw-r--r-- 2 1000 1000 0 Sat Nov 25 12:51:14 2023 testdir4

    // Test second ls
    // Current path: \testdir
    // -rwx------ 2 1000 1000 0 Sat Nov 25 12:51:14 2023 test.txt
    // -rwxr-xr-x 2 10 10 0 Sat Nov 25 12:51:14 2023 test2.txt
    // -rw-r--r-- 2 1000 1000 0 Sat Nov 25 12:51:14 2023 test3.txt

    // Test mv
    // drw-r--r-- 2 1000 1000 0 Sat Nov 25 12:51:14 2023 test3dir
    // hello josld (This one is printed out because my_mv contians my_read inside)
    // drw-r--r-- 2 1000 1000 0 Sat Nov 25 12:51:14 2023 test3dir
    // -rw------- 2 1000 1000 11 Sat Nov 25 12:51:14 2023 testdir.txt

    // Test ln
    // hello josld


    return 0;
}