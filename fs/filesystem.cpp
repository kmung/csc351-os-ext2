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
#include "superblock.h"
#include "bitmap.h"

using namespace std;

fs::fs(string vhd_path, int uid) {
    // Write your own disk path here
    string devicePath = vhd_path;
    this->uid = uid;

    curPath = "root";
    curInum = 1;
    remainDatablocks = NDATA_BLOCKS;

    // Initialize the bitmaps
    inodeBitmap = Bitmap(NINODES);
    blockBitmap = Bitmap(NDATA_BLOCKS);

    // Create a filesystem object
    createDisk(devicePath);

    // Initialize the disk member variable
    disk.open(devicePath, ios::in | ios::out | ios::binary);
    if (!disk) {
        cerr << "Failed to open device: " << devicePath << endl;
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

    // the root directory
    my_creat("root", 0777 | S_IFDIR);
}

//******************************************************************************

fs::~fs() {

}

//******************************************************************************
void fs::writeInode(fstream& disk, int inodeNum, Inode& inode) {
    streampos curPosition = disk.tellp();

    // Check if the disk file is open
    if (!disk.is_open()) {
        throw invalid_argument("Disk file is not open\n");
    }

    // Calculate the position of the inode in the file
    int pos = FIRST_INODE * BLOCK_SIZE + (inodeNum * INODE_SIZE);

    // Seek to the position of the inode
    disk.seekp(pos, ios_base::beg);

    // Check if the seek failed
    if (disk.fail()) {
        throw invalid_argument("Failed to seek to inode\n");
    }

    // Write the inode data
    disk.write(reinterpret_cast<const char*>(&inode), INODE_SIZE);

    // Check if the write failed
    if (disk.fail()) {
        throw invalid_argument("Failed to write inode\n");
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
    strncpy(dotEntry.fname, ".", MAX_NAME_LEN);
    dotEntry.nEntries = 2;

    // Write the "." dentry to the data block
    disk.write(reinterpret_cast<const char*>(&dotEntry), DENTRY_SIZE);

    // Initialize a dentry structure for ".."
    dentry dotDotEntry;
    dotDotEntry.inode = parentInum;
    strncpy(dotDotEntry.fname, "..", MAX_NAME_LEN);
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
    while (getline(iss, token, '/')) {
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
    size_t pos = fileName.find_last_of("/");
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
        throw invalid_argument("Disk file is not open\n");
    }

    // Check if the file name is empty
    if (fileName.empty()) {
        throw invalid_argument("File name is empty\n");
    }

    // Check if the file already exists
    if (my_open(fileName.c_str(), mode) != -1) {
        throw invalid_argument("File already exists: " + fileName + "\n");
    }

    // Extract the file name from the file path
    size_t pos = fileName.find_last_of("//");
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
    Inode newInode{mode, inodeNum, 2, uid, 0, 0, time(nullptr), time(nullptr), time(nullptr), FIRST_DATA_BLOCK + blockNum};

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
        // If failed to find parent directory, free the inode and data block
        // Inode still remains on the disk, but it will be overwritten later
        inodeBitmap.setBit(inodeNum, false);
        blockBitmap.setBit(blockNum, false);
        blockBitmap.setBit(blockNum + 1, false);

        // Throw an error
        throw invalid_argument("Parent directory not found\n");
    }

    // Mark openFdTable to indicate the file is opened
    openFdTable.push_back(inodeNum);

    remainDatablocks -= 2;

    // Return the inode number as the file descriptor
    return inodeNum;
}

//******************************************************************************
int fs::my_open(const char *pathname, mode_t mode){
    int rc = -1;
    string pathStr = pathname;
    size_t pos = pathStr.find_last_of("/");
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
        throw invalid_argument("File descriptor not found\n");
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
    size_t pos = pathStr.find_last_of("/");
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
        throw invalid_argument("File not found\n");
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
            throw invalid_argument("Invalid whence value\n");
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
                if(dataAddress * BLOCK_SIZE <= curPosition && (dataAddress * BLOCK_SIZE + node.size) >= curPosition){
                    // If remain bytes to read is less than nbytes, read the remaining bytes
                    // Otherwise, read nbytes

                    if(nbytes > node.size + dataAddress * BLOCK_SIZE - curPosition){
                        rc = node.size + dataAddress * BLOCK_SIZE - curPosition;
                    } else {
                        rc = nbytes;
                    }

                    // Move the file pointer to the start of the file data
                    disk.seekg(dataAddress * BLOCK_SIZE, ios::beg);

                    // As reading starts from current position, do not move pointer
                    disk.read(buffer, rc);

                    // Null-terminate the buffer
                    buffer[rc] = '\0';

                    if(disk.fail()){
                        throw invalid_argument("Failed to read file\n");
                    }
                } else {
                    throw invalid_argument("Current position is not on the right spot\n");
                }
            } else {
                throw invalid_argument("File is not readable\n");
            }
        } else {
            throw invalid_argument("File is not opened\n");
        }
    } else {
        throw invalid_argument("Invalid file descriptor\n");
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

                    // // Update inode
                    // int overWriteSize;
                    // if((rc + BLOCK_SIZE + node.size - curWritePosition) > nbytes){
                    //     overWriteSize = nbytes;
                    // } else {
                    //     overWriteSize = rc + BLOCK_SIZE + node.size - curWritePosition;
                    // }
                    node.size += nbytes;
                    node.blockAddress = rc / 4096;
                    writeInode(disk, fd, node);

                    Inode newNode;
                    readInode(disk, fd, newNode);

                } else {
                    throw invalid_argument("Failed to allocate memory\n");
                }
            } else {
                throw invalid_argument("File is not writable\n");
            }
        } else {
            throw invalid_argument("File is not opened\n");
        }
    } else {
        throw invalid_argument("Invalid file descriptor\n");
    }

    return rc;
}

