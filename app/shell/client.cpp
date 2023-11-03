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
#define MAX_BUFFER_SIZE 1024 // max buffer size for incoming data, server can receive up to 1024 bytes of data at a time from the client

 int main() {

  /*
  * TODO: need to get the IP address of the server.
  * For now, assume the client and server are on the same machine
  * Use the loopback address, 127.0.0.1
  */

  // create the client socket
  int client = socket(AF_INET, SOCK_STREAM, 0);
  if (client < 0) {
    cerr << "Error: Cannot create client socket." << endl;
    return EXIT_FAILURE;
  }

  // incoming server details
  struct sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT); // port to connect to
  // modify later?
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // hardcoded IP address of the loopback server

  // connect to the server
  if (connect(client, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    cerr << "Error connecting to the server." << endl;
    return EXIT_FAILURE;
  }

  // send a simple test msg
  const char* msg = "client";
  send(client, msg, strlen(msg), 0);

  // receive from the server
  char buffer[MAX_BUFFER_SIZE] = {0};
  recv(client, buffer, sizeof(buffer), 0);
  cout << "from server: " << buffer << endl;

  // close the client socket connection
  close(client);

  return 0;
 }