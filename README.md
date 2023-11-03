# CSC351 ext2 based Operating System simulation.
The app is composed of the filesystem and the shell to accompany the filesystem.

## File System Overview
The File System (FS) shall be sized at 2GB of raw storage, and shall be implemented within a file on the computer.
The program shall have the ability to create the file system, upon request. Otherwise, the program shall communicate via TCP sockets.
When the FS is told to shutdown, the FS program shall terminate cleanly. Upon starting again, the FS shall be in the same state as it was when it shut down.

### FS Block Size
The FS shall use 4KB block size and for each time a file needs to be lengthened, it shall allocate 8 more blocks.

## Git basics
### Installing Git
[Git Documentation](https://git-scm.com/docs)

### Cloning this repository
- For this project, please create your own branch first with your name prepended.
- $ git clone {url}

### Git quck instructions
- Initialize your project
    - $ git init
- To create your own branch
    - $ git checkout -b {new branch name}
- Switch between branches
    - $ git checkout {branch name}
- Once you've made code changes:
    - $ git status (can skip this step)
    - $ git add .
    - $ git commit -m "commit comment"
    - $ git push {branch name}
- To push your changes to the main repo
    - $ git push origin main

### Important!!!
- Make sure to pull from the main repo first before pushing to main
    - $ git pull
