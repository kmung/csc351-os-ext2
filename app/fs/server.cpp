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
    cerr << "Error!" << endl;
    perror("Cannot bind to the client socket");
    return EXIT_FAILURE;
  }

  // listen for incoming connections
  if (listen(server, 5) < 0) {
    cerr << "Error!" << endl;
    perror("Error listening for incoming connections!");
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
  char buffer[MAX_BUFFER_SIZE];
  
  while (true) {
    memset(buffer, 0, MAX_BUFFER_SIZE); // clear the buffer
    if (recv(client, buffer, MAX_BUFFER_SIZE, 0) < 0) {
      cerr << "Error receiving data from client!" << endl;
      break;
    }

    if (string(buffer) == "shutdown") {
      cout << "Client has quit the session" << endl;
      break;
    }

    // process the commands here and generate a response
    if (send(client, buffer, strlen(buffer), 0) < 0) {
      cerr << "Error sending data to client!" << endl;
      break;
    }
  }

  // close the client socket
  close(client);
  // close the server socket
  close(server);
  
  return 0;
}