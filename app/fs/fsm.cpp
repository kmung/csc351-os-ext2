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
    
    int readFd = open("input.txt", O_RDONLY);
    int writeFd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    char buffer[100];
    int bytesRead = myFs.my_read(readFd, buffer, sizeof(buffer));
    int bytesWritten = myFs.my_write(writeFd, buffer, bytesRead);

    close(readFd);
    close(writeFd);

    if (bytesRead == -1 || bytesWritten == -1) {
        std::cerr << "Error reading/writing from/to file." << std::endl;
        return -1;
    }

    // Test my_creat function
    int createResult = myFs.my_creat("newfile.txt", 0644);
    if (createResult == 0) {
        std::cout << "File created successfully." << std::endl;
    } else if (createResult == 2) {
        std::cout << "File already exists." << std::endl;
    } else if (createResult == -1) {
        std::cout << "Error creating file." << std::endl;
    }

    // Test my_open and my_close functions
    int openFd = myFs.my_open("example.txt", O_RDONLY);
    if (openFd != -1) {
        myFs.my_close(openFd);
    }

    // Test my_stat function
    struct stat fileStat;
    int statResult = myFs.my_stat("example.txt", fileStat);
    if (statResult == 0) {
        std::cout << "File size: " << fileStat.st_size << " bytes" << std::endl;
    } else if (statResult == -1) {
        std::cout << "Error getting file status." << std::endl;
    }

    return 0;
}
