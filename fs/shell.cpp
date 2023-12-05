#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <vector>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <arpa/inet.h> 
    #include <sys/socket.h>
    #include <sys/ioctl.h>
#endif
#include <iomanip>
#include <io.h>

using namespace std;

#define SERVER_PORT 8080
#define MAX_BUFFER_SIZE 4096

vector<string> splitString(const string& s, const string& delimiters) {
    vector<string> tokens;
    size_t start = 0, end = 0;

    while ((end = s.find_first_of(delimiters, start)) != string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
    }

    tokens.push_back(s.substr(start));
    return tokens;
}

string replaceWord(string& str, const string& target, const string& replacement) {
    size_t pos = 0;
    while ((pos = str.find(target, pos)) != string::npos) {
        str = str.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }
    return str;
}

int getTerminalWidth() {
  #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
  #else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
  #endif
}

// clearing the shell using escape sequences
void clearShell() {
  cout << "\033[H\033[J";
}

// center-align the text within the terminal window
void printCentered(const string& text) {
  int width = getTerminalWidth();
  int padding = (width - text.length()) / 2;

  cout << setw(padding + text.length()) << text << endl;
}

// shell greeting during start up
void init_shell() {
    clearShell();
    cout << "\n\n\n\n";

    printCentered("********************************************");
    printCentered("**** The Creative Awesome Shell - Crash ****");
    printCentered("**** Proceed with caution... ***************");
    printCentered("********************************************\n");
}


  /*
  TODO: write a REPL : Read-Eval-Print-Loop
  */
void repl(int sock) {
  char buffer[MAX_BUFFER_SIZE];
  // char cwd[MAX_BUFFER_SIZE]; // to store the current working directory

  string cwd = "";

  bool firstTimeCalled = true;
  while (true) {
    // cout << "breaking" <<endl;
    // break;

    // // get the current working directory
    // if (getcwd(cwd, sizeof(cwd)) != NULL) {
    //   cout << cwd << "-> "; // prompt the user with current working directory and -> to indicate that the user can enter a command
    // } else {
    //   cerr << "Error getting current working directory!" << endl;
    //   break;
    // }

    // receive the current working directory from the server
    // cout << "waiting to recieve cwd" << endl;
    // memset(buffer, 0, MAX_BUFFER_SIZE); // clear the buffer
    // int cwd_received = recv(sock, cwd, MAX_BUFFER_SIZE - 1, 0);
  //   cout << "cwd_received: " << cwd_received << endl;
  //  if (cwd_received < 0) {
  //     int errorNumber = errno;      
  //     cerr << "Error receiving current working directory from server! Error number: " << errorNumber << " Error description: " << strerror(errorNumber) << endl;
  //     break;
  //   } else if (cwd_received == 0) {
  //     cerr << "The server closed the connection." << endl;
  //     break;
  //   } else {
  //     cwd[cwd_received] = '\0';

  //     // output the current working directory
  //     cout << string(cwd) << "-> ";
  //   }

    // get user input
    if (!firstTimeCalled){
      cout << cwd << "->";
      cin.getline(buffer, MAX_BUFFER_SIZE);
    }

    // close the shell when user enters exit
    if (string(buffer) == "exit") {
      cout << "Exiting..." << endl;
      break;
    }

    // cout << "Sending data to server..." << endl;
    // send the user input to the server
    // output error if sending fails

    // cout << "sending user input" << endl;
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
      cerr << "Error sending data to server!" << endl;
      break;
    }

    if (string(buffer) == "shutdown") {
      break;
    }

    // cout << "recieving final output" << endl;
    memset(buffer, 0, MAX_BUFFER_SIZE); // 
    if (recv(sock, buffer, MAX_BUFFER_SIZE, 0) < 0) {
      cerr << "Error receiving data from server!" << endl;
      // break;
    }

    string splitchar = "[$&^]";
    vector<string> splits = splitString(string(buffer), splitchar);
    cwd = splits[0];
    if (!firstTimeCalled){
      string buffer_str = string(buffer);

      cout << replaceWord(buffer_str, string(cwd+"[$&^]"), string("")) << endl;
    }else{
      firstTimeCalled = false;
    }
      

  }
 }

int main(int argc, char **argv) {

  // call the shell greeting
  init_shell();

  // sock is client
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    cerr << "Error: Cannot create client socket." << endl;
    return EXIT_FAILURE;
  }

  int enable = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    cerr << "Error: setsockopt(SO_REUSEADDR) failed" << endl;
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

  
  // call the repl function here
  repl(sock);

  close(sock);
 
  return 0;
}