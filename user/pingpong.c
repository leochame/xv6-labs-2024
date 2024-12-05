//
// Created by liulch on 24-10-13.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define WRITE 1
#define READ 0

int
main(int argc, char *argv[])
{
  int p2c[2];
  int c2p[2];
  char buf[5];
  pipe(p2c);
  pipe(c2p);

  int pid = fork();
  if (pid < 0) {
    exit(1);
  }else if(pid == 0) {
    // This is child
    close(p2c[WRITE]);
    close(c2p[READ]);

    read(p2c[READ], buf, 4);
    printf("%d: received %s\n",getpid(), buf);
    write(c2p[WRITE], "pong", 4);
    exit(0);
    }else{
      close(p2c[READ]);
      close(c2p[WRITE]);
      write(p2c[WRITE], "ping", 4);
      read(c2p[READ], buf, 4);
      printf("%d: received %s\n",getpid(), buf);
      exit(0);
     }
}