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

#include "shell.h"

// clearing the shell using escape sequences
#define clear() printf("\033[H]033]J")

// shell greeting during start up
void init_shell() {
  clear();
  printf("\n\n\n\n*******************");
  printf("\n\n\tThe Shell*****");

  sleep(1);
  clear();
}

int main(int argc, char **argv) {
  return 0;
}