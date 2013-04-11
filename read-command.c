// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    /*
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
    */ 
    //new
    
    command_t current_command = NULL;
    command_t top_command = NULL;
    command_t subshell_parent = NULL;
    
    int byte = get_next_byte(get_next_byte_argument);
    while (byte > 0)
    {
        char c = (char)byte;
        switch(c)
        {
          //normal chars
          case 'a':
          case 'A':
          case 'b':
          case 'B':
          case 'c':
          case 'C':
          case 'd':
          case 'D':
          case 'e':
          case 'E':
          case 'f':
          case 'F':
          case 'g':
          case 'G':
          case 'h':
          case 'H':
          case 'i':
          case 'I':
          case 'j':
          case 'J':
          case 'k':
          case 'K':
          case 'l':
          case 'L':
          case 'm':
          case 'M':
          case 'n':
          case 'N':
          case 'o':
          case 'O':
          case 'p':
          case 'P':
          case 'q':
          case 'Q':
          case 'r':
          case 'R':
          case 's':
          case 'S':
          case 't':
          case 'T':
          case 'u':
          case 'U':
          case 'v':
          case 'V':
          case 'w':
          case 'W':
          case 'x':
          case 'X':
          case 'y':
          case 'Y':
          case 'z':
          case 'Z':
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case '!':
          case '%':
          case '+':
          case ',':
          case '-':
          case '.':
          case '/':
          case ':':
          case '@':
          case '^':
          case '_':
          {
            if (!current_command) //if there is no current command, first letter
            {
              //DEBUG
              printf("NO CURRENT COMMAND, CREATING ONE.\n");

              command_t cmd = checked_malloc(sizeof(struct command));
              cmd->type = SIMPLE_COMMAND;
              cmd->status = 0;
              cmd->input = 0;
              cmd->output = 0;
              //need to make a new string for this command to point to
              char* str = checked_malloc(2 * sizeof(char));
              *str = c;
              *(str + sizeof(char)) = '\0'; //null terminated

              //need to point the command at this string
              char** fbptr = checked_malloc(sizeof(char*));
              *fbptr = str;
              cmd->u.word = fbptr;

              current_command = cmd;
              top_command = cmd;

              //continue with next byte
              break;
            }
            else //there is a current command
            {
              if (current_command->type == SIMPLE_COMMAND)
              {
                //DEBUG
                printf("CURRENT COMMAND, APPENDING.\n");
                printf("%c\n", byte);
                char* str = *(current_command->u.word);
                int len = strlen(str);
                str = checked_realloc(str, len+1); //increase size by 1 and add new byte
                *(str + (len)*sizeof(char)) = c;
                *(str + len+1 * sizeof(char)) = '\0'; //re-nullterminate

                
                //reinit the ptr
                char** fbptr = checked_malloc(sizeof(char*));
                *fbptr = str;
                current_command->u.word = fbptr;

                printf("The third value is: %x\n", *(*fbptr+2));
                break;
              }
              else if (current_command->type == SUBSHELL_COMMAND)
              {
                //inside a subshell, need to point it to a new simple command
                command_t cmd = checked_malloc(sizeof(struct command));
                cmd->type = SIMPLE_COMMAND;
                cmd->status = 0;
                cmd->input = 0;
                cmd->output = 0;
                //need to make a new string for this command to point to
                char* str = checked_malloc(2 * sizeof(char));
                *str = c;
                *(str + sizeof(char)) = '\0'; //null terminated

                //need to point the command at this string
                char** fbptr = checked_malloc(sizeof(char*));
                *fbptr = str;
                cmd->u.word = fbptr;

                //point the old command to this new one and then "go into" it for next byte
                current_command->u.subshell_command = cmd;
                subshell_parent = current_command;
                //change current to the "inside"
                current_command = cmd;
                
                //top does not move so we can "get back"
        
                break;
              }

              else //AND, OR, SEQ, or PIPE
              {
                //need to make the "rightside" for this tree
                command_t cmd = checked_malloc(sizeof(struct command));
                cmd->type = SIMPLE_COMMAND;
                cmd->status = 0;
                cmd->input = 0;
                //need to make a new string for this command to point to
                char* str = checked_malloc(2 * sizeof(char));
                *str = c;
                *(str + sizeof(char)) = '\0'; //null terminated

                //need to point the command at this string
                char** fbptr = checked_malloc(sizeof(char*));
                *fbptr = str;
                cmd->u.word = fbptr;

                current_command->u.command[1] = cmd; //rightside pts to new branch
                current_command = cmd;
                //top stays the same.


                break;
              }
            }
          }
        }
        //goto next byte in the loop
        byte = get_next_byte(get_next_byte_argument);
    }
    
    //DEBUG
    printf("EXITING.\n");
    struct command_stream temp;
    command_stream_t r;
    r = checked_malloc(sizeof(struct command_stream));
    *r = temp; //init
    r->c = *top_command; //point it to the top of the tree
    return r;
              

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
            //need to return the command and then modify the stream so we don't return it again.
            //malloc a new version and delete the old one.

            command_t cmd = checked_malloc(sizeof(struct command));
            *cmd = *the_command; //copy it over
            free(the_command); //delete
            return cmd;
        }
        //In the other cases need to use recursion to go down the tree, and the
        //command "status" variable to remember which direction to go
        default:
        {
            //This gets called if the stream is empty (reached the end)
            return 0;
        }
    }
    return 0; //should never reach here (default should catch all)
  //Since we currently can just deal with one command, we pull the command and print it.
}
