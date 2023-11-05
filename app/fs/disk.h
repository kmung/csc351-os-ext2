#ifndef DISK_H
#define DISK_H
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

class Disk {
    private:
        string devicePath;
        // Other internal state and methods...
    public:
        Disk(const string &devicePath);
        bool openDevice();
        void readBlock(unsigned int blockNumber, void* buffer, size_t size);
        // Other I/O methods...
};

#endif