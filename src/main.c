#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "commands.h"
#include "built_in.h"
#include "utils.h"
#include "signal_handlers.h"

int main()
{ 
  catch_sigint(0);
  catch_sigtstp(0);
  /*struct sigaction act1, act2;
  act1.sa_handler=catch_sigint;
  sigemptyset(&act1.sa_mask);
  act1.sa_flags=0;
  act2.sa_handler=catch_sigtstp;
  sigemptyset(&act2.sa_mask);
  act2.sa_flags=0;
  sigaction(SIGINT, &act1, 0);
  sigaction(SIGTSTP, &act2, 0);
  */	
  char buf[8096];

  while (1) {
    fgets(buf, 8096, stdin);

    struct single_command commands[512];
    int n_commands = 0;
    mysh_parse_command(buf, &n_commands, &commands);

    int ret = evaluate_command(n_commands, &commands);

    free_commands(n_commands, &commands);
    n_commands = 0;
    fflush(stdin);
    fflush(stdout);
    if (ret == 1) {
      break;
    }
  }

  return 0;
}
