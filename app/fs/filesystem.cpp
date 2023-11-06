#include <iostream>
#include <fstream>

#include "filesystem.h"

using namespace std;

//******************************************************************************
FileSystem::FileSystem(const string& devicePath) : devicePath(devicePath), isMounted(false) {
    // Initialize other data members
}

//******************************************************************************
bool FileSystem::mount() {
    // Implement file system mounting logic
    bool rc = false;

    if (!isMounted) {
        // Open the device for reading and writing.
        ifstream device(devicePath, ios::in | ios::out | ios::binary);
        if (device.is_open()) {
            // If device is open, continue to mount
            if (readSuperblock() && initializeBlockGroups()) {
                // File system is successfully mounted
                isMounted = true;
                rc = true;
            } else {
                device.close();
            }
        } else {
            cerr << "Failed to open the device." << endl;
        }
    } else {
        cerr << "File system is already mounted" << endl;
    }



    return rc;    
}

//******************************************************************************
bool FileSystem::unmount() {
    // Implement file system unmounting logic
    isMounted = false;
    return true;
}

//******************************************************************************
bool FileSystem::createFile(const std::string& filename, int fileSize) {
    bool rc = false;
    if (!isMounted) {
        // File system is not mounted, return false or throw an exception
        cerr << "Cannot create a file. The file system is not mounted." << endl;
    } else {
        // Step 1: Allocate an available inode
        int newInodeNumber;
        allocateInode(newInodeNumber);

        // Step 2: Create a new Inode for the file
        Inode *newInode = new Inode(newInodeNumber);
        newInode->setSize(fileSize);  // Set the file size
        // Set permissions, timestamps, and other inode properties as needed

        // Step 3: Reserve data blocks for the file content
        vector<unsigned int> dataBlockNumbers = allocateDataBlocks(fileSize);

        // Step 4: Update the new Inode with data block pointers
        newInode->setDataBlockPointers(dataBlockNumbers);

        // Step 5: Update the directory entry for the new file
        // This involves adding a new DirectoryEntry in the parent directory
        if (addDirectoryEntry(filename, newInodeNumber)) {
            // Step 6: Update the superblock and inode table to reflect changes

            // Step 7: Write the new Inode and data blocks to disk

            rc = true; // File creation was successful
        }

        // If any step fails, you should handle error conditions and cleanup as needed.
        // This includes deallocating any allocated resources in case of failure.
    }

    

    return rc; // File creation failed
}
//******************************************************************************
// TO BE IN SUPERBLOCK
Superblock readSuperblock() {
    Superblock superblock;

    // Determine the location of the superblock on the storage medium
    unsigned int superblockBlockNumber = /* calculate based on file system format */;

    // Read the superblock data from the specified block
    readBlock(superblockBlockNumber, &superblock, sizeof(Superblock));

    // Perform validation and error handling here if necessary

    return superblock;
}

Superblock getInodeBitmapBlockCount() {
    return inodeCount;
}

Superblock getBlockSize() {
    return blockSize;
}

Superblock getInodeBitmapBlock() {
    return inodeBlockNum
}


