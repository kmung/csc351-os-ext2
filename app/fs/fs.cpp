//fs.cpp

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
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

int my_Iseek(fstream& file, streamoff offset, ios_base::seekdir when) {
  file.seekg(offset, when); //use seekg for input
  int rc = 0;

  if(file.fail()) {
    cerr << "Seek failed" << endl;
    rc = -1;
  }

  return rc;
}