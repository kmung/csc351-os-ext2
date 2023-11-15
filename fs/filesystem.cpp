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
#include "superblock.h"
#include "bitmap.h"

using namespace std;

fs::fs() {
    string devicePath = "C:/Users/ssyak/Downloads/disk.vhd";

    // Create a filesystem object
    createDisk(devicePath);

    // Initialize the disk member variable
    disk.open(devicePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!disk) {
        std::cerr << "Failed to open device: " << devicePath << std::endl;
    }

    // Initialize all contents of the disk
    readSuperBlock(disk, sb);
    readInode(disk, 0, inode);
    readDentry(disk, rootEntry, 0);
    readInodeBitmap(disk, inodeBitmap);
    readBlockBitmap(disk, blockBitmap);
}

//******************************************************************************

fs::~fs() {

}

//******************************************************************************

int fs::my_creat(const string& fileName, mode_t mode) {
    // Find a free inode
    int inodeNum = inodeBitmap.findFirstFree();
    int blockNum = blockBitmap.findFirstFree();

    // Initialize the inode
    Inode newInode{mode, inodeNum, 2, 1000, 1000, 0, time(nullptr), time(nullptr), time(nullptr), FIRST_DATA_BLOCK + blockNum};

    // Write the inode to disk
    writeInode(disk, inodeNum, newInode);

    // Mark the inode and data block as used if they found a free one
    if(inodeNum != -1){
        inodeBitmap.setBit(inodeNum, true);
    }
    if(blockNum != -1){
        blockBitmap.setBit(blockNum, true);
        blockBitmap.setBit(blockNum, true);
    }

    // Update parent directory to include the entry for the new file
    int parentInum = -1;
    updateDentry(disk, fileName, parentInum, inodeNum);

    // Write the new dentry for the new file
    writeStartDentry(disk, inodeNum, parentInum);

    // Check what parent directory contains
    // Remove this part if check is not needed
    vector<dentry> parentDentry;
    readDentry(disk, parentDentry, parentInum);

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
    int pos = FIRST_INODE * BLOCK_SIZE + inodeNum * INODE_SIZE;

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
    disk.seekp(node.blockAddress * BLOCK_SIZE, ios_base::beg);

    // Initialize a dentry structure for "."
    dentry dotEntry;
    dotEntry.inode = inum;
    strncpy(dotEntry.fname, ".", MAX_NAME_LEN);
    dotEntry.nEntries = 2;

    // Write the "." dentry to the data block
    disk.write(reinterpret_cast<const char*>(&dotEntry), sizeof(dentry));

    // Initialize a dentry structure for ".."
    dentry dotDotEntry;
    dotDotEntry.inode = parentInum;
    strncpy(dotDotEntry.fname, "..", MAX_NAME_LEN);
    dotDotEntry.nEntries = 2;

    // Write the ".." dentry to the data block
    disk.write(reinterpret_cast<const char*>(&dotDotEntry), sizeof(dentry));
}

//******************************************************************************
void fs::updateDentry(fstream& disk, string fileName, int& parentInum, int currentInum) {
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

    // Read the root directory entry to start searching for the file
    vector<dentry> currentEntry;
    readDentry(disk, currentEntry, 0);

    // Search for the file in the root directory if the path has at least one subdirectory
    if(pathComponents.size() > 1){
        // Stop at the last parent directory entry
        for (int i = 0; i < pathComponents.size() - 1; i++) {
            for (int j = 0; j < currentEntry[0].nEntries; j++) {
                // Check if the current dentry contains the entry for the file path component
                if (currentEntry[j].fname == pathComponents[i]) {
                    parentInum = currentEntry[j].inode;

                    // For testing, remove it later
                    cout << "Find " << currentEntry[j].fname << ": " << currentEntry[j].inode << endl << endl;

                    // Clear the current entry and update to the next directory entry
                    currentEntry.clear();
                    readDentry(disk, currentEntry, parentInum);
                    break;
                } else {
                    // For testing, remove it later
                    cout << pathComponents[i] << " not found" << endl;
                }
            }
        }
    }

    // Create the new dentry for the file that will be added to the parent directory
    dentry newEntry;
    newEntry.inode = currentInum;
    strncpy(newEntry.fname, pathComponents.back().c_str(), MAX_NAME_LEN);
    newEntry.nEntries = 2;

    // Add the new dentry to the parent directory
    currentEntry.push_back(newEntry);
    currentEntry[0].nEntries++;
    
    // Write the updated parent directory entry to the disk
    writeDentry(disk, currentEntry, parentInum);
}



