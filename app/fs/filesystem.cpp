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

    curPath = "";
    curInum = 0;

    // Initialize the bitmaps
    inodeBitmap = Bitmap(NINODES);
    blockBitmap = Bitmap(NDATA_BLOCKS);

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
void fs::writeInode(fstream& disk, int inodeNum, Inode& inode) {
    streampos curPosition = disk.tellp();

    // Check if the disk file is open
    if (!disk.is_open()) {
        throw runtime_error("Disk file is not open");
    }

    // Calculate the position of the inode in the file
    int pos = FIRST_INODE * BLOCK_SIZE + (inodeNum * INODE_SIZE);

    // Seek to the position of the inode
    disk.seekp(pos, ios_base::beg);

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

    disk.seekp(curPosition, ios_base::beg);
}

//******************************************************************************
void fs::writeDentry(fstream& disk, const vector<dentry>& entries, int inum) {
    streampos curPosition = disk.tellp();

    // Read the inode of the directory
    Inode node;
    readInode(disk, inum, node);

    // Seek to the data block of the directory
    disk.seekp(node.blockAddress * BLOCK_SIZE, ios_base::beg);

    // Write each dentry structure to the disk
    for (const dentry& entry : entries) {
        disk.write(reinterpret_cast<const char*>(&entry), DENTRY_SIZE);
    }

    disk.seekp(curPosition, ios_base::beg);
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

                    // Clear the current entry and update to the next directory entry
                    parentDentries.clear();
                    readDentry(disk, parentDentries, parentInum);

                    break;

                } else {
                    // If fail to find parent, return -1 as parentInum
                    parentInum = -1;
                }
            }
        }
    }
}

//******************************************************************************
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

