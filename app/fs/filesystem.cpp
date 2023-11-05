#include <iostream>
#include <fstream>

#include "filesystem.h"
#include "disk.h"
#include "SuperBlock.h"

using namespace std;

//******************************************************************************
FileSystem::FileSystem(const string& devicePath) : devicePath(devicePath), isMounted(false) {
    mount();
}

//******************************************************************************
bool FileSystem::mount() {
    // Implement file system mounting logic
    isMounted = true;

    // Initialize other data members
    createDisk(devicePath);
    
    return true;
}

//******************************************************************************
bool FileSystem::unmount() {
    // Implement file system unmounting logic
    isMounted = false;
    return true;
}
//******************************************************************************
bool FileSystem::allocateInode(int &inodeNumber) {
    SuperBlock SuperBlock = readSuperBlock();

    // Read the inode bitmap block from disk
    char inodeBitmap[SuperBlock.getInodeBitmapBlockCount() * SuperBlock.getBlockSize()];
    readBlock(SuperBlock.getInodeBitmapBlock(), inodeBitmap);
}

//******************************************************************************
bool FileSystem::createFile(const std::string& filename, int fileSize) {
    bool rc = false;
    if (!isMounted) {
        // File system is not mounted, return false or throw an exception
        cerr << "Cannot create a file. The file system is not mounted." << endl;
        return false;
    }

    // Step 1: Allocate an available inode
    int newInodeNumber;
    allocateInode(newInodeNumber);

    // Step 2: Create a new Inode for the file
    Inode newInode;
    newInode.setSize(fileSize);  // Set the file size
    // Set permissions, timestamps, and other inode properties as needed

    // Step 3: Reserve data blocks for the file content
    vector<unsigned int> dataBlockNumbers = allocateDataBlocks(fileSize);

    // Step 4: Update the new Inode with data block pointers
    newInode.setDataBlockPointers(dataBlockNumbers);

    // Step 5: Update the directory entry for the new file
    // This involves adding a new DirectoryEntry in the parent directory
    if (addDirectoryEntry(filename, newInodeNumber)) {
        // Step 6: Update the SuperBlock and inode table to reflect changes

        // Step 7: Write the new Inode and data blocks to disk

        rc = true; // File creation was successful
    }

    // If any step fails, you should handle error conditions and cleanup as needed.
    // This includes deallocating any allocated resources in case of failure.

    return false; // File creation failed
}
//******************************************************************************
// TO BE IN SuperBlock
SuperBlock readSuperBlock(const string& devicePath) {
    SuperBlock SuperBlock;

    // The SuperBlock is the first block in the file system
    unsigned int SuperBlockBlockNumber = 0;

    // Read the SuperBlock data from the specified block
    readBlock(devicePath, SuperBlockBlockNumber, &SuperBlock, sizeof(SuperBlock));

    // Perform validation and error handling here if necessary

    return SuperBlock;
}

SuperBlock getInodeBitmapBlockCount() {
    return inodeCount;
}

SuperBlock getBlockSize() {
    return blockSize;
}

SuperBlock getInodeBitmapBlock() {
    return inodeBlockNum
}

