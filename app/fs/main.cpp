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
    fd = filesystem.my_creat("testdir", 0755 | S_IFDIR);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir created with fd " << fd << endl << endl << endl;
    }

    fd = filesystem.my_creat("testdir2", 0620 | S_IFDIR);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir2 created with fd " << fd << endl << endl << endl;
    }

    fd = filesystem.my_creat("testdir\\test.txt", 0644 | S_IFREG);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir\\test.txt created with fd " << fd << endl << endl << endl;
    }

    fd = filesystem.my_creat("testdir2\\test3dir", 0600 | S_IFDIR);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir2\\test3dir created with fd " << fd << endl << endl << endl;
    }

    fd = filesystem.my_creat("testdir2\\test2.txt", 0644 | S_IFREG);
    if (fd == -1) {
        cerr << "Failed to create file\n";
        return 1;
    } else {
        cout << "testdir2\\test2.txt created with fd " << fd << endl << endl << endl;
    }

    // fd = filesystem.my_creat("testdir2\\test2.txt", 0644 | S_IFREG);
    // if (fd == -1) {
    //     cerr << "Failed to create file\n";
    //     return 1;
    // } else {
    //     cout << "testdir2\\test2.txt created with fd " << fd << endl << endl << endl;
    // }

    // fd = filesystem.my_creat("", 0644);
    // if (fd == -1) {
    //     cerr << "Failed to create file\n";
    //     return 1;
    // } else {
    //     cout << "" << fd << endl << endl << endl;
    // }

    // filesystem.my_mkdir("testdir\\test3dir", 0644);

    filesystem.my_close(2);

    filesystem.my_open("testdir2", 0644);

    struct stat buf;

    if(filesystem.my_stat("testdir\\test.txt", buf)){
       cout << "testdir\\test.txt inf: " << endl;
        cout << "Inode: " << buf.st_ino << endl;
        cout << "Atime: " << buf.st_atime << endl;
        cout << "Size: " << buf.st_size << endl; 
    }
    

    cout << endl;

    struct stat buf2;

    if(filesystem.my_fstat(2, buf2)){
        cout << "testdir2 inf: " << endl;
        cout << "Inode: " << buf2.st_ino << endl;
        cout << "Atime: " << buf2.st_atime << endl;
        cout << "Size: " << buf2.st_size << endl;
    } else {
        cout << "testdir2 not found" << endl;
    }
    
    filesystem.my_lseek(1, 0, SEEK_SET);
    filesystem.my_write(1, "hello", 5);
    filesystem.my_write(1, " world", 6);

    filesystem.my_lseek(1, 5, SEEK_SET);
    filesystem.my_write(1, " jos", 4);

    filesystem.my_lseek(1, 0, SEEK_SET);
    char buffer[15];
    int bytesRead = filesystem.my_read(1, buffer, 15);
    // Null-terminate the buffer
    buffer[bytesRead] = '\0'; 
    cout << "How much bytes read: " << bytesRead << endl;
    cout << "Buffer: " << buffer << endl;

    filesystem.my_ls("testdir2");


    // filesystem.my_rmdir("testdir2\\test3dir");
    filesystem.my_chown("testdir2\\test2.txt", 10, 10);
    filesystem.my_cd("testdir2");
    filesystem.my_ls();

    cout << endl;

    filesystem.my_mv("testdir", "testdir.txt");


    return 0;
}