//******************************************************************************
int fs::allocateMem(int allocateSize, int inum, int& curMaxBlocks) {
    int rc = -1;

    // Read Inode from given inum
    Inode node;
    readInode(disk, inum, node);
    
    // If file size is less than block size, current max is 1
    // Otherwise update current max to the next multiple of 8
    if(node.size <= BLOCK_SIZE){
        curMaxBlocks = 1;
    } else {
        // Number of bytes that over one block
        int sizeOverOne = node.size - BLOCK_SIZE;

        // Check if the file is not exactly multiple of 8
        // If not, do not add 1 on quotient 
        int isFull = (sizeOverOne % (BLOCK_SIZE * 8) != 0);

        curMaxBlocks = (sizeOverOne / (BLOCK_SIZE * 8) + isFull) * 8 + 1;
    }

    // Check remainSpace is enough for allocateSize
    int remainSpace = curMaxBlocks * BLOCK_SIZE - node.size;

    // If not, calculate the number of blocks needed to be extened
    int numBlocksNeeded;
    if(remainSpace < allocateSize){
        int isFull = (allocateSize % (BLOCK_SIZE * 8) != 0);
        numBlocksNeeded = ((allocateSize - remainSpace) / (BLOCK_SIZE * 8) + isFull) * 8;
    } else {
        numBlocksNeeded = 0;
    }

    int newMaxBlocks = curMaxBlocks + numBlocksNeeded + 1;

    // Before looking for free blocks, free block bitmaps of the current file
    // If current address has enough space to extend enough blocks
    // It will be overwritten later
    int curDataBit = node.blockAddress - FIRST_DATA_BLOCK + 1;
    for (int i = 0; i < curMaxBlocks + 1; i++){
        blockBitmap.setBit(curDataBit + i, 0);
    }

    // Find contiguous free blocks to extend the file
    for (int i = 0; i < blockBitmap.getSize(); ++i) {
        int consecutiveFreeBlocks = 0;

        // Check if the current block is free
        if (!(blockBitmap.isBitSet(i))) {
            consecutiveFreeBlocks++;

            // Check the next blocks
            for (int j = i + 1; (j < blockBitmap.getSize()) && (consecutiveFreeBlocks < newMaxBlocks); ++j) {
                if (!(blockBitmap.isBitSet(j))) {
                    consecutiveFreeBlocks++;
                } else {
                    // Stop checking consecutive blocks if one is not free
                    break;
                }
            }

            // If enough consecutive free blocks are found, mark them as allocated and return the starting index
            if (consecutiveFreeBlocks == newMaxBlocks) {
                for (int k = i; k < i + newMaxBlocks; ++k) {
                    // Mark block as allocated
                    blockBitmap.setBit(k, 1);
                }
                // Return the starting index of the consecutive free blocks
                rc = (i + FIRST_DATA_BLOCK - 1) * 4096;
            }

            if(rc){
                break;
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

    // Check if the file already exists
    if (my_open(fileName.c_str(), mode) != -1) {
        throw runtime_error("File already exists: " + fileName);
    }

    // Extract the file name from the file path
    size_t pos = fileName.find_last_of("/\\");
    string fName = fileName.substr(pos + 1);

    // Find a free inode and data block
    int inodeNum = inodeBitmap.findFirstFree();
    int blockNum = -1;
    for (int i = 0; i < blockBitmap.getSize(); ++i) {
        int consecutiveFreeBlocks = 0;

        // Check if the current block is free
        if (!(blockBitmap.isBitSet(i))) {
            consecutiveFreeBlocks++;

            // Check the next blocks
            for (int j = i + 1; (j < blockBitmap.getSize()) && (consecutiveFreeBlocks < 2); ++j) {
                if (!(blockBitmap.isBitSet(j))) {
                    consecutiveFreeBlocks++;
                } else {
                    // Stop checking consecutive blocks if one is not free
                    break;
                }
            }

            // If enough consecutive free blocks are found, mark them as allocated and return the starting index
            if (consecutiveFreeBlocks == 2) {
                blockNum = i;
                break;
            }
        }
    }

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

    streampos curPosition = disk.tellg();
    if(fileOpened){
        Inode inode;
        readInode(disk, fd, inode);
        position = (inode.blockAddress + 1) * BLOCK_SIZE;

        switch (whence) {
        case SEEK_SET:
            // The position is set to file location plus offset bytes
            position += offset;
            disk.seekp(position, ios_base::beg);
            disk.seekg(position, ios_base::beg);
            break;
        case SEEK_CUR:
            // The position is set to its current location plus offset bytes
            position = curPosition + offset;
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

    curPosition = position;

    return position;
}

//******************************************************************************
int fs::my_read(int fd, char* buffer, int nbytes) {
    int rc = -1;
    bool fileOpened = (find(openFdTable.begin(), openFdTable.end(), fd) != openFdTable.end());
    streampos curPosition = disk.tellg();

    Inode node;
    readInode(disk, fd, node);

    int dataAddress = node.blockAddress + 1;

    // Check if fd is valid
    if(fd > 0 && fd <= MAX_OPEN_FILES){
        // Check if file is opened
        if(fileOpened){
            // Check if file is readable
            if (node.mode & S_IRUSR){
                // Check if current position is on the right spot
                // As blockAddress point is the index of dentry, add 1 and times BLOCK_SIZE to it
                if(dataAddress * BLOCK_SIZE <= curPosition && (dataAddress * BLOCK_SIZE + node.size) > curPosition){
                    // If remain bytes to read is less than nbytes, read the remaining bytes
                    // Otherwise, read nbytes
                    if(nbytes > node.size + dataAddress * BLOCK_SIZE - curPosition){
                        rc = node.size + dataAddress * BLOCK_SIZE - curPosition;
                    } else {
                        rc = nbytes;
                    }

                    // As reading starts from current position, do not move pointer
                    disk.read(buffer, rc);

                    // Null-terminate the buffer
                    buffer[rc] = '\0';

                    cout << buffer << endl;

                    if(disk.fail()){
                        if (disk.eof()) {
                            cout << "Failed to read file: Reached end of file" << endl;
                        } else if (disk.bad()) {
                            cout << "Failed to read file: Bad input or I/O error" << endl;
                        } else {
                            cout << "Failed to read file: Unknown error" << endl;
                        }
                        throw runtime_error("Failed to read file");
                    }
                } else {
                    cout << "Current position is not on the right spot" << endl;
                }
            } else {
                cout << "File is not readable" << endl;
            }
        } else {
            cout << "File is not opened" << endl;
        }
    } else {
        cout << "Invalid file descriptor" << endl;
    }

    return rc;
}

//******************************************************************************
int fs::my_write(int fd, const char* buffer, int nbytes){
    int rc = -1;
    bool fileOpened = (find(openFdTable.begin(), openFdTable.end(), fd) != openFdTable.end());
    streampos curWritePosition = disk.tellp();

    Inode node;
    readInode(disk, fd, node);
    char dentry_buffer[BLOCK_SIZE];
    disk.seekg(node.blockAddress * BLOCK_SIZE, ios_base::beg);
    disk.read(dentry_buffer, BLOCK_SIZE);

    char data_buffer[node.size];
    disk.seekg((node.blockAddress + 1) * BLOCK_SIZE, ios_base::beg);
    disk.read(data_buffer, node.size);

    // Check if fd is valid, file is opened, current position is on the right spot and the file is writable
    // Current position can locate at the end of the file
    if (fd > 0 && fd <= MAX_OPEN_FILES){
        if(fileOpened){
            if(node.mode & S_IWUSR){
                int curMaxBlocks;
                rc = allocateMem(nbytes, fd, curMaxBlocks);
                if(rc != -1){
                    disk.seekp(rc, ios_base::beg);
                    disk.write(dentry_buffer, BLOCK_SIZE);
                    disk.write(data_buffer, node.size);
                    disk.seekp(curWritePosition, ios_base::beg);
                    disk.write(buffer, nbytes);

                    // Update inode
                    int overWriteSize;
                    if((rc + BLOCK_SIZE + node.size - curWritePosition) > nbytes){
                        overWriteSize = nbytes;
                    } else {
                        overWriteSize = rc + BLOCK_SIZE + node.size - curWritePosition;
                    }
                    node.size += nbytes - overWriteSize;
                    node.blockAddress = rc / 4096;
                    writeInode(disk, fd, node);

                    Inode newNode;
                    readInode(disk, fd, newNode);

                } else {
                    cout << "Failed to allocate memory" << endl;
                }
            } else {
                cout << "File is not writable" << endl;
            }
        } else {
            cout << "File is not opened" << endl;
        }
    } else {
        cout << "Invalid file descriptor" << endl;
    }

    return rc;
}

//******************************************************************************
void fs::my_ls(){
    vector<dentry> entries;
    readDentry(disk, entries, curInum);

    if (entries[0].nEntries > 2){
        for (int i = 2; i < entries[0].nEntries; i++) {
            string newPath = curPath + "\\" + entries[i].fname;
            struct stat fileStat;
            if (my_stat(newPath, fileStat)) {
                string timeStr = ctime(&fileStat.st_mtime);
                timeStr.erase(std::remove(timeStr.begin(), timeStr.end(), '\n'), timeStr.end());

                // Print the file details in a format similar to ls -l
                cout << ((S_ISDIR(fileStat.st_mode)) ? 'd' : '-')
                    << ((fileStat.st_mode & S_IRUSR) ? 'r' : '-')
                    << ((fileStat.st_mode & S_IWUSR) ? 'w' : '-')
                    << ((fileStat.st_mode & S_IXUSR) ? 'x' : '-')
                    << ((fileStat.st_mode & S_IRGRP) ? 'r' : '-')
                    << ((fileStat.st_mode & S_IWGRP) ? 'w' : '-')
                    << ((fileStat.st_mode & S_IXGRP) ? 'x' : '-')
                    << ((fileStat.st_mode & S_IROTH) ? 'r' : '-')
                    << ((fileStat.st_mode & S_IWOTH) ? 'w' : '-')
                    << ((fileStat.st_mode & S_IXOTH) ? 'x' : '-')
                    << ' ' << fileStat.st_nlink
                    << ' ' << fileStat.st_uid
                    << ' ' << fileStat.st_gid
                    << ' ' << fileStat.st_size
                    << ' ' << timeStr
                    << ' ' << entries[i].fname << endl;
            } else {
                cout << "Failed to get stats for file: " << newPath << endl;
            }
        }
    }
}

//******************************************************************************
void fs::my_ls(const string& path){
    string pathStr = path;
    size_t pos = pathStr.find_last_of("\\");
    string filename = pathStr.substr(pos + 1);

    int parentInum;
    int inum = -1;
    vector<dentry> parentDentry;
    findParent(disk, path, parentDentry, parentInum);
    
    for (int i = 0; i < parentDentry[0].nEntries; i++) {
        if (parentDentry[i].fname == filename) {
            inum = parentDentry[i].inode;
            break;
        }
    }

    vector<dentry> entries;
    readDentry(disk, entries, inum);

    if (entries[0].nEntries > 2){
        for (int i = 2; i < entries[0].nEntries; i++) {
            string newPath = path + "\\" + entries[i].fname;
            struct stat fileStat;
            if (my_stat(newPath, fileStat)) {
                string timeStr = ctime(&fileStat.st_mtime);
                timeStr.erase(std::remove(timeStr.begin(), timeStr.end(), '\n'), timeStr.end());

                // Print the file details in a format similar to ls -l
                cout << ((S_ISDIR(fileStat.st_mode)) ? 'd' : '-')
                    << ((fileStat.st_mode & S_IRUSR) ? 'r' : '-')
                    << ((fileStat.st_mode & S_IWUSR) ? 'w' : '-')
                    << ((fileStat.st_mode & S_IXUSR) ? 'x' : '-')
                    << ((fileStat.st_mode & S_IRGRP) ? 'r' : '-')
                    << ((fileStat.st_mode & S_IWGRP) ? 'w' : '-')
                    << ((fileStat.st_mode & S_IXGRP) ? 'x' : '-')
                    << ((fileStat.st_mode & S_IROTH) ? 'r' : '-')
                    << ((fileStat.st_mode & S_IWOTH) ? 'w' : '-')
                    << ((fileStat.st_mode & S_IXOTH) ? 'x' : '-')
                    << ' ' << fileStat.st_nlink
                    << ' ' << fileStat.st_uid
                    << ' ' << fileStat.st_gid
                    << ' ' << fileStat.st_size
                    << ' ' << timeStr
                    << ' ' << entries[i].fname << endl;
            } else {
                cout << "Failed to get stats for file: " << newPath << endl;
            }
        }
    }

}

//******************************************************************************
void fs::my_cd(const string& name){
    // Parse and save given file name(file path)
    istringstream iss(name);
    string token;
    vector<string> pathComponents;

    // Use a while loop to tokenize the path and store components in the vector
    while (getline(iss, token, '\\')) {
        pathComponents.push_back(token);
    }

    for (int i = 0; i < pathComponents.size(); i++) {
        vector<dentry> entries;
        readDentry(disk, entries, curInum);
        // Check if the file exists in the current directory
        for (int j = 0; j < entries[0].nEntries; j++) {
            if (entries[j].fname == pathComponents[i]) {
                // The file exists, update current path and current inode
                curPath += "\\" + pathComponents[i];
                curInum = entries[j].inode;
                break;
            } 
        }
    }

    cout << "Current path: " << curPath << endl;

}

//******************************************************************************
int fs::my_mkdir(const string& path, mode_t mode) {
    return my_creat(path, mode | S_IFDIR);
}

//******************************************************************************
int fs::my_mkdir(mode_t mode) {
    return my_mkdir(curPath, mode | S_IFDIR);
}

//******************************************************************************
int fs::my_rmdir(const string& path){
    int rc = -1;

    // // Split the path into components
    // stringstream ss(path);
    // string token;
    // vector<string> pathComponents;
    // while (getline(ss, token, '/')) {
    //     pathComponents.push_back(token);
    // }

    // if (pathComponents.size() == 1){
    //     vector<dentry> entries;
    //     readDentry(disk, entries, curInum);
    // }

    string pathStr = path;
    size_t pos = pathStr.find_last_of("\\");
    string filename = pathStr.substr(pos + 1);

    int parentInum;
    int inum = -1;
    vector<dentry> parentDentry;
    findParent(disk, path, parentDentry, parentInum);

    // Check if the file exists in the parent directory
    for (int i = 0; i < parentDentry[0].nEntries; i++) {
        if (parentDentry[i].fname == filename) {
            inum = parentDentry[i].inode;
            rc = 0;
            break;
        }
    }

    if(rc == 0){
        // Read the inode from disk
        Inode inode;
        readInode(disk, inum, inode);

        // Check if the file is a directory
        if (inode.mode & S_IFDIR) {
            // Check if the directory is empty
            vector<dentry> entries;
            readDentry(disk, entries, inum);
            if (entries[0].nEntries == 2) {
                // Remove the directory entry from the parent directory
                for (int i = 0; i < parentDentry[0].nEntries; i++) {
                    if (parentDentry[i].fname == filename) {
                        parentDentry.erase(parentDentry.begin() + i);
                        parentDentry[0].nEntries--;
                        break;
                    }
                }

                // Write the updated parent directory entry to the disk
                writeDentry(disk, parentDentry, parentInum);

                // Free the inode and data block
                inodeBitmap.setBit(inum, false);
                blockBitmap.setBit(inode.blockAddress, false);
                blockBitmap.setBit(inode.blockAddress + 1, false);

            } else {
                cout << "Directory is not empty" << endl;
                throw runtime_error("Directory is not empty");
            }
        } else {
            cout << "File is not a directory" << endl;
            throw runtime_error("File is not a directory");
        }
    } else {
        cout << "rmdir File not found" << endl;
        throw runtime_error("File not found");
    }

    return rc;
}

//******************************************************************************
int fs::my_chown(const string& name, int owner, int group) {
    int rc = -1;

    string pathStr = name;
    size_t pos = pathStr.find_last_of("\\");
    string filename = pathStr.substr(pos + 1);

    int parentInum;
    int inum = -1;
    vector<dentry> parentDentry;
    findParent(disk, name, parentDentry, parentInum);

    // Check if the file exists in the parent directory
    for (int i = 0; i < parentDentry[0].nEntries; i++) {
        if (parentDentry[i].fname == filename) {
            inum = parentDentry[i].inode;
            rc = 0;
            break;
        }
    }

    if(rc == 0){
        // Read the inode from disk
        Inode inode;
        readInode(disk, inum, inode);

        // Change the owner and group
        inode.uid = owner;
        inode.gid = group;

        // Write the inode to disk
        writeInode(disk, inum, inode);
    } else {
        cout << "Chown File not found" << endl;
        throw runtime_error("File not found");
    }

    return rc;
}

//******************************************************************************
int fs::my_cp(const string& srcPath, const string& destPath){
    int rc = -1;
    int fd = my_open(srcPath.c_str(), S_IRUSR | S_IWUSR);
    if(fd != -1){
        struct stat fileStat;
        if(my_stat(srcPath, fileStat)){
            my_lseek(fd, 0, SEEK_SET);
            char buffer[fileStat.st_size];
            my_read(fd, buffer, fileStat.st_size);
            my_close(fd);
            int newFd = my_creat(destPath, S_IRUSR | S_IWUSR);
            if(fd != -1){
                Inode inode;
                readInode(disk, newFd, inode);
                my_lseek(newFd, 0, SEEK_SET);
                my_write(newFd, buffer, fileStat.st_size);
                my_close(newFd);
                rc = 0;
            } else {
                cout << "Failed to create destination file" << endl;
            }
        } else {
            cout << "Failed to get stats for file: " << srcPath << endl;
        }
    } else {
        cout << "Failed to open source file" << endl;
    }
    
    return rc;
}

//******************************************************************************
int fs::my_mv(const string& srcPath, const string& destPath){
    int rc = my_cp(srcPath, destPath);

    // Find the source file's parent directory and its dentry
    vector<dentry> srcParentDentries;
    int srcParentInum;
    findParent(disk, srcPath, srcParentDentries, srcParentInum);

    // Find the dentry for the source file and remove it
    int indexToRemove = -1;
    for (int i = 0; i < srcParentDentries[0].nEntries; ++i) {
        if (srcParentDentries[i].fname == srcPath) {
            indexToRemove = i;
            break;
        }
    }

    if (indexToRemove != -1) {
        srcParentDentries.erase(srcParentDentries.begin() + indexToRemove);
    }

    // Write the modified dentries back to the disk
    writeDentry(disk, srcParentDentries, srcParentInum);

    return rc;
}

//******************************************************************************
int fs::my_rm(const string& name){
    int rc = -1;

    string pathStr = name;
    size_t pos = pathStr.find_last_of("\\");
    string filename = pathStr.substr(pos + 1);

    int parentInum;
    int inum = -1;
    vector<dentry> parentDentry;
    findParent(disk, name, parentDentry, parentInum);

    // Check if the file exists in the parent directory
    for (int i = 0; i < parentDentry[0].nEntries; i++) {
        if (parentDentry[i].fname == filename) {
            inum = parentDentry[i].inode;
            rc = 0;
            break;
        }
    }

    if(rc == 0){
        // Read the inode from disk
        Inode inode;
        readInode(disk, inum, inode);

        // Check if the file is a file
        if (inode.mode & S_IFREG) {
            // Check if the directory is empty
            vector<dentry> entries;
            readDentry(disk, entries, inum);
            if (entries[0].nEntries == 2) {
                // Remove the directory entry from the parent directory
                for (int i = 0; i < parentDentry[0].nEntries; i++) {
                    if (parentDentry[i].fname == filename) {
                        parentDentry.erase(parentDentry.begin() + i);
                        parentDentry[0].nEntries--;
                        break;
                    }
                }

                // Write the updated parent directory entry to the disk
                writeDentry(disk, parentDentry, parentInum);

                // Free the inode and data block
                inodeBitmap.setBit(inum, false);
                blockBitmap.setBit(inode.blockAddress, false);
                blockBitmap.setBit(inode.blockAddress + 1, false);

            } else {
                cout << "Directory is not empty" << endl;
                throw runtime_error("Directory is not empty");
            }
        } else {
            cout << "File is not a file" << endl;
            throw runtime_error("File is not a file");
        }
    } else {
        cout << "rmdir File not found" << endl;
        throw runtime_error("File not found");
    }

    return rc;
}

//******************************************************************************
int fs::my_ln(const string& srcPath, const string& destPath){
    int rc = -1;
    string pathStr = srcPath;
    size_t pos = pathStr.find_last_of("\\");
    string filename = pathStr.substr(pos + 1);

    int parentInum;
    int inum = -1;
    vector<dentry> parentDentry;
    findParent(disk, srcPath, parentDentry, parentInum);
    // Check if the file exists in the parent directory
    for (int i = 0; i < parentDentry[0].nEntries; i++) {
        if (parentDentry[i].fname == filename) {
            inum = parentDentry[i].inode;
            break;
        }
    }
    
    Inode srcnode;
    readInode(disk, inum, srcnode);
    if(inum != -1){
        int fd = my_creat(destPath, srcnode.mode);
        if(fd != -1){
            srcnode.nlink++;
            writeInode(disk, inum, srcnode);

            Inode destnode;
            readInode(disk, fd, destnode);
            destnode.size = srcnode.size;
            destnode.mode = srcnode.mode;
            destnode.atime = srcnode.atime;
            destnode.ctime = srcnode.ctime;
            destnode.mtime = srcnode.mtime;
            destnode.uid = srcnode.uid;
            destnode.gid = srcnode.gid;
            destnode.blockAddress = srcnode.blockAddress;
            destnode.nlink = srcnode.nlink;
        
            writeInode(disk, fd, destnode);

            int unusedBit = destnode.blockAddress - FIRST_DATA_BLOCK + 1;
            blockBitmap.setBit(unusedBit, 1);
            
            my_close(fd);

            rc = fd;
        } else {
            cout << "Failed to create destination file" << endl;
        }
    } else {
        cout << "File is not a file" << endl;
    }    

    return rc;
}

// //******************************************************************************
// int fs::my_cat(const string& name){
//     int rc = -1;
//     return rc;
// }

// //******************************************************************************
// bool fs::my_lcp(const string& hostFilePath, const string& destPath) {
//     bool rc = true;
//     // Open the file from the host
//     ifstream hostFile(hostFilePath, ios::binary);
//     if (!hostFile) {
//         cout << "Failed to open source file" << endl;
//         rc = false;
//     }

//     // Create the destination file
//     if(rc){
//         int fd = my_creat(destPath, S_IRUSR | S_IWUSR);
//         if (fd < 0) {
//             cout << "Failed to create destination file" << endl;
//             rc = false;
//         }
//         if(rc){
//             // Copy the data
//             char buffer[BLOCK_SIZE];
//             while (hostFile.read(buffer, BLOCK_SIZE)) {
//                 if (my_write(fd, buffer, hostFile.gcount()) < 0) {
//                     cout << "Failed to write to destination file" << endl;
//                     rc = false;
//                 } else {
//                     my_close(fd);
//                 }
//             }
//         }
//     }

//     hostFile.close();

//     return true;
// }

