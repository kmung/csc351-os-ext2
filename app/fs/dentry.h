#ifndef DENTRY_H
#define DENTRY_H
#include <sys/stat.h>
#include <stdlib.h>
#define EXT2_NAME_LEN 256

#pragma once
#include <string>

class DirectoryEntry {
	private:
		string filename;
		unsigned int inodeNumber;
	public:
		DirectoryEntry(const string& filename, unsigned int inodeNumber);
		
		const string& getFilename() const;
		unsigned int getInodeNumber() const;
};

class Directory {
	private:
		std::vector<DirectoryEntry> entries;
	public:
		void addEntry(const string& filename, unsigned int inodeNumber);
		void removeEntry(const string &filename);
		void listEntries() const;
};


#endif