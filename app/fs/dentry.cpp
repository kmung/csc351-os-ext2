#include "dentry.h"
#include <iostream>
#include <fstream>

using namespace std;

//******************************************************************************
DirectoryEntry::DirectoryEntry(const string& filename, unsigned int inodeNumber) {
    this->filename = filename;
    this->inodeNumber = inodeNumber;
}

//******************************************************************************
const string& DirectoryEntry::getFilename() const {
    return filename;
}

//******************************************************************************
unsigned int DirectoryEntry::getInodeNumber() const {
    return inodeNumber;
}

//******************************************************************************
void Directory::addEntry(const std::string& filename, unsigned int inodeNumber) {
    DirectoryEntry entry(filename, inodeNumber);
    entries.push_back(entry);
}

//******************************************************************************
void Directory::removeEntry(const string &filename) {
    entries.erase(remove_if(entries.begin(), entries.end(),[filename](
        const DirectoryEntry &entry) {
            return entry.getFilename() == filename;
        }), entries.end());
}

//******************************************************************************
void Directory::listEntries() const {
    for (const DirectoryEntry& entry : entries) {
        cout << "Filename: " << entry.getFilename() << " Inode: " << entry.getInodeNumber() << endl;
    }
}