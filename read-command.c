// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <stdlib.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */


//Command_stream needs to have a head an

struct command_stream
{
	struct command c;
};



command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

    //Right now this just tries to create a simple command from all inputs.
    struct command_stream tempstr;
    command_stream_t r;
    r = checked_malloc(sizeof(struct command_stream));
    *r = tempstr; //init
    
    struct command cmd;
    cmd.type = SIMPLE_COMMAND;
    cmd.status = 0;
    cmd.input = 0;
    cmd.output = 0;

    //Temp implementation: get all the chars from the input script and output as 1 command.
    char* b = checked_malloc(sizeof(char));
    int num_bytes_read = 0;
   
    int the_byte = get_next_byte(get_next_byte_argument);
    while (the_byte > 0) //-1 on EOF
    {
        num_bytes_read++;
        b = checked_realloc(b, num_bytes_read+1); //needs to be null terminated
        *(b + (num_bytes_read-1)*sizeof(char)) = the_byte;
        the_byte = get_next_byte(get_next_byte_argument); 
    }
    *(b + (num_bytes_read)*sizeof(char)) = '\0'; //last value is null-term

    //b holds all the chars input from the shell. Now make a pointer to it and put it
    //in the command stream.
    char** fbptr = checked_malloc(sizeof(char*));
    *fbptr = b;
    cmd.u.word = fbptr;

    r->c = cmd;
    return r;
/*
    //This stores the first byte and a pointer to the first byte in dynamic memory.
    //We later return this so it cannot be local
    char* first_byte = checked_malloc(2 * sizeof(char));
    char* end_byte = first_byte + sizeof(char); //points to what should be the null byte
    *first_byte = (char)get_next_byte(get_next_byte_argument);
    *end_byte = '\0';
    char** fbptr = checked_malloc(sizeof(char*));
    *fbptr = first_byte;
   
    
    cmd.u.word = fbptr;
    //copy the constructed command to r
    r->c = cmd;
    
    return r;
*/
  //error (1, 0, "command reading not yet implemented");
  //keeping this as a comment because don't remember how to do "error" lol
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
    command_t the_command = &(s->c);

    switch (the_command->type)
    {
        case SIMPLE_COMMAND: //leaf of the tree
        {
            char first_byte = *(*(the_command->u.word));

            //need to return the command and then modify the stream so we don't return it again.
            //malloc a new version and delete the old one.

            command_t cmd = checked_malloc(sizeof(struct command));
            *cmd = *the_command; //copy it over
            free(the_command); //delete
            return cmd;
        }
        default:
        {
            //This gets called if the stream is empty (reached the end)
            return 0;
        }
    }
    return 0; //should never reach here (default should catch all)
  //Since we currently can just deal with one command, we pull the command and print it.
}
