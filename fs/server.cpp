// here we take care of TCP connection
// the fs is the server side
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <syscall.h>
#include <arpa/inet.h> //The arpa/inet.h header file contains definitions for internet operations, from IBM
#include <sys/socket.h> //The sys/socket.h header file contains sockets definitions. from IBM
#include "libraries/json.hpp"
#include "libraries/json_fwd.hpp"
#include <fstream>
#include <sstream>

using namespace std;
using json = nlohmann::json;

// define port number to listen to
#define PORT 8080 // port 8080 is common application web server port
#define MAX_BUFFER_SIZE 4096 // max buffer size for incoming data, server can receive up to 1024 bytes of data at a time from the client

// function to load commands from commands.json
nlohmann::json loadCommands() {
  ifstream commandsFile("commands.json");
  nlohmann::json commandsJson;
  commandsFile >> commandsJson;
  return commandsJson;
}

// call the commands
// nlohmann::json commands = loadCommands();

class Path {
    vector<string> disassembled_path;
    bool is_absolute = false;

public:
    Path() = default;
    explicit Path(string& path) {
        make_sure_path_is_valid(path);
        if (path[0] == '/') {
            path = path.substr(1);
            is_absolute = true;
        }
        disassembled_path = disassemble_path(path);
    }

    string format() const {
        string result;
        if (is_absolute) {
            result += "/";
        }
        for (const string& i : disassembled_path) {
            result += i + "/";
        }
        result.pop_back();
        return result;
    }

    static void make_sure_path_is_valid(string& path) {
        if (path.empty()) {
            throw invalid_argument("Path cannot be empty");
        }
        if (path[path.size() - 1] == '/') {
            throw invalid_argument("Path cannot end with '/'");
        }
        if (path.find("//") != string::npos) {
            throw invalid_argument("Path cannot contain '//'");
        }
    }

    static vector<string> disassemble_path(string path) {
        vector<string> disassembled_path;
        string current_dir;
        for (char i : path) {
            if (i == '/') {
                disassembled_path.push_back(current_dir);
                current_dir = "";
            } else {
                current_dir += i;
            }
        }
        disassembled_path.push_back(current_dir);
        return disassembled_path;
    }

    friend ostream& operator<<(ostream& os, const Path& path) {
        for (const string& i : path.disassembled_path) {
            os << i << endl;
        }
        return os;
    }

    static bool is_folder(const Path& path) {
        return path.disassembled_path.back().find('.') == string::npos;
    }

    static Path make_absolute(const Path& cwd, const Path& path) {
        if (path.is_absolute) {
            return path;
        }

        if (cwd.disassembled_path.empty()) {
            return path;
        }

        // Make sure cwd is a directory
        if (!is_folder(cwd)) {
            throw invalid_argument("cwd must be a directory");
        }

        vector<string> new_path = cwd.disassembled_path;
        for (const string& i : path.disassembled_path) {
            if (i == "..") {
                new_path.pop_back();
            } else {
                new_path.push_back(i);
            }
        }
        Path result = Path();
        result.disassembled_path = new_path;
        return result;
    }
};


string del_spaces(string str){
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    return str;
}

vector<string> disassemble_command(const string& command){
    vector<string> disassembled_command;

    string current_string;
    bool inside_quotes = false;

    for (char i : command) {
        if (i == ' ' && !inside_quotes) {
            // Do not add spaces to the disassembled command
            if (!del_spaces(current_string).empty()) {
                disassembled_command.push_back(current_string);
            }
            current_string = "";
        } else if (i == '"') {
            inside_quotes = !inside_quotes;
        } else {
            current_string += i;
        }
    }
    disassembled_command.push_back(current_string);
    return disassembled_command;
}

// TODO: MAKE SURE NOT TO USE YOUR OWN SYSTEM PATHS
ifstream f(R"(/home/ckmung/Documents/csc351/csc351-os-ext2/fs/commands.json)");
json COMMAND_TEMPLATE = json::parse(f);

