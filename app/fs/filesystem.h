#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>

#include "disk.h"
#include "superblock.h"
#include "dentry.h"
#include "inode.h"
#include "datablock.h"

using namespace std;

class FileSystem {
    private:
        string devicePath;
        bool isMounted;
        fstream diskFile;

        //  responsible for finding and reserving an available inode for the new file
        bool allocateInode(int &inodeNumber);

    public:
        FileSystem(const string &devicePath);

        bool mount();
        bool unmount();

        bool createFile(const string &filename, int fileSize);
        bool deleteFile(const string &path);
        int openFile(const string &path, const string &mode);
        bool closeFile(int fileDescriptor);
        ssize_t readFile(int fileDescriptor, char* buffer, size_t size);
        ssize_t writeFile(int fileDescriptor, const char* buffer, size_t size);
        bool seekFile(int fileDescriptor, size_t position);
        vector<string> listDirectory(const string& path);
        bool changeDirectory(const string& path);
};

#endif