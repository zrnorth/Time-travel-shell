// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "alloc.h"

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */



int
command_status (command_t c)
{
    return c->status;
}

//returns a pointer to an array of strings, terminated with a null pointer
char** split_string (char* s)
{
    int num_tokens = 0;
    char* p;
    char** head = checked_malloc(sizeof(char*));
    head[0] = NULL; //initialize
    
    p = strtok(s, " ");
    while (p != NULL)
    {
        num_tokens++;
        head = checked_realloc(head, sizeof(char*) * (num_tokens + 1)); //1 extra for null
        head[num_tokens-1] = p;
        head[num_tokens] = NULL;
        p = strtok(NULL, " ");
    }
    //debug
    int i;
    for (i = 0; i < num_tokens; i++)
        printf("%s\n", head[i]); 
    
    return head;
}

void
execute_command (command_t c, bool time_travel)
{
    //assuming that time_travel is false
    //TODO: this changes in part 1c
    time_travel = false;

    enum command_type type = c->type;
    switch(type)
    {
    case SIMPLE_COMMAND:
    {
        //execute the command; just use execvp and return what it returns.
        char* the_command = *c->u.word;
        
        char** cmd_tokenized = split_string(the_command);

        char* filenm = cmd_tokenized[0];
        char** args = cmd_tokenized; 
        if (execvp(filenm, args) == -1) //some sort of error
        {
            fprintf(stderr, "Error in input file.\n");
            c->status = -1;
        }
    }
    case AND_COMMAND:
    {
        command_t leftside = c->u.command[0];
        command_t rightside= c->u.command[1];
        //fork, and run the left side in a shell. if it returns -1, we know its borked.
        pid_t pid = fork();
        if (pid == 0) //child
        {
            execute_command(leftside, time_travel);
        }
        else //parent
        {
            int status = 0;
            if (wait(&status) == -1) //child exited incorrectly, so fail.
                break;
            else //child exited
            {
                if (status != 0) //exited with failstate, so fail (AND)
                    break;
                else  //exited normally, so we run the rightside.
                    execute_command(rightside, time_travel);
            }
        }
        //if we get here, something failed so return bad
        c->status = -1;
        break;
    }
    case OR_COMMAND:
    {
        //same as AND, but if the left side succeeds just return.
        command_t leftside = c->u.command[0];
        command_t rightside= c->u.command[1];
        //fork, and run the left side in a shell. if it returns -1, we know its borked.
        pid_t pid = fork();
        if (pid == 0) //child
        {
            execute_command(leftside, time_travel);
        }
        else //parent
        {
            int status = 0;
            if (wait(&status) == -1 || status != 0) //child exited incorrectly, so try the rightside.
            {
                execute_command(rightside, time_travel);
            }
        }
        //if we get here, means they both failed. so, return fail.
        c->status = -1;
        
        break;
    }
    case PIPE_COMMAND:
        //execute left and feed into right
        //treat each command as a subshell and run them seperately
        //if no time travel, the right side waits for left to complete and grabs its output
        break;

    //the parser handles subshells and sequence commands already
    default:
        break;
    } //end switch
}
