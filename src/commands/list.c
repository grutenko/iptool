#include "cmd.h"

#include <stdlib.h>
#include <stdio.h>

int cmd_list(int argc, char **argv) {
  struct cmd_struct *commands = list_cmd();
  int i;
  for (i = 0; commands[i].name; i++) {
    printf("%s\t\t%s\n", commands[i].name, commands[i].descr);
  }
  putc('\n', stdout);
  return EXIT_SUCCESS;
}
