#pragma once

#include "console/console.h"
#include "helper_math.h"
typedef enum CommandStatus {
  STAT_OK,
  STAT_ERR,
  STAT_COUNT,
} CommandStatus;

typedef struct CommandResponse {
  CommandStatus status_code;
} CommandResponse;

void command_run(CommandResponse *response, char *command, int len);
