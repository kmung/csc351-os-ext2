// Josiah Branch

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fs.h"
using namespace std;



int main() {
    fs myFs;
    
    // Test my_read and my_write functions with placeholder data
    char dataToWrite[] = "This is a test.";
    int writeFd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int bytesWritten = myFs.my_write(writeFd, dataToWrite, strlen(dataToWrite));

    // Check if writing was successful
    if (bytesWritten > 0) {
        cout << "Successfully wrote data." << endl;
    } else {
        cerr << "Error writing to the file." << endl;
    }

    close(writeFd);

    // Test my_creat function
    int createResult = myFs.my_creat("newfile.txt", 0644);
    if (createResult == 0) {
        cout << "File created successfully." << endl;
    } else if (createResult == 2) {
        cout << "File already exists." << endl;
    } else if (createResult == -1) {
        cout << "Error creating file." << endl;
    }

    // Test my_open and my_close functions
    int openFd = myFs.my_open("example.txt", O_RDONLY);
    if (openFd != -1) {
        myFs.my_close(openFd);
    } else {
        cerr << "Error opening the file." << endl;
    }

    // Test my_stat function
    struct stat fileStat;
    int statResult = myFs.my_stat("example.txt", fileStat);
    if (statResult == 0) {
        cout << "File size: " << fileStat.st_size << " bytes" << endl;
    } else if (statResult == -1) {
        cerr << "Error getting file status." << endl;
    }

    return 0;
}