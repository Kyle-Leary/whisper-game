#include "commands.h"
#include "areas/area_server.h"
#include "audio/audio.h"
#include "console/console.h"
#include "gui/gui.h"
#include "helper_math.h"
#include "object.h"
#include "objects/sphere.h"
#include "physics/physics.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

#include "whisper/colmap.h"
#include "window.h"

#define MAX_WORDS 16
#define MAX_WORD_SIZE 256

typedef struct CommandInput {
  int argc;
  char argv[MAX_WORDS][MAX_WORD_SIZE];
  char joined_argv[(MAX_WORDS - 1) * MAX_WORD_SIZE];
} CommandInput;

static CommandInput cmd_buf;

typedef void (*command_fn)(CommandInput *);

// map over these in a hashtable to do really fast keyword matching.
typedef struct Command {
  command_fn fn;
} Command;

static WColMap command_map;

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

  Command *c = w_cm_get(&command_map, base_cmd);
  if (c) {
    if (c->fn) {
      c->fn(&cmd_buf);
    }
  } else {
    console_printf("'%s' is not a valid command.\n", command);
  }
}

void reload(CommandInput *cmd) { area_reload_curr(); }

void sphere(CommandInput *cmd) {
  // add it directly into the current area's object scope.
  object_add((Object *)sphere_build((vec3){0, 9, 0}, 1.0f, 7), OT_AREA);
}

void ptimestep(CommandInput *cmd) {
  float timestep = atof(cmd->argv[1]);
  physics_state.accumulator_trigger = timestep;
  console_printf("physics timestep set to %f\n", timestep);
}
void print(CommandInput *cmd) { console_printf("%s", cmd->joined_argv); }
void area(CommandInput *cmd) {
  console_printf("trying to load area: %s...\n", cmd->joined_argv);
  int area_num = 0;
  area_switch(cmd->joined_argv);
  toggle_console();
}
void quit(CommandInput *cmd) { window_force_close(); }
void clear(CommandInput *cmd) {
  console_printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

void gui(CommandInput *cmd) {
  if (strncmp(cmd->argv[1], "toggle", 6) == 0) {
    gui_toggle_visibility();
  } else {
    console_printf("Invalid gui subcommand.\n");
  }
}

void audio(CommandInput *cmd) {
  if (strncmp(cmd->argv[1], "killall", 7) == 0) {
    a_kill_all();
  } else if (strncmp(cmd->argv[1], "play", 4) == 0) {
    a_play_pcm(cmd->argv[2]);
  } else {
    console_printf("Invalid audio subcommand.\n");
  }
}

#define INSERT(name, func)                                                     \
  {                                                                            \
    void *com_ptr = w_cm_insert(&command_map, name, &(Command){.fn = func});   \
    if (com_ptr == NULL) {                                                     \
      fprintf(stderr,                                                          \
              "Warning: collision in the command name hashmap [name: %s].\n",  \
              name);                                                           \
    }                                                                          \
  }

void init_commands() {
  w_create_cm(&command_map, sizeof(Command), 509);

  INSERT("reload", reload);
  INSERT("sphere", sphere);
  INSERT("ptimestep", ptimestep);
  INSERT("print", print);
  INSERT("clear", clear);
  INSERT("area", area);
  INSERT("q", quit);
  INSERT("quit", quit);
  INSERT("gui", gui);
  INSERT("audio", audio);
}

void clean_commands() { w_free_cm(&command_map); }
