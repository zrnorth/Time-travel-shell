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
    
    return head;
}

void
execute_command (command_t c, bool time_travel)
{
    //assuming that time_travel is false
    //TODO: this changes in part 1c
    time_travel = false;

    enum command_type type = c->type;

    int old_stdin  = dup(fileno(stdin));
    int old_stdout = dup(fileno(stdout));
    int old_stderr = dup(fileno(stderr));

    int infd = -1;
    int outfd = -1;
    if (c->input)
    {
        infd = fileno(fopen(c->input, "r"));
        if (infd)
        {
            dup2(infd, fileno(stdin));
        }
        else
        {
            fprintf(stderr, "%s: No such file or directory\n", c->input);
            return;
        }
    }
    if (c->output)
    {
        outfd = fileno(fopen(c->output, "w"));
        if (outfd)
        {
            dup2(outfd, fileno(stdout));
            dup2(outfd, fileno(stderr));
        }
        else
        {
            fprintf(stderr, "%s: Could not open this file\n", c->output);
            return;
        }
    }
    switch(type)
    {
    case SIMPLE_COMMAND:
    {
        //execute the command; just use execvp and return what it returns.
        char* the_command = *c->u.word;
        
        char** cmd_tokenized = split_string(the_command);

        char* filenm = cmd_tokenized[0];
        char** args = cmd_tokenized; 

        pid_t pid = fork(); //fork and execute this cmd
        if (pid == 0)  //child
        {
            if (execvp(filenm, args) == -1) //some sort of error
            {
                fprintf(stderr, "Error in input file.\n");
                c->status = -1;
            }
            break;
        }
        else //parent
        {
            int status = 0;
            if (wait(&status) == -1) //child exited incorrectly
            {
                c->status = -1;
            }
            break;
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
    case PIPE_COMMAND: //this can't be changed in time_travel, bc dependencies
    {
        int pipefd[2]; //pipe file descriptors
        pid_t left_pid, right_pid; //leftside and rightside of the pipe_command

        command_t leftside = c->u.command[0];
        command_t rightside = c->u.command[1];

        pipe(pipefd); //create a pipe

        //leftside child, generating output to the pipe
        if ((left_pid = fork()) == 0)
        {   //we want to attach the leftside's stdout to the pipe
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]); //not using rightside

            //execute left side
            execute_command(leftside, time_travel);
            //if we ever get here we want to set status to fail
            c->status = -1;
        }

        //we don't wait for leftside pid

        //rightside child, consuming input to the pipe
        if ((right_pid = fork()) == 0)
        {   //want to attach rightside stdin to the pipe
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[1]); //not using leftside

            //now execute the right side
            execute_command(rightside, time_travel);
            //if we ever get here, set status to failed
            c->status = -1;
        }
        
        else // parent
        {
            int status = 0;
            if (wait(&status) == -1 || status != 0)
                c->status = -1;
        }

        break;
    }

    //the parser handles subshells and sequence commands already
    default:
        break;
    } //end switch

    //need to reset the fds now
    if (infd)
    {
        dup2(old_stdin, infd);
    }
    if (outfd)
    {
        dup2(old_stdout, fileno(stdout));
        dup2(old_stderr, fileno(stderr));
        close(old_stdout);
        close(old_stderr);
    }
}
