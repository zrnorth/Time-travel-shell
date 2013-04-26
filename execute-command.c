// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
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
    c->status = 0;

    int old_stdin  = dup(STDIN_FILENO);
    int old_stdout = dup(STDOUT_FILENO);
    int old_stderr = dup(STDERR_FILENO);

    int infd = -1;
    int outfd = -1;

    if (c->input)
    {
        FILE* in = fopen(c->input, "r");
        if (in)
        {
            infd = fileno(in);
            dup2(infd, STDIN_FILENO);
        }
        else
        {
            error(1, 0, "%s: No such file or directory", c->input);
            c->status = -1;
            exit(1);
        }
    }
    if (c->output)
    {
        FILE* out = fopen(c->output, "w");
        if (out)
        {
            outfd = fileno(out);
            dup2(outfd, STDOUT_FILENO);
            dup2(outfd, STDERR_FILENO);
        }
        else
        {
            error(1, 0, "%s: Could not open the file", c->output);
            c->status = -1;
            exit(1);
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
        }
        else //parent
        {
            waitpid(pid, &(c->status), 0);
        }
        break;
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
            exit(0);
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
            exit(0);
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

        command_t leftside = c->u.command[0];
        command_t rightside = c->u.command[1];

        pid_t p = fork();
        if (p == -1) 
        {
            error(1, 0, "Fork failed in pipe execution.");
            c->status = 1;
            exit(1);
        }
        else if (p == 0) //run everything in the child, and wait for it to finish
        {
            int pipefd[2]; //pipe file descriptors
            if (pipe(pipefd)) //pipe error of some sort.
            {
                error(1, 0, "Pipe creation failure");
                c->status = -1;
                exit(1);
            }

            pid_t r = fork();
            if (r == -1)
            {
                error(1, 0, "Fork failed in pipe execution.");
                c->status = 1;
                exit(1);
            }
            else if (!r) //leftside
            {
                close (pipefd[0]);
                dup2(pipefd[1], 1);
                close(pipefd[1]);
                execute_command(leftside, time_travel);
                exit(0);
            }
            else //rightside
            {
                int status;
                wait(&status);
                close(pipefd[1]);
                dup2(pipefd[0], 0);
                close(pipefd[0]);
                execute_command(rightside, time_travel);
            }
        }
        else
        {
            wait(&c->status);
            exit(c->status);
        }
        break;
    }

    //the parser handles subshells and sequence commands already
    default:
        break;
    } //end switch

    //need to reset the fds now
    if (infd != -1)
    {
        dup2(old_stdin, STDIN_FILENO);
        close(old_stdin);
    }
    if (outfd != -1)
    {
        dup2(old_stdout, STDOUT_FILENO);
        dup2(old_stderr, STDERR_FILENO);
        close(old_stdout);
        close(old_stderr);
    } 
}
