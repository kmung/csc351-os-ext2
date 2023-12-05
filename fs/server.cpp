// here we take care of TCP connection
// the fs is the server side
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <unistd.h>
    #include <arpa/inet.h> //The arpa/inet.h header file contains definitions for internet operations, from IBM
    #include <sys/socket.h> //The sys/socket.h header file contains sockets definitions. from IBM
#endif
#include "libraries/json.hpp"
#include "libraries/json_fwd.hpp"
#include <fstream>
#include <sstream>
#include "filesystem.h"

using namespace std;
using json = nlohmann::json;

// define port number to listen to
#define PORT 8080 // port 8080 is common application web server port
#define MAX_BUFFER_SIZE 4096 // max buffer size for incoming data

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
    str.erase(remove(str.begin(), str.end(), ' '), str.end());
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

string json_path ="C:/Users/ssyak/OneDrive/Desktop/class/2023fall/CSC351/csc351-os-ext2/fs/commands.json";

// TODO: MAKE SURE NOT TO USE YOUR OWN SYSTEM PATHS
ifstream f(json_path);
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
            // disassembled_command.erase(disassembled_command.begin());

            int no_of_args = command_data["syntax"]["args"].size();
            int no_of_required_args = 0;
            for (const auto& arg: command_data["syntax"]["args"]){
                if (arg["required"]){
                    no_of_required_args++;
                }
            }

            // if (no_of_args < disassembled_command.size() - 1){
            //     throw invalid_argument("Too many arguments");
            // }

            if (no_of_required_args > disassembled_command.size() - 1){
                throw invalid_argument("Too few arguments");
            }

            disassembled_command.erase(disassembled_command.begin());
            for (const auto& arg: command_data["syntax"]["args"]){
                if (required_argument_mode && !arg["required"]){
                    required_argument_mode = false;
                }

                if (!required_argument_mode && arg["required"]){
                    throw invalid_argument("Required argument cannot be followed by optional argument\n "
                                           "This is caused by an internal error in the command template");
                }

                if (!disassembled_command.empty()){
                    string argument = disassembled_command[0];
                    // if (arg["type"] == "path"){
                    //     Path path = Path(argument);
                    //     argument = Path::make_absolute(cwd_path, path).format();
                    // }else{
                    //     cout << "Invalid argument type" << endl;
                    //     throw invalid_argument("Invalid argument type\n "
                    //                            "This is caused by an internal error in the command template");
                    // }
                    command_sent_to_filesystem.push_back(argument);
                    disassembled_command.erase(disassembled_command.begin());
                }
            }
            cout << "command_sent_to_filesystem.size(): " << command_sent_to_filesystem.size() << endl;
            for (int i = 0; i<command_sent_to_filesystem.size(); i++){
                cout << "command_sent_to_filesystem[" << i << "]: " << command_sent_to_filesystem[i] << endl;
            }
            break;
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

    // create a filesystem object
   fs filesystem("virtual_disk.vhd");

   // shutdown flag
   bool shutdown = false;  

   // state variable
   bool is_sent = false;  

   // buffer to store data
    char buffer[MAX_BUFFER_SIZE];
   
  while (!shutdown) {
    // accept incoming connections
    struct sockaddr_in clientAddress;
    socklen_t clientAddrLen = sizeof(clientAddress);
    int client = accept(server, (struct sockaddr*)&clientAddress, &clientAddrLen);
    if (client < 0) {
        cerr << "Error accepting connection!" << endl;
        continue;
    }

   // cout << "found client and connected" << endl;

   
    //cout << cwd << endl;
    
    while (true) {
         // send current working directory to the client
         
        string cwd = filesystem.my_getcwd();
        // memset(buffer, 0, MAX_BUFFER_SIZE); // clear the buffer
        // cwd.copy(buffer, MAX_BUFFER_SIZE - 1);
        // buffer[MAX_BUFFER_SIZE - 1] = '\0'; // null terminate the buffer
        // cout << "sending cwd" << endl;
        // if (send(client, buffer, strlen(buffer), 0) < 0) {
        //     cerr << "Couldn't send current working directory to the client" << endl;
        //     break;
        // }
        
        // cout << "Starting..." << endl;
        
        memset(buffer, 0, MAX_BUFFER_SIZE); // clear the buffer
        if (recv(client, buffer, MAX_BUFFER_SIZE, 0) < 0) {
            cerr << "Error receiving data from client!" << endl;
            break;
        }
        cout << "buffer: " << buffer << endl;
        // cout << "AAAA-Starting..." << endl;
        string finalOutput = "";
        // if the client sends a non-empty string, process the command
        if (string(buffer) != "") {
            string command = string(buffer);
            string path = string("/system/user");

            try {
                vector<string> command_parsed = parse_command(command, path);
                // for (const auto& i : command_parsed) {
                //     finalOutput = i + " -- ";
                // }
                cout << "command_parsed.size(): " << command_parsed.size() << endl;
                for (int i = 0; i<command_parsed.size(); i++){
                    cout << "command_parsed[" << i << "]: " << command_parsed[i] << endl;
                }

                if (command_parsed[0] == "ls") {
                    
                    if (command_parsed.size() == 1) {
                        finalOutput += filesystem.my_ls();
                    } else {
                        finalOutput += filesystem.my_ls(command_parsed[1]);
                    }
                } else if (command_parsed[0] == "cd") {
                    if(command_parsed.size() == 1){
                        filesystem.my_cd();
                    } else {
                        filesystem.my_cd(command_parsed[1]);
                    }
                }else if (command_parsed[0] == "mkdir"){
                    filesystem.my_mkdir(command_parsed);
                }else if (command_parsed[0] == "Lcp"){
                    filesystem.my_Lcp(command_parsed[1], command_parsed[2]);
                }else if (command_parsed[0] == "lcp"){
                    filesystem.my_lcp(command_parsed[1], command_parsed[2]);
                }else if (command_parsed[0] == "rm"){
                    filesystem.my_rm(command_parsed);
                }else if (command_parsed[0] == "rmdir"){
                    filesystem.my_rmdir(command_parsed);
                }else if (command_parsed[0] == "chown"){
                    filesystem.my_chown(command_parsed[1], stoi(command_parsed[2]), stoi(command_parsed[3]));
                }else if (command_parsed[0] == "cp"){
                    filesystem.my_cp(command_parsed[1], command_parsed[2]);
                }else if (command_parsed[0] == "mv"){
                    filesystem.my_mv(command_parsed[1], command_parsed[2]);
                }else if (command_parsed[0] == "cat"){
                    finalOutput += filesystem.my_cat(command_parsed);
                }else if (command_parsed[0] == "ln"){
                    filesystem.my_ln(command_parsed[1], command_parsed[2]);
                } else if (command_parsed[0] == "shutdown"){
                    cout << "Shutting down the server..." << endl;
                    shutdown = true; // set the shutdown flag to true
                    // break;
                }else{
                    throw invalid_argument("[Internal Error] Invalid Command");
                }

                // for (const auto& i : command_parsed) {
                //     finalOutput += i + " -- ";
                // }
                cwd = filesystem.my_getcwd();
            } catch (std::exception& e) {
                finalOutput = e.what();
            }
        } 

        finalOutput = cwd + "[$&^]" + finalOutput;
        // cout << finalOutput << "[server-output]" << endl;
        
        if (send(client, finalOutput.c_str(), finalOutput.size(), 0) < 0) {
            cerr << "Couldn't send an output to the client" << endl;
            break;
        } 
    }
    // close the client socket
    close(client);
  }

  // close the server socket
  close(server);
  
  return 0;
}

