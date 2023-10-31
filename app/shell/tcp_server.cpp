// here we take care of TCP connection
// this file will be called in other files where needed
// the shell is the client side that sends TCP requests
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h> //The arpa/inet.h header file contains definitions for internet operations, from IBM
#include <sys/socket.h> //The sys/socket.h header file contains sockets definitions. from IBM

using namespace std;

// define port number to listen to
#define PORT 8080 // port 8080 is common application web server port

int main() {

  // create a socket here
  int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0) {
    cerr << "Socket creation failed!" << endl;
    return EXIT_FAILURE;
  }


  
  return 0;
}