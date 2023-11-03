// here we take care of TCP connection
// the fs is the server side
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

  // create the server socket here
  int server = socket(AF_INET, SOCK_STREAM, 0); // AF_INET is IPv4, SOCK_STREAM is TCP, 0 is IP
  if (server < 0) {
    cerr << "Socket creation failed!" << endl;
    return EXIT_FAILURE;
  }

  // server details
  struct sockaddr_in serverAddress; // server address details
  serverAddress.sin_family = AF_INET; // IPv4
  serverAddress.sin_port = htons(PORT); // port to connect to
  serverAddress.sin_addr.s_addr = INADDR_ANY; // listen on all available network interfaces

  // bind the socket to the address
  if (bind(server, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    cerr << "Error binding socket!" << endl;
    return EXIT_FAILURE;
  }

  // listen for incoming connections
  if (listen(server, 5) < 0) {
    cerr << "Error listening for connections" << endl;
    return EXIT_FAILURE;
  }

  // accept incoming connections
  struct sockaddr_in clientAddress;
  socklen_t clientAddrLen = sizeof(clientAddress);
  int client = accept(server, (struct sockaddr*)&clientAddress, &clientAddrLen);
  if (client < 0) {
    cerr << "Error accepting connection!" << endl;
    return EXIT_FAILURE;
  }

  // receive data from the client
  char buffer[MAX_BUFFER_SIZE] = {0};
  recv(client, buffer, sizeof(buffer), 0);
  cout << "From client: " << buffer << endl;

  // send a response to the client
  const char* response = "Hello!";
  send(client, response, strlen(response), 0);

  // close the client socket
  close(client);

  // close the server socket
  close(server);
  
  return 0;
}