// //******************************************************************************
// int fs::my_read(int fd, char* buffer, int nbytes) {
//     int bytesRead;
//     try {
//         bytesRead = read(fd, buffer, nbytes);
//         if (bytesRead == -1) {
//             throw exception();
//         }
//     } catch (...) {
//         bytesRead = 0;
//     }

//     return bytesRead;
// }

// //******************************************************************************

// int fs::my_write(int fd, const char* buffer, int nbytes) {
//     // Use the write function with the file descriptor to write data.
//     int bytesWritten;
//     try{
//         bytesWritten = write(fd, buffer, nbytes);
//         if (bytesWritten == -1) {
//             throw exception();
//         }
//     } catch (...) {
//         bytesWritten = 0;
//     }

//     return bytesWritten;
// }

// //******************************************************************************

// int fs::my_Iseek(int fd, off_t offset, int when) {
//     // Use the lseek function with the file descriptor to seek.
//     off_t result = lseek(fd, offset, when);
//     int rc = 0;
//     if (result == -1) {
//         // Handle the error, e.g., print an error message.
//         std::cerr << "Seek failed" << std::endl;
//         rc = -1;
//     }

//     return rc; 
// }

// //******************************************************************************

// int my_fstat(int fd, struct stat* buf) {
//     // Call the fstat system call to retrieve file status information.
//     int rc = 0;
//     if (fstat(fd, buf) == -1) {
//         // If fstat returns -1, an error occurred.
//         std::cerr << "Error: Failed to retrieve file status." << std::endl;
//         rc = -1; 
//     }

//     // Successfully retrieved file status information.
//     return rc; 
// }


// //******************************************************************************

// int fs::my_open(const char *pathname, mode_t mode) {
//     //try opening the file first
//     int fd = open(pathname, mode);

//     if (fd == -1) {
// 		//using the c library, we can error handle and see if the most recent
// 		//function call occured an error and is set to defined numerical value
// 		//recognized by the OS  to show that the file doesn't exist
// 		if (errno == ENOENT) { 
// 			fd = 0;
// 			cout << "ERROR! File does not exist!" << endl;
// 		} else { //another possible error
// 			cout << "Cannot open file!" << endl;
// 		}
//     } else {
// 		//open call was successful
// 		cout << "File," << pathname << "is open with descriptor" << fd << "." << endl;
// 	}

// 	return fd;
// }

// //******************************************************************************

// int fs::my_close(int fd) {
// 	int errC;
// 	//check if file descriptor is valid since it can only be 0,1 or 2
// 	if (fd < 0) {
// 		cout << "Invalid file descriptor!" << endl;
// 	} else {
// 		errC = close(fd);
// 		if (errC == -1) {
// 		//using the c library, we can error handle and see if the most recent
// 		//function call occured an error and is set to defined numerical value
// 		//recognized by the OS  to show that the file doesn't exist
// 			if (errno == ENOENT) { 
// 				cout << "ERROR! File does not exist!" << endl;
// 			} else { //another possible error
// 				cout << "Cannot close file!" << endl;
// 			}
//     	} else {
// 			cout << "File closed successfully!" << endl;
// 			errC = 0;
// 		}
// 	}

// 	return errC;
// }

// //******************************************************************************

// int fs::my_stat(const string& name, struct stat& buf) {
//     // Call the stat system call to retrieve file status information for the specified path.
//     int rc = 0;
//     if (stat(name.c_str(), &buf) == -1) {
//         // If stat returns -1, an error occurred.
//         std::cerr << "Error: Failed to retrieve file status for '" << name << "'" << std::endl;
//         rc = -1; 
//     }

//     // Successfully retrieved file status information.
//     return rc; 
// }