vector<string> parse_command(const string& command, string& cwd){
    Path cwd_path = Path(cwd);
    vector<string> disassembled_command = disassemble_command(command);

    bool valid_command = false;
    vector<string> command_sent_to_filesystem;

    for (auto command_data: COMMAND_TEMPLATE["commands"]){
        bool required_argument_mode = true;
        if (command_data["name"] == disassembled_command[0]){
            valid_command = true;
            command_sent_to_filesystem.push_back(command_data["name"]);
            disassembled_command.erase(disassembled_command.begin());
            int no_of_args = command_data["syntax"]["args"].size();
            int no_of_required_args = 0;
            for (const auto& arg: command_data["syntax"]["args"]){
                if (arg["required"]){
                    no_of_required_args++;
                }
            }

            if (no_of_args < disassembled_command.size()){
                throw invalid_argument("Too many arguments");
            }

            if (no_of_required_args > disassembled_command.size()){
                throw invalid_argument("Too few arguments");
            }

            for (const auto& arg: command_data["syntax"]["args"]){

                cout << arg << endl;
                if (required_argument_mode && !arg["required"]){
                    required_argument_mode = false;
                }

                if (!required_argument_mode && arg["required"]){
                    throw invalid_argument("Required argument cannot be followed by optional argument\n "
                                           "This is caused by an internal error in the command template");
                }

                if (!disassembled_command.empty()){
                    string argument = disassembled_command[0];
                    if (arg["type"] == "path"){
                        Path path = Path(argument);
                        argument = Path::make_absolute(cwd_path, path).format();
                    }else{
                        throw invalid_argument("Invalid argument type\n "
                                               "This is caused by an internal error in the command template");
                    }
                    command_sent_to_filesystem.push_back(argument);
                    disassembled_command.erase(disassembled_command.begin());
                }
            }
        }
    }

    if (!valid_command){
        throw invalid_argument("Invalid command");
    }
    return command_sent_to_filesystem;
}

// -----------------------------
int main() {

  // create the server socket here
  int server = socket(AF_INET, SOCK_STREAM, 0); // AF_INET is IPv4, SOCK_STREAM is TCP, 0 is IP
  if (server < 0) {
    cerr << "Server socket creation failed!" << endl;
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
    perror("Cannot bind to the socket");
    return EXIT_FAILURE;
  }

  // listen for incoming connections
  if (listen(server, 5) < 0) {
    cerr << "Error!" << endl;
    perror("Error listening for incoming connections!");
    return EXIT_FAILURE;
  }

  // shutdown flag
  bool shutdown = false;

  while (true) {
    // accept incoming connections
    struct sockaddr_in clientAddress;
    socklen_t clientAddrLen = sizeof(clientAddress);
    int client = accept(server, (struct sockaddr*)&clientAddress, &clientAddrLen);
    if (client < 0) {
        cerr << "Error accepting connection!" << endl;
        continue;
    }

    // receive data from the client
    char buffer[MAX_BUFFER_SIZE];
    
    while (true) {
        memset(buffer, 0, MAX_BUFFER_SIZE); // clear the buffer
        if (recv(client, buffer, MAX_BUFFER_SIZE, 0) < 0) {
        cerr << "Error receiving data from client!" << endl;
        break;
        }
        // if the client sends a non-empty string, process the command
        if (string(buffer) != "") {
            string command = string(buffer);
            cout << "Received command: " << command << endl;
            string path = string("/system/user");

            string finalOutput = "";
            try {
                vector<string> command_parsed = parse_command(command, path);

                if (command_parsed[0] == "ls") {
                // code goes here
                } else if (command_parsed[0] == "shutdown"){
                cout << "Shutting down the server..." << endl;
                shutdown = true; // set the shutdown flag to true
                break;
                }

                for (const auto& i : command_parsed) {
                    finalOutput += i + " -- ";
                }
            } catch (const invalid_argument& e) {
                finalOutput = e.what();
            }

            cout << finalOutput << "[server-output]" << endl;
            if (send(client, finalOutput.c_str(), finalOutput.size(), 0) < 0) {
            cerr << "Couldn't send an output to the client" << endl;
            break;
            }
        }  
    }
    // close the client socket
    close(client);

    // break the loop if the shutdown flag is set to true
    if (shutdown) {
      break;
    }
  }

  // close the server socket
  close(server);
  
  return 0;
}

