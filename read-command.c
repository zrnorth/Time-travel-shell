// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include "alloc.h"

#include <error.h>
#include <stdio.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */


// Temp stream; just accumulates a single command

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

    //Right now this just tries to create a simple command made of one byte.
    struct command_stream tempstr;
    command_stream_t r;
    r = checked_malloc(sizeof(struct command_stream));
    *r = tempstr; //init
    
    struct command cmd;
    cmd.type = SIMPLE_COMMAND;
    cmd.status = 0;
    cmd.input = 0;
    cmd.output = 0;

    //pointer to the ptr which pts to the first byte in the word.
    //Currently just copies the first byte and stores it in the command.
   

    //This stores the first byte and a pointer to the first byte in dynamic memory.
    //We later return this so it cannot be local
    char* first_byte = checked_malloc(sizeof(char));
    *first_byte = (char)get_next_byte(get_next_byte_argument);

    char** fbptr = checked_malloc(sizeof(char*));
    *fbptr = first_byte;
   
    
    cmd.u.word = fbptr;

    //copy the constructed command to r
    r->c = cmd;
    
    return r;

  //error (1, 0, "command reading not yet implemented");
  //keeping this as a comment because don't remember how to do "error" lol
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
    command_t the_command = &(s->c);
    printf("VALUE RECEIVED: %c\n",*(*(the_command->u.word)));
 
    return the_command;
  
  //Since we currently can just deal with one command, we pull the command and print it.
}
