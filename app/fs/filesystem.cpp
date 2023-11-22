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
#include <fcntl.h>
#include <algorithm>

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
// Allocate memory using the first-fit algorithm
int fs::allocateMem(int size, Bitmap bitmap) {
    int rc = -1;
    int numBlocksNeeded = size / BLOCK_SIZE + (size % BLOCK_SIZE != 0);

    for (int i = 0; i < (MEMORY_SIZE / BLOCK_SIZE); ++i) {
        int consecutiveFreeBlocks = 0;

        // Check if the current block is free
        if (!(bitmap.isBitSet(i))) {
            consecutiveFreeBlocks++;

            // Check the next blocks
            for (int j = i + 1; (j < (MEMORY_SIZE / BLOCK_SIZE)) && (consecutiveFreeBlocks < numBlocksNeeded); ++j) {
                if (!(bitmap.isBitSet(j))) {
                    consecutiveFreeBlocks++;
                } else {
                    // Stop checking consecutive blocks if one is not free
                    break;
                }
            }

            // If enough consecutive free blocks are found, mark them as allocated and return the starting index
            if (consecutiveFreeBlocks == numBlocksNeeded) {
                for (int k = i; k < i + numBlocksNeeded; ++k) {
                    // Mark block as allocated
                    bitmap.setBit(k, 1);
                }
                rc = i * BLOCK_SIZE;
            }
        }
    }

    return rc;
}
//******************************************************************************

int fs::my_creat(const string& fileName, mode_t mode) {
    // Check if the disk file is open
    if (!disk.is_open()) {
        throw runtime_error("Disk file is not open");
    }

    // Check if the file name is empty
    if (fileName.empty()) {
        throw runtime_error("File name is empty");
    }

    // // Check if the mode is valid
    // if ((mode & S_IFMT) != S_IFREG) {
    //     throw runtime_error("Invalid mode. Only regular files are supported.");
    // }

    // Extract the file name from the file path
    size_t pos = fileName.find_last_of("/\\");
    string fName = fileName.substr(pos + 1);

    // Find a free inode and data block
    int inodeNum = inodeBitmap.findFirstFree();
    int blockNum = blockBitmap.findFirstFree();

    // Initialize the inode
    // Gid and Uid is not using for this assignment, set them to 1000 for now
    // As blocknum is the index of the data block, add it to the first data block address
    Inode newInode{mode, inodeNum, 2, 1000, 1000, 0, time(nullptr), time(nullptr), time(nullptr), FIRST_DATA_BLOCK + blockNum};

    // Write the inode to disk
    writeInode(disk, inodeNum, newInode);

    // Mark the inode and data block as used if they found enough free
    if(inodeNum != -1){
        inodeBitmap.setBit(inodeNum, true);
    }
    if(blockNum != -1){
        // Mark two datablocks, one for the dentry, one for the file content
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

    // Mark openFdTable to indicate the file is opened
    openFdTable.push_back(inodeNum);

    // Return the inode number as the file descriptor
    return inodeNum;
}

//******************************************************************************
int fs::my_open(const char *pathname, mode_t mode){
    int rc = -1;
    string pathStr = pathname;
    size_t pos = pathStr.find_last_of("\\");
    string filename = pathStr.substr(pos + 1);

    int parentInum;
    vector<dentry> parentDentry;
    findParent(disk, pathname, parentDentry, parentInum);

    // Check if the file exists in the parent directory
    for (int i = 0; i < parentDentry[0].nEntries; i++) {
        if (parentDentry[i].fname == filename) {
            // The file exists, return its file descriptor (inode number)
            rc = parentDentry[i].inode;

            // Add the file descriptor to the open file table
            openFdTable.push_back(rc);

            break;
        }
    }

    return rc;

}

//******************************************************************************
bool fs::my_close(int fd){
    bool rc = false;

    if(find(openFdTable.begin(), openFdTable.end(), fd) == openFdTable.end()){
        cout << "File descriptor not found" << endl;
    } else {
        // Remove the file descriptor from the open file table
        openFdTable.erase(find(openFdTable.begin(), openFdTable.end(), fd));
        rc = true;
    }

    return rc; 
}

//******************************************************************************
bool fs::my_stat(const string& pathname, struct stat& buf) {
    // Return false if file not found
    bool rc = false;

    string pathStr = pathname;
    size_t pos = pathStr.find_last_of("\\");
    string filename = pathStr.substr(pos + 1);

    int parentInum;
    int inum = -1;
    vector<dentry> parentDentry;
    findParent(disk, pathname, parentDentry, parentInum);

    // Check if the file exists in the parent directory
    for (int i = 0; i < parentDentry[0].nEntries; i++) {
        if (parentDentry[i].fname == filename) {
            inum = parentDentry[i].inode;
            rc = true;
            break;
        }
    }

    if(rc) {
        // Read the inode from disk
        Inode inode;
        readInode(disk, inum, inode);

        // Fill the stat structure
        buf.st_mode = inode.mode;
        buf.st_ino = inode.num;
        buf.st_nlink = inode.nlink;
        buf.st_uid = inode.uid;
        buf.st_gid = inode.gid;
        buf.st_size = inode.size;
        buf.st_atime = inode.atime;
        buf.st_mtime = inode.mtime;
        buf.st_ctime = inode.ctime;
    } else {
        cout << "File not found" << endl;
        throw runtime_error("File not found");
    }

    return rc;
}

//******************************************************************************
bool fs::my_fstat(int fd, struct stat& buf) {
    bool rc = false;

    // Check if the file descriptor is in openFdTable
    if (find(openFdTable.begin(), openFdTable.end(), fd) != openFdTable.end()) {
        // Read the inode from disk
        Inode inode;
        readInode(disk, fd, inode);

        // Fill the stat structure
        buf.st_mode = inode.mode;
        buf.st_ino = inode.num;
        buf.st_nlink = inode.nlink;
        buf.st_uid = inode.uid;
        buf.st_gid = inode.gid;
        buf.st_size = inode.size;
        buf.st_atime = inode.atime;
        buf.st_mtime = inode.mtime;
        buf.st_ctime = inode.ctime;

        rc = true;
    }

    return rc;
}

//******************************************************************************
int fs::my_lseek(int fd, off_t offset, int whence) {
    int position = -1;
    bool fileOpened = (find(openFdTable.begin(), openFdTable.end(), fd) != openFdTable.end());

    if(fileOpened){
        Inode inode;
        readInode(disk, fd, inode);
        position = inode.blockAddress * BLOCK_SIZE;

        switch (whence) {
        case SEEK_SET:
            // The position is set to file location plus offset bytes
            position += offset;
            disk.seekp(position, ios_base::beg);
            disk.seekg(position, ios_base::beg);
            break;
        case SEEK_CUR:
            // The position is set to its current location plus offset bytes
            position = disk.tellp() + offset;
            disk.seekp(position, ios_base::beg);
            disk.seekg(position, ios_base::beg);
            break;
        case SEEK_END:
            // The position is set to file location plus size of the file plus offset bytes
            // In this case, offset is negative
            position += (inode.size + offset);
            disk.seekp(position, ios_base::beg);
            disk.seekg(position, ios_base::beg);
            break;
        default:
            cout << "Invalid whence value" << endl;
            position = -1;
        }
    }


    return position;
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

