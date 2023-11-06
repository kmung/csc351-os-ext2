#include <iostream>
#include "json.hpp"
#include "json_fwd.hpp"
#include <fstream>
using json = nlohmann::json;

using namespace std;


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
// TODO: Tell the group we cannot have chaining requirements of false, false then later true in arguments
std::ifstream f(R"(C:\Users\cjlyn\CLionProjects\FileSystemShell\commands.json)");
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


int main(){
    string command = "lcp ../home/person/test home/person/test2";
    string cwd = "/system/user";
    vector<string> command_sent_to_filesystem = parse_command(command,cwd);
    cout << "command_sent_to_filesystem: " << endl;
    for (const string& i : command_sent_to_filesystem) {
        cout << i << "  |" << endl;
    }
}

//int main() {
////    std::ifstream f("example.json");
////    json data = json::parse(f);
//
//    string cwd = "/home/person/test";
//    Path cwd_path = Path(cwd);
//
//    cout << "cwd: " << endl;
//    cout << cwd_path << endl;
//
//    string path = "../user/person/thing.json";
//
//    Path final_path = Path(path);
//
//    cout << "final_path: " << endl;
//    cout << final_path << endl;
//
//
//    cout << "complete_path: " << endl;
//    cout << Path::make_absolute(cwd_path, final_path) << endl;
//
//}
