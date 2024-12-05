//
// Created by liulch on 24-10-13.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]){
  if ( argc == 1 ){
    printf("Usage: sleep is error, could't find any argv\n");
    exit(1);
    }
    int time = atoi(argv[1]);
    if (time < 0){
      printf("Invalid sleep value\n");
      exit(1);
    }

    sleep(time);

    exit(0);
}