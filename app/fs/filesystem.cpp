#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <stdexcept> 
#include <string> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>

#include "filesystem.h"
#include "disk.h"
#include "SuperBlock.h"
#include "bitmap.h"

using namespace std;

fs::fs() {
    string devicePath = "C:/Users/ssyak/Downloads/disk.vhd";

    // Create a filesystem object
    createDisk(devicePath);

    // Initialize the disk member variable
    disk.open(devicePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!disk) {
        cerr << "Failed to open device: " << devicePath << std::endl;
    }

    // Initialize all contents of the disk
    readSuperBlock(disk, sb);
    readInode(disk, 0, inode);
    readDentry(disk, rootEntry, 0);
    readInodeBitmap(disk, inodeBitmap);
    readBlockBitmap(disk, blockBitmap);

    // Set the first inode and first two data blocks as used
    blockBitmap.setBit(0, true);
    blockBitmap.setBit(1, true);
    inodeBitmap.setBit(0, true);
}

//******************************************************************************

fs::~fs() {

}

//******************************************************************************

int fs::my_creat(const string& fileName, mode_t mode) {
    // Extract the file name from the file path
    size_t pos = fileName.find_last_of("/\\");
    string fName = fileName.substr(pos + 1);

    // Find a free inode and data block
    int inodeNum = inodeBitmap.findFirstFree();
    int blockNum = blockBitmap.findFirstFree();

    // Initialize the inode
    // Gid and Uid is not using for this assignment
    // So set them to 1000 for now
    // As blocknum is the index of the data block, add it to the first data block address
    Inode newInode{mode, inodeNum, 2, 1000, 1000, 0, time(nullptr), time(nullptr), time(nullptr), FIRST_DATA_BLOCK + blockNum};

    // Write the inode to disk
    writeInode(disk, inodeNum, newInode);

    // Mark the inode and data block as used if they found enough free
    if(inodeNum != -1){
        inodeBitmap.setBit(inodeNum, true);
    }
    if(blockNum != -1){
        blockBitmap.setBit(blockNum, true);
        blockBitmap.setBit(blockNum + 1, true);
    }

    // Find the parent directory of the new file
    // If the parent directory is not found, parent Inum will be -1, then throw an error
    int parentInum = -1;
    vector<dentry> parentDentries;
    findParent(disk, fileName, parentDentries, parentInum);

    if(parentInum >= 0){
        // If parent directory exists,
        // Update the parent directory entry to include the new file
        updateParentDentry(disk, fileName, inodeNum, parentDentries, parentInum);

        // Write the new dentry for the new file
        writeStartDentry(disk, inodeNum, parentInum);

    } else {
        cout << "Parent directory not found" << endl;
        // If failed to find parent directory, free the inode and data block
        // Inode still remains on the disk, but it will be overwritten later
        inodeBitmap.setBit(inodeNum, false);
        blockBitmap.setBit(blockNum, false);
        blockBitmap.setBit(blockNum + 1, false);

        // Throw an error
        throw runtime_error("Parent directory not found");
    }

    // Check what parent directory contains
    // Remove this part if check is not needed
    vector<dentry> parentDentry;
    readDentry(disk, parentDentry, parentInum);

    cout << endl << "Last parent contains:" << endl;
    for (int i = 0; i < parentDentry[0].nEntries; i++) {
        if (i < 0 || i >= parentDentry.size()) {
            cout << "Index out of range" << endl;
            throw out_of_range("Index out of range");
        }

        if (parentDentry[i].fname == nullptr) {
            cout << "fname is not initialized" << endl; 
            throw runtime_error("fname is not initialized");
        }

        // If inode is a pointer
        if (parentDentry[i].inode == -1) {
            cout << "inode is not initialized" << endl;
            throw runtime_error("inode is not initialized");
        }
        cout << parentDentry[i].fname << ": " << parentDentry[i].inode << endl;
    }

    // Return the inode number as the file descriptor
    return inodeNum;
}

//******************************************************************************
void fs::writeInode(fstream& disk, int inodeNum, Inode& inode) {
    // Check if the disk file is open
    if (!disk.is_open()) {
        throw runtime_error("Disk file is not open");
    }

    // Calculate the position of the inode in the file
    int pos = FIRST_INODE * BLOCK_SIZE + (inodeNum * INODE_SIZE);

    // Seek to the position of the inode
    disk.seekp(pos);

    // Check if the seek failed
    if (disk.fail()) {
        throw runtime_error("Failed to seek to inode");
    }

    // Write the inode data
    disk.write(reinterpret_cast<const char*>(&inode), INODE_SIZE);

    // Check if the write failed
    if (disk.fail()) {
        throw runtime_error("Failed to write inode");
    }
}

