#include "commands.h"
#include "console/console.h"
#include "helper_math.h"
#include <stdio.h>
#include <string.h>

void command_run(CommandResponse *response, char *command, int len) {
  if (len <= 0) {
    return;
  }

  switch (command[0]) {
  case 'p': {
    if (strncmp(command, "print", 5) == 0) {
      // lol
      int len = MIN(LINE_BUF_SZ, strlen(command + 5));
      console_println(command + 6, len);
    }
  } break;
  }
}
