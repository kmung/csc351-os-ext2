// Josiah Branch

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

#include "fs.h"

fs::fs() {

}

//******************************************************************************

fs::~fs() {

}

//******************************************************************************

int fs::my_read(int fd, char* buffer, int nbytes) {
    // Use the read function with the file descriptor to read data.
    int bytesRead = read(fd, buffer, nbytes);
    if (bytesRead == -1) {
        // Handle the error, e.g., print an error message.
        std::cerr << "Error while reading from file descriptor." << std::endl;
    }

    return bytesRead;
}

//******************************************************************************

int fs::my_write(int fd, const char* buffer, int nbytes) {
    // Use the write function with the file descriptor to write data.
    int bytesWritten = write(fd, buffer, nbytes);

    if (bytesWritten == -1) {
        // Handle the error, e.g., print an error message.
        std::cerr << "Error while writing to file descriptor." << std::endl;
    }

    return bytesWritten;
}

//******************************************************************************

int fs::my_creat(const string& name, int mode) {
    int fd = open(name.c_str(), O_CREAT | O_WRONLY | O_TRUNC, mode);
    int rc = 0;

    if (fd == -1) {
        // Handle the error, e.g., print an error message.
        std::cerr << "Error creating file." << std::endl;
        rc = -1;
    }

    // Successfully created the file.
    close(fd); // Close the file descriptor.
    return rc;
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

//******************************************************************************

