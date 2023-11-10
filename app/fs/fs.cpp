//fs.cpp

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cerrno>
#include <sys/stat.h>  //to define the stat structure
#include <unistd.h>
#include <sys/types.h>
#include <ctime>
#define st_mtime st_mtimespec.tv_sec
#define st_ctime st_ctimespec.tv_sec
#define st_ctime st_ctimespec.tv_sec



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

int my_close(int file_descriptor) {
    int errC;
    //check if file descriptor is valid since it can only be 0,1 or 2
    if (file_descriptor < 0) {
	    cout << "Invalid file descriptor!" << endl;
    } else {
	    errC = close(file_descriptor);
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
//the function will obtaion the information about the named file and
//write it to the area pointed to by the buf argument
// the buf argument passed in is a pointer to a stat structure
int my_stat(const char *pathname, struct stat& buf) {
	int errC = 0;

	if (stat(pathname, &buf) == 0) {
		//meaning it was successfull print the information
		cout << "Device name:" << buf.st_dev << endl;
		cout << "Inode:" << buf.st_ino << endl;
		cout << "Mode:" << buf.st_mode << endl;
		cout << "Owner:" << buf.st_uid <<endl;
		cout << "Number of links:" << buf.st_nlink <<endl;
		cout << "Group:" << buf.st_gid << endl;
		cout << "File size:" << buf.st_blksize << endl;
		//for the time_t values, we will extract each one by one and convert them into readable format
		//which can then be printed out
		char accessT[15];
		char modT[15];
		char createT[15];

        //use local time() to get the time_t variables of each desired file time
		tm *access_time = localtime(&buf.st_atime);
		tm *mod_time = localtime(&buf.st_mtime);
		tm *create_time = localtime(&buf.st_ctime);

        //put the date in the correct format and then print
		strftime(accessT, 15, "%Y-%m-%d %H:%M:%S", access_time);
		strftime(modT, 15, "%Y-%m-%d %H:%M:%S", mod_time);
		strftime(createT, 15, "%Y-%m-%d %H:%M:%S", create_time);

        cout << "Access Time:" << accessT << endl;
        cout << "Modification Time:" << modT << endl;
        cout << "Creation Time:" << createT << endl;

	} else {
        errC = -1;
    }

    return errC;
}

//******************************************************************************

//the function shall obtain information about about a file
//with the specififed fd and write it to the are pointed
//to by buf
int fs::my_fstat(int fd, struct stat& buf) {
    int errC = 0;
    if (fstat(fd, &buf) == 0) {
        //meaning it was successfull therefore print information about the open file
        cout << "Device name:" << buf.st_dev << endl;
		cout << "Inode:" << buf.st_ino << endl;
		cout << "Mode:" << buf.st_mode << endl;
		cout << "Owner:" << buf.st_uid <<endl;
		cout << "Number of links:" << buf.st_nlink <<endl;
		cout << "Group:" << buf.st_gid << endl;
		cout << "File size:" << buf.st_blksize << endl;
		//for the time_t values, we will extract each one by one and convert them into readable format
		//which can then be printed out
		char accessT[15];
		char modT[15];
		char createT[15];

        //use local time() to get the time_t variables of each desired file time
		tm *access_time = localtime(&buf.st_atime);
		tm *mod_time = localtime(&buf.st_mtime);
		tm *create_time = localtime(&buf.st_ctime);

        //put the date in the correct format and then print
		strftime(accessT, 15, "%Y-%m-%d %H:%M:%S", access_time);
		strftime(modT, 15, "%Y-%m-%d %H:%M:%S", mod_time);
		strftime(createT, 15, "%Y-%m-%d %H:%M:%S", create_time);

        cout << "Access Time:" << accessT << endl;
        cout << "Modification Time:" << modT << endl;
        cout << "Creation Time:" << createT << endl;

        //close file descriptor
        //my_close(fd);

    } else {
        errC = -1;
    }

    return errC;
}

//******************************************************************************