//******************************************************************************
string fs::my_ls(){
    stringstream ss;

    vector<dentry> entries;
    readDentry(disk, entries, curInum);

    if (entries[0].nEntries > 2){
        for (int i = 2; i < entries[0].nEntries; i++) {
            struct stat fileStat;
            string newPath = curPath + "/" + entries[i].fname;
            if (my_stat(newPath, fileStat)) {
                string timeStr = ctime(&fileStat.st_mtime);
                timeStr.erase(remove(timeStr.begin(), timeStr.end(), '\n'), timeStr.end());

                // Print the file details in a format similar to ls -l
                ss << ((S_ISDIR(fileStat.st_mode)) ? 'd' : '-')
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
                throw invalid_argument("Failed to get stats for file\n");
            }
        }
    }

    return ss.str();
}

//******************************************************************************
string fs::my_ls(const string& path){
    stringstream ss;

    string pathStr = path;
    size_t pos = pathStr.find_last_of("/");
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
            string newPath = path + "/" + entries[i].fname;
            struct stat fileStat;
            if (my_stat(newPath, fileStat)) {
                string timeStr = ctime(&fileStat.st_mtime);
                timeStr.erase(remove(timeStr.begin(), timeStr.end(), '\n'), timeStr.end());

                // Print the file details in a format similar to ls -l
                ss << ((S_ISDIR(fileStat.st_mode)) ? 'd' : '-')
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
                throw invalid_argument("Failed to get stats for file\n");
            }
        }
    }

    return ss.str();

}

//******************************************************************************
string fs::my_cd(const string& name){
    string absPath = getAbsolutePath(name);
    stringstream ss;

    // Parse and save given file name(file path)
    istringstream iss(absPath);
    string token;
    vector<string> pathComponents;

    while (getline(iss, token, '/')) {
        if (!token.empty()) {
            pathComponents.push_back(token);
        }
    }

    int temp = curInum;
    curInum = 0;
    bool find = false;

    if(name == ".."){
        if (curPath == "root") {
            curInum = 1;
            throw invalid_argument("Already at root directory\n");
        }

        istringstream iss(curPath);
        string token;
        vector<string> pathComponents;

        while (getline(iss, token, '/')) {
            if (!token.empty()) {
                pathComponents.push_back(token);
            }
        }

        for (int i = 0; i < pathComponents.size() - 1; i++) {
            vector<dentry> entries;
            readDentry(disk, entries, curInum);
            
            // Check if the file exists in the current directory
            for (int j = 1; j < entries[0].nEntries; j++) {
                
                if (entries[j].fname == pathComponents[i]) {
                    if(curInum != 0){
                        // The file exists, update current path and current inode
                        curPath += "/" + pathComponents[i];
                    } else {
                        curPath = "root";
                    }
                    curInum = entries[j].inode;
                    find = true;
                    break;
                } 
            }
            if(!find){
                curInum = temp;
                throw invalid_argument("File not found\n");
            }
        }

        ss << curPath << endl;
        return ss.str();
    }

    for (int i = 0; i < pathComponents.size(); i++) {
        vector<dentry> entries;
        readDentry(disk, entries, curInum);
        
        // Check if the file exists in the current directory
        for (int j = 1; j < entries[0].nEntries; j++) {
            
            if (entries[j].fname == pathComponents[i]) {
                if(curInum != 0){
                    // The file exists, update current path and current inode
                    curPath += "/" + pathComponents[i];
                } else {
                    curPath = "root";
                }
                curInum = entries[j].inode;
                find = true;
                break;
            } 
        }
        if(!find){
            curInum = temp;
            throw invalid_argument("File not found\n");
        }
    }

    ss << curPath << endl;

    return ss.str();
}

