#include "commands.h"
#include "areas/area_server.h"
#include "console/console.h"
#include "helper_math.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

#define MAX_WORDS 16
#define MAX_WORD_SIZE 256

typedef struct CommandInput {
  int argc;
  char argv[MAX_WORDS][MAX_WORD_SIZE];
  char joined_argv[(MAX_WORDS - 1) * MAX_WORD_SIZE];
} CommandInput;

static CommandInput cmd_buf;

// fill in the provided buffer based on the string input. input should be null
// termed.
static void split_string(CommandInput *dest, char *input) {
  memset(dest, 0, sizeof(CommandInput));

  int which_word = 0; // which word buffer are we writing into?
  int word_ch_index =
      0; // which char of the current word buffer are we writing into?

  int i = 0;
  char ch = input[i];
  while (ch != '\0') {
#define NEXT_WORD()                                                            \
  {                                                                            \
    which_word++;                                                              \
    word_ch_index = 0;                                                         \
  }

    ch = input[i];
    switch (ch) {

    case ' ': {
      NEXT_WORD();
      if (which_word > MAX_WORDS) {
        fprintf(stderr, "Could not parse %s, too many words.\n", input);
      }
    } break;

    default: {
      dest->argv[which_word][word_ch_index] = ch;
      word_ch_index++;

      if (word_ch_index > MAX_WORD_SIZE) {
        fprintf(stderr,
                "Could not parse %s, too many characters in one word.\n",
                input);
      }
    } break;
    }

    i++;
#undef NEXT_WORD
  }

  dest->argc = which_word + 1;

  char *joined_ptr = dest->joined_argv;
  for (int i = 1; i < dest->argc - 1; i++) {
    joined_ptr += sprintf(joined_ptr, "%s ", dest->argv[i]);
  }
  sprintf(joined_ptr, "%s", dest->argv[dest->argc - 1]);
}

void command_run(CommandResponse *response, char *command, int len) {
  if (len <= 0) {
    return;
  }

  split_string(&cmd_buf, command);
  if (cmd_buf.argc == 0)
    return;

  char *base_cmd = cmd_buf.argv[0];

  if (strncmp(base_cmd, "print", 5) == 0) {
    console_printf("%s", cmd_buf.joined_argv);

  } else if (strncmp(command, "area", 4) == 0) {
    console_printf("trying to load area: %s...\n", cmd_buf.joined_argv);

    int area_num = 0;
    area_switch(cmd_buf.joined_argv);
  } else if (strncmp(command, "newline", 7) == 0) {
    console_printf("\n\n");
  } else if (strncmp(command, "clear", 5) == 0) {
    console_printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
  } else {
    console_printf("'%s' is not a valid command.\n", command);
  }
}
