#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <stdexcept> 
#include <string> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "filesystem.h"
#include "disk.h"
#include "SuperBlock.h"

using namespace std;

fs::fs() {

}

//******************************************************************************

fs::~fs() {

}

//******************************************************************************
int fs::my_read(int fd, char* buffer, int nbytes) {
    int bytesRead;
    try {
        bytesRead = read(fd, buffer, nbytes);
        if (bytesRead == -1) {
            throw exception();
        }
    } catch (...) {
        bytesRead = 0;
    }

    return bytesRead;
}

//******************************************************************************

int fs::my_write(int fd, const char* buffer, int nbytes) {
    // Use the write function with the file descriptor to write data.
    int bytesWritten;
    try{
        bytesWritten = write(fd, buffer, nbytes);
        if (bytesWritten == -1) {
            throw exception();
        }
    } catch (...) {
        bytesWritten = 0;
    }

    return bytesWritten;
}

//******************************************************************************

int fs::my_creat(const string& name, int mode) {
    int fd;
    try{
        fd = creat(name.c_str(), mode);
        if (fd == -1) {
            throw exception();
        }
    } catch (...) {
        fd = 0;
    }
    
    return fd;
}

//******************************************************************************

int fs::my_Iseek(int fd, off_t offset, int when) {
    // Use the lseek function with the file descriptor to seek.
    off_t result = lseek(fd, offset, when);
    int rc = 0;
    if (result == -1) {
        // Handle the error, e.g., print an error message.
        std::cerr << "Seek failed" << std::endl;
        rc = -1;
    }

    return rc; 
}

//******************************************************************************

int my_fstat(int fd, struct stat* buf) {
    // Call the fstat system call to retrieve file status information.
    int rc = 0;
    if (fstat(fd, buf) == -1) {
        // If fstat returns -1, an error occurred.
        std::cerr << "Error: Failed to retrieve file status." << std::endl;
        rc = -1; 
    }

    // Successfully retrieved file status information.
    return rc; 
}


//******************************************************************************

int fs::my_open(const char *pathname, mode_t mode) {
    //try opening the file first
    int fd = open(pathname, mode);

    if (fd == -1) {
		//using the c library, we can error handle and see if the most recent
		//function call occured an error and is set to defined numerical value
		//recognized by the OS  to show that the file doesn't exist
		if (errno == ENOENT) { 
			fd = 0;
			cout << "ERROR! File does not exist!" << endl;
		} else { //another possible error
			cout << "Cannot open file!" << endl;
		}
    } else {
		//open call was successful
		cout << "File," << pathname << "is open with descriptor" << fd << "." << endl;
	}

	return fd;
}

//******************************************************************************

int fs::my_close(int fd) {
	int errC;
	//check if file descriptor is valid since it can only be 0,1 or 2
	if (fd < 0) {
		cout << "Invalid file descriptor!" << endl;
	} else {
		errC = close(fd);
		if (errC == -1) {
		//using the c library, we can error handle and see if the most recent
		//function call occured an error and is set to defined numerical value
		//recognized by the OS  to show that the file doesn't exist
			if (errno == ENOENT) { 
				cout << "ERROR! File does not exist!" << endl;
			} else { //another possible error
				cout << "Cannot close file!" << endl;
			}
    	} else {
			cout << "File closed successfully!" << endl;
			errC = 0;
		}
	}

	return errC;
}

//******************************************************************************

int fs::my_stat(const string& name, struct stat& buf) {
    // Call the stat system call to retrieve file status information for the specified path.
    int rc = 0;
    if (stat(name.c_str(), &buf) == -1) {
        // If stat returns -1, an error occurred.
        std::cerr << "Error: Failed to retrieve file status for '" << name << "'" << std::endl;
        rc = -1; 
    }

    // Successfully retrieved file status information.
    return rc; 
}