//******************************************************************************
string fs::my_cd(){
    curPath = "root";
    curInum = 1;
    
    return curPath;
}

//******************************************************************************
int fs::my_mkdir(const vector<string>& path) {
    int rc = -1;
    for(int i = 1; i < path.size(); i++){
        string absPath = getAbsolutePath(path[i]);

        string pathStr = absPath;
        size_t pos = pathStr.find_last_of("/");
        string filename = pathStr.substr(pos + 1);

        rc = my_creat(absPath, 0644 | S_IFDIR);
        
    }
    return rc;
}


//******************************************************************************
int fs::my_rmdir(const vector<string>& path){
    int rc = -1;
    for(int i = 1; i < path.size(); i++){
        string absPath = getAbsolutePath(path[i]);

        string pathStr = absPath;
        size_t pos = pathStr.find_last_of("/");
        string filename = pathStr.substr(pos + 1);

        int parentInum;
        int inum = -1;
        vector<dentry> parentDentry;
        findParent(disk, absPath, parentDentry, parentInum);

        Inode parentInode;
        readInode(disk, parentInum, parentInode);
        int octalMode = std::stoi(std::to_string(parentInode.mode), nullptr, 8);
        
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
            struct stat fileStat;
            my_stat(absPath, fileStat);

            if(inode.uid != uid){
                throw invalid_argument("Permission denied\n");
            }

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
                    throw invalid_argument("Directory is not empty\n");
                }
            } else {
                throw invalid_argument("File is not a directory\n");
            }
        } else {
            throw invalid_argument("File not found\n");
        }
    }

    return rc;
}

//******************************************************************************
int fs::my_chown(const string& name, int owner, int group) {
    int rc = -1;
    string absPath = getAbsolutePath(name);

    string pathStr = absPath;
    size_t pos = pathStr.find_last_of("/");
    string filename = pathStr.substr(pos + 1);

    int parentInum;
    int inum = -1;
    vector<dentry> parentDentry;
    findParent(disk, absPath, parentDentry, parentInum);

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

        if(inode.uid != uid){
            throw invalid_argument("Permission denied\n");
            
        }

        // Change the owner and group
        inode.uid = owner;
        inode.gid = group;

        // Write the inode to disk
        writeInode(disk, inum, inode);
    } else {
        throw invalid_argument("File not found\n");
    }

    return rc;
}

//******************************************************************************
int fs::my_cp(const string& srcPath, const string& destPath){
    int rc = -1;
    string absSrcPath = getAbsolutePath(srcPath);
    string absDestPath = getAbsolutePath(destPath);
    int fd = my_open(absSrcPath.c_str(), S_IRUSR | S_IWUSR);
    if(fd != -1){
        struct stat fileStat;
        if(my_stat(absSrcPath, fileStat)){
            if (fileStat.st_mode & S_IFDIR) {
                throw invalid_argument("Cannot copy directory\n");
                
            }

            if(!my_lseek(fd, 0, SEEK_SET)){
                throw invalid_argument("Failed to move file pointer\n");
                
            }

            if(fileStat.st_uid != uid){
                throw invalid_argument("Permission denied\n");
                
            }

            if(fileStat.st_size > remainDatablocks * 4096){
                throw invalid_argument("Not enough space\n");
                
            } else {
                remainDatablocks -= (fileStat.st_size / 4096) + 1;
            }

            

            if(fileStat.st_size > 1024){
                vector<char> buffer(fileStat.st_size);
                my_read(fd, buffer.data(), fileStat.st_size);
                my_close(fd);

                int newFd = my_creat(absDestPath, S_IRUSR | S_IWUSR);
                    if(fd != -1){
                    Inode inode;
                    readInode(disk, newFd, inode);
                    my_lseek(newFd, 0, SEEK_SET);

                    my_write(newFd, buffer.data(), fileStat.st_size);
                    my_close(newFd);
                    rc = 0;
                } else {
                    throw invalid_argument("Failed to create destination file\n");
                }
            } else {
                    char buffer[fileStat.st_size];
                    my_read(fd, buffer, fileStat.st_size);
                    my_close(fd);

                    int newFd = my_creat(absDestPath, S_IRUSR | S_IWUSR);
                    if(fd != -1){
                    Inode inode;
                    readInode(disk, newFd, inode);
                    my_lseek(newFd, 0, SEEK_SET);

                    
                    my_write(newFd, buffer, fileStat.st_size);
                    my_close(newFd);
                    rc = 0;
                } else {
                    throw invalid_argument("Failed to create destination file\n");
                }
            }
            
        } else {
            throw invalid_argument("Failed to get stats for file: " + absSrcPath + "\n");
        }
    } else {
        throw invalid_argument("Failed to open source file\n");
    }
    
    return rc;
}