//******************************************************************************
void fs::writeDentry(fstream& disk, const vector<dentry>& entries, int inum) {
    // Read the inode of the directory
    Inode node;
    readInode(disk, inum, node);

    // Seek to the data block of the directory
    disk.seekp(node.blockAddress * BLOCK_SIZE, ios_base::beg);

    // Write each dentry structure to the disk
    for (const dentry& entry : entries) {
        disk.write(reinterpret_cast<const char*>(&entry), DENTRY_SIZE);
    }
}

//*****************************************************************************
void fs::writeStartDentry(fstream& disk, int inum, int parentInum) {
    // Read the inode of the directory
    Inode node;
    readInode(disk, inum, node);

    // Seek to the data block of the directory
    disk.seekp((node.blockAddress) * BLOCK_SIZE, ios_base::beg);

    // Initialize a dentry structure for "."
    dentry dotEntry;
    dotEntry.inode = inum;
    strncpy(dotEntry.fname, "itself-start", MAX_NAME_LEN);
    dotEntry.nEntries = 2;

    // Write the "." dentry to the data block
    disk.write(reinterpret_cast<const char*>(&dotEntry), DENTRY_SIZE);

    // Initialize a dentry structure for ".."
    dentry dotDotEntry;
    dotDotEntry.inode = parentInum;
    strncpy(dotDotEntry.fname, "parent-start", MAX_NAME_LEN);
    dotDotEntry.nEntries = 2;

    // Write the ".." dentry to the data block
    disk.write(reinterpret_cast<const char*>(&dotDotEntry), DENTRY_SIZE);
}

//******************************************************************************
void fs::findParent(fstream& disk, string fileName, vector<dentry>& parentDentries, int& parentInum) {
    // Check if the disk file is open
    if (!disk.is_open()) {
        throw runtime_error("Disk file is not open");
    }

    // Check if the file name is empty
    if (fileName.empty()) {
        throw runtime_error("File name is empty");
    }

    // Parse and save given file name(file path)
    istringstream iss(fileName);
    string token;
    vector<string> pathComponents;

    // Use a while loop to tokenize the path and store components in the vector
    while (getline(iss, token, '\\')) {
        pathComponents.push_back(token);
    }

    // For the case when the path is just contains one file name
    // Parent inum is 0, which is the root directory as default
    readDentry(disk, parentDentries, 0);
    parentInum = 0;

    // Search for the file in the root directory if the path has at least one subdirectory
    if(pathComponents.size() > 1){

        // Terminates the loop when finish searching all parents
        for (int i = 0; i < pathComponents.size() - 1; i++) {

            // Scan all entries in parent dentries to find the entry for the file path component
            for (int j = 0; j < parentDentries[0].nEntries; j++) {

                // Check if the current dentry contains the entry for the file path component
                if (parentDentries[j].fname == pathComponents[i]) {
                    parentInum = parentDentries[j].inode;

                    // For testing, remove it later
                    cout << endl << "Find " << parentDentries[j].fname << ", inum : " << parentDentries[j].inode << endl;

                    // Clear the current entry and update to the next directory entry
                    parentDentries.clear();
                    readDentry(disk, parentDentries, parentInum);

                    // Print out the contents of parentDentries
                    cout << "Current Dentry contains:" << endl;
                    for (int i = 0; i < parentDentries.size(); i++) {
                        cout << "Entry " << i+1 << ":" << endl;
                        cout << "\tInode: " << parentDentries[i].inode << endl;
                        cout << "\tFile Name: " << parentDentries[i].fname << endl;
                        cout << "\tNumber of Entries: " << parentDentries[i].nEntries << endl;
                    }

                    break;

                } else {
                    // If fail to find parent, return -1 as parentInum
                    parentInum = -1;
                }
            }
        }
    }
}

// //******************************************************************************
void fs::updateParentDentry(fstream& disk, string fileName, int inum, vector<dentry> parentDentries, int parentInum) {
    // Check if the disk file is open
    if (!disk.is_open()) {
        throw runtime_error("Disk file is not open");
    }

    // Check if the file name is empty
    if (fileName.empty()) {
        throw runtime_error("File name is empty");
    }

    // Extract the file name from the file path
    size_t pos = fileName.find_last_of("\\");
    string fName = fileName.substr(pos + 1);

    // Create the new dentry for the file that will be added to the parent directory
    dentry newEntry;
    newEntry.inode = inum;
    strncpy(newEntry.fname, fName.c_str(), MAX_NAME_LEN);
    newEntry.nEntries = 2;

    // Add the new dentry to the parent directory
    parentDentries.push_back(newEntry);
    parentDentries[0].nEntries++;
    
    // Write the updated parent directory entry to the disk
    writeDentry(disk, parentDentries, parentInum);
}

