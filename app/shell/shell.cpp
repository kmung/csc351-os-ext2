#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <syscall.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "shell.h"

using namespace std;

#define SERVER_PORT 8080
#define MAX_BUFFER_SIZE 1024

// clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

// shell greeting during start up
void init_shell() {
  clear();
  printf("\n\n\n\n*******************");
  printf("\n\n\t*****The Shell*****");
  printf("\n\n\n\n*******************");

  sleep(1);
  clear();
}

int main(int argc, char **argv) {

  // call the shell greeting
  init_shell();

  /*
  TODO: write a REPL : Read-Eval-Print-Loop
  */

 int sock = socket(AF_INET, SOCK_STREAM, 0);
 if (sock < 0) {
   cerr << "Error: Cannot create client socket." << endl;
   return EXIT_FAILURE;
 }

  // incoming server details
  struct sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(SERVER_PORT); // port to connect to
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  // connect to the server
  if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    cerr << "Error connecting to the server." << endl;
    return EXIT_FAILURE;
  }

  char buffer[MAX_BUFFER_SIZE];
  char cwd[MAX_BUFFER_SIZE];

  while (true) {
    // get the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      cout << cwd << "-> "; // prompt the user with current working directory and -> to indicate that the user can enter a command
    } else {
      cerr << "Error getting current working directory!" << endl;
      break;
    }

    cin.getline(buffer, MAX_BUFFER_SIZE);

    if (send(sock, buffer, strlen(buffer), 0) < 0) {
      cerr << "Error sending data to server!" << endl;
      break;
    }

    memset(buffer, 0, MAX_BUFFER_SIZE);
    if (recv(sock, buffer, MAX_BUFFER_SIZE, 0) < 0) {
      cerr << "Error receiving data from server!" << endl;
      break;
    }

    cout << buffer << endl;
  }

  close(sock);
 
  return 0;
}