//******************************************************************************
int fs::my_mv(const string& srcPath, const string& destPath){
    string absSrcPath = getAbsolutePath(srcPath);
    string absDestPath = getAbsolutePath(destPath);
    int rc = my_cp(srcPath, destPath);

    if(rc == -1){
        throw invalid_argument("Failed to copy file\n");
    }

    string pathStr = absSrcPath;
    size_t pos = pathStr.find_last_of("/");
    string filename = pathStr.substr(pos + 1);

    // Find the source file's parent directory and its dentry
    vector<dentry> srcParentDentries;
    int srcParentInum;
    int srcInum = -1;
    findParent(disk, absSrcPath, srcParentDentries, srcParentInum);

    // Find the dentry for the source file and remove it
    int indexToRemove = -1;
    for (int i = 0; i < srcParentDentries[0].nEntries; ++i) {
        if (srcParentDentries[i].fname == filename) {
            srcInum = srcParentDentries[i].inode;
            indexToRemove = i;
            break;
        }
    }

    Inode srcInode;
    readInode(disk, srcInum, srcInode);

    if(srcInode.uid != uid){
        throw invalid_argument("Permission denied\n");
        
    }


    if (indexToRemove != -1) {
        srcParentDentries.erase(srcParentDentries.begin() + indexToRemove);
        srcParentDentries[0].nEntries--;
    }

    // Write the modified dentries back to the disk
    writeDentry(disk, srcParentDentries, srcParentInum);

    return rc;
}

//******************************************************************************
int fs::my_rm(const vector<string>& name){
    int rc = -1;
    

    for(int i = 1; i < name.size(); i++){
        string absPath = getAbsolutePath(name[i]);
        string pathStr = absPath;
        size_t pos = pathStr.find_last_of("/");
        string filename = pathStr.substr(pos + 1);

        int parentInum;
        int inum = -1;
        vector<dentry> parentDentry;
        findParent(disk, absPath, parentDentry, parentInum);

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
            if (inode.mode & S_IFDIR) {
                throw invalid_argument("Cannot delete directory\n");
                
            }

            if(inode.uid != uid){
                throw invalid_argument("Permission denied\n");
                
            }

            // Check if the file is a file
            if (inode.mode) {
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
                    throw invalid_argument("Directory is not empty\n");
                }
            } else {
                throw invalid_argument("File is not a file\n");
            }
        } else {
            throw invalid_argument("File not found\n");
        }
    }

    return rc;
}

//******************************************************************************
int fs::my_ln(const string& srcPath, const string& destPath){
    int rc = -1;
    string absSrcPath = getAbsolutePath(srcPath);
    string absDestPath = getAbsolutePath(destPath);
    string pathStr = absSrcPath;
    size_t pos = pathStr.find_last_of("/");
    string filename = pathStr.substr(pos + 1);

    int parentInum;
    int inum = -1;
    vector<dentry> parentDentry;
    findParent(disk, absSrcPath, parentDentry, parentInum);
    // Check if the file exists in the parent directory
    for (int i = 0; i < parentDentry[0].nEntries; i++) {
        if (parentDentry[i].fname == filename) {
            inum = parentDentry[i].inode;
            break;
        }
    }
    
    Inode srcnode;
    readInode(disk, inum, srcnode);
    if(srcnode.uid != uid){
        throw invalid_argument("Permission denied\n");
    }

    if (srcnode.mode & S_IFDIR) {
        throw invalid_argument("Cannot link directory\n");
    }

    if(inum != -1){
        int fd = my_creat(absDestPath, srcnode.mode);
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
            destnode.nlink--;
        
            writeInode(disk, fd, destnode);

            int unusedBit = destnode.blockAddress - FIRST_DATA_BLOCK + 1;
            blockBitmap.setBit(unusedBit, 1);
            
            my_close(fd);

            rc = fd;
        } else {
            throw invalid_argument("Failed to create destination file\n");
        }
    } else {
        throw invalid_argument("File not found\n");
    }    

    return rc;
}

