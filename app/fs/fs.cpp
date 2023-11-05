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


using namespace std;

#include "fs.h"

int my_read(ifstream& file, char* buffer, int nbytes) {
  //Pass in fd 'file' along with buffer and number of bytes to read
  file.read(buffer, nbytes);
  //OS manages the file pointer which tracks current position in the file, reads file and updates pointer
  return file.gcount();
  //returns number of bytes read
}

int my_write(ofstream& file, const char* buffer, int nbytes) {
  //Pass in fd, the data to write, and number of bytes to write
  file.write(buffer, nbytes);
  //file pointer is managed to be at the correct position, if not specified it will write to the end of the file
  //specify and offset and it writes to that position
  return file.good() ? nbytes : 0;
  //returns number of bytes written 
} 

//******************************************************************************

int my_creat(const string& name, int mode) {
  filesystem::path filePath(name); //library
  int rc = 0;
  //check to see if file already exists
  if(filesystem::exists(filePath)) {
    rc = 2; //file exists
  } 
  //if file doesnt exist, create it (inode implementation?)
  ofstream file(name, ios::out); 
  if (!file.is_open()) {
    rc = -1;
  }
  file.close();
  return rc;
}

//******************************************************************************

int my_Iseek(fstream& file, streamoff offset, ios_base::seekdir when) {
  file.seekg(offset, when); //use seekg for input
  int rc = 0;

  if(file.fail()) {
    cerr << "Seek failed" << endl;
    rc = -1;
  }

  return rc;
}

//******************************************************************************

int my_open(const char *pathname, mode_t mode) {
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
int my_stat(const char *pathname, struct stat buf) {
	int errC;

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
		char *accessT;
		char *modT;
		char *createT;

		struct tm *at;
		struct tm *mt;
		struct tm *ct;

		at = localtime(&buf.st_atimespec);
		strftime(accessT, 15, "%Y-%m-%d %H:%M:%S", localtime(&buf.st_atimespec));
		strftime(modT, 15, "%Y-%m-%d %H:%M:%S", localtime(&buf.st_mtimespec));
		strftime(createT, 15, "%Y-%m-%d %H:%M:%S", localtime(&buf.st_ctimespec));

	}
}



//******************************************************************************
// open function 