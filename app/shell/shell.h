#ifndef SHELL_H
#define SHELL_H

class shell {
  private:
    /* data */

  public:
    // constructor
    shell(int argc, char *argv[]);
    // destructor
    ~shell();

    void init_shell();
};


#endif