//******************************************************************************
string fs::my_cat(const vector<string>& srcPath){
    stringstream ss;

    for(int i = 1; i < srcPath.size(); i++){
        string absSrcPath = getAbsolutePath(srcPath[i]);
        int fd = my_open(absSrcPath.c_str(), S_IRUSR | S_IWUSR);
        if(fd != -1){
            struct stat fileStat;
            if(my_stat(absSrcPath, fileStat)){
                my_lseek(fd, 0, SEEK_SET);
                char buffer[fileStat.st_size];
                my_read(fd, buffer, fileStat.st_size);
                my_close(fd);
                ss << buffer;
            } else {
                throw invalid_argument("Failed to get stats for file: " + absSrcPath + "\n");
            }
        } else {
            throw invalid_argument("Failed to open source file\n");
        }
    }

    return ss.str();
}

//******************************************************************************
int fs::my_lcp(const string& srcPath, const string& destPath) {
    string absDestPath = getAbsolutePath(destPath);
    // Open the source file on the local filesystem
    ifstream srcFile(srcPath, ios::binary);
    if (!srcFile) {
        throw invalid_argument("Failed to open source file\n");
        
    }
    // Get the size of the source file
    srcFile.seekg(0, ios::end);
    int size = srcFile.tellg();
    srcFile.seekg(0, ios::beg);

    // Read the source file into a buffer
    vector<char> buffer(size);
    if (!srcFile.read(buffer.data(), size)) {
        throw invalid_argument("Failed to read source file\n");
        
    }
    // Close the source file
    srcFile.close();

    if(size > remainDatablocks * 4096){
        throw invalid_argument("Not enough space\n");
        
    } else {
        remainDatablocks -= (size / 4096) + 1;
    }

    // Open the destination file on the custom filesystem
    int fd = my_creat(absDestPath.c_str(), S_IRUSR | S_IWUSR);
    if (fd == -1) {
        throw invalid_argument("Failed to create destination file\n");
        
    }
    
    // Seek to the end of the destination file
    my_lseek(fd, 0, SEEK_SET);

    // Write the buffer to the destination file
    if(my_write(fd, buffer.data(), size) == -1){
        throw invalid_argument("Failed to write to destination file\n");
        
    }

    // Close the destination file
    my_close(fd);

    return 0;
}

//******************************************************************************
int fs::my_Lcp(const string& srcPath, const string& destPath) {
    string absSrcPath = getAbsolutePath(srcPath);

    // Open the source file on the custom filesystem
    int fd = my_open(absSrcPath.c_str(), S_IRUSR);
    if (fd == -1) {
        throw invalid_argument("Failed to open source file\n");
        
    }

    // Get the size of the source file
    readInode(disk, fd, inode);
    int size = inode.size;

    // Read the source file into a buffer
    my_lseek(fd, 0, SEEK_SET);
    vector<char> buffer(size);
    my_read(fd, buffer.data(), size);

    // Close the source file
    my_close(fd);

    // Check if the destination file exists on the host filesystem
    ifstream testFile(destPath);
    if (testFile.good()) {
        throw invalid_argument("Destination file already exists\n");
        
    }
    testFile.close();

    // Create a new file on the host filesystem
    ofstream destFile(destPath, ios::binary);
    if (!destFile) {
        throw invalid_argument("Failed to create destination file\n");
        
    }

    // Write the buffer to the new file
    if (!destFile.write(buffer.data(), size)) {
        throw invalid_argument("Failed to write to destination file\n");
        
    }

    // Close the destination file
    destFile.close();

    return 0;
}

// get current working directory
string fs::my_getcwd(){
    return curPath;
}

string fs::getAbsolutePath(const string& path) {
    if (path.substr(0, 4) == "root") {
        return path; // path is already absolute
    } else {
        return curPath + "/" + path; // prepend the current working directory
    }
}