// UCLA CS 111 Lab 1 main program
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "command.h"

static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-pt] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}

int
main (int argc, char **argv)
{
  int command_number = 1;
  bool print_tree = false;
  bool time_travel = false;
  program_name = argv[0];

  for (;;)
    switch (getopt (argc, argv, "pt"))
      {
      case 'p': print_tree = true; break;
      case 't': time_travel = true; break;
      default: usage (); break;
      case -1: goto options_exhausted;
      }
 options_exhausted:;

  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

  command_t last_command = NULL;
  command_t command;
  if (time_travel && !print_tree)
  {
    pid_t p = fork(); //parent will wait for entire loop to finish
    if (!p) //child
    {
        while ((command = read_command_stream(command_stream))) //new proc for each command
        {
            pid_t n = fork();
            if (!n)
            {
                last_command = command;
                execute_command(command, time_travel);
                exit(0);
            }
            else //parent
            {
                continue; //make the next proc
            }
        }
    }
    else //parent waits for all children to finish
    {
        int status = 0;
        wait(&status);
    }
  }
  else
  {
    while ((command = read_command_stream (command_stream)))
    {
        if (print_tree)
	    {
	        printf ("# %d\n", command_number++);
	        print_command (command);
	    }
        else 
	    {
	        last_command = command;
	        execute_command (command, time_travel);
	    }
    }
  }
  return print_tree || !last_command ? 0 : command_status (last_command);
}
