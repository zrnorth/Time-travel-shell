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
    
    command_t current_command = NULL;
    command_t top_command = NULL;
    command_t subshell_parent = NULL;
    
    char c = (char)get_next_byte(get_next_byte_argument);
    while ((int)c > 0)
    {
        bool shouldGetAnotherByte = true;
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
          case ' ':
          case '\t':
          {
            if (!current_command) //if there is no current command, first letter
            {
              command_t cmd = checked_malloc(sizeof(struct command));
              cmd->type = SIMPLE_COMMAND;
              cmd->status = -1; //indicates not done yet
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
              if (current_command->type == SIMPLE_COMMAND && current_command->status == -1)
              {
                char* str = *(current_command->u.word);
                int len = strlen(str);
                str = checked_realloc(str, len+1); //increase size by 1 and add new byte
                *(str + (len)*sizeof(char)) = c;
                *(str + len+1 * sizeof(char)) = '\0'; //re-nullterminate

                
                //reinit the ptr
                char** fbptr = checked_malloc(sizeof(char*));
                *fbptr = str;
                current_command->u.word = fbptr;

                break;
              }
              else if (current_command->type == SUBSHELL_COMMAND)
              {
                //inside a subshell, need to point it to a new simple command
                command_t cmd = checked_malloc(sizeof(struct command));
                cmd->type = SIMPLE_COMMAND;
                cmd->status = -1;
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
                cmd->status = -1;
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
                
                current_command->u.command[1] = cmd; //rightside pts to new branch
                current_command = cmd;
                //top stays the same.


                break;
              }
            }
          }

          //redirs
          case '<':
          case '>':
          {
            if (!current_command)
              error(1, 0, "syntax error: can't start with an IO redirect");
            if (current_command->type != SIMPLE_COMMAND) //has to be inside a simple command
              error(1, 0, "syntax error: IO redirect");
            if (c == '<' && !current_command->input && !current_command->output) //valid input
            {
                shouldGetAnotherByte = false;
                char* str = checked_malloc(sizeof(char));
                *str = '\0';
                int len = 0;
                while (true)
                {
                  c = (char)get_next_byte(get_next_byte_argument);
                  if ((int)c < 0) break; //eof
                  if (c == '>' || c == '|' || c == '&' || c == '\n' || c=='(' || c==')') break; //switch to output
                  if (c == ' ' || c == '\t') continue; // ignore whitespace
                  //lengthen string
                  len++;
                  str = checked_realloc(str, len+1); //increase size by 1
                  *(str + (len-1)*sizeof(char)) = c;
                  *(str + (len)*sizeof(char)) = '\0';
                }
                current_command->input = str; //set the input to this string
            }
            if (c == '>' && !current_command->output) //valid output
            {
                shouldGetAnotherByte = false;
                char* str = checked_malloc(sizeof(char));
                *str = '\0';
                int len = 0;
                while (true)
                {
                  c = (char)get_next_byte(get_next_byte_argument);
                  if ((int)c < 0) break; //eof
                  if (c == '>' || c == '|' || c == '&' || c == '\n' || c=='(' || c == ')') break; //switch to output
                  if (c == ' ' || c == '\t') continue;
                  //lengthen string
                  len++;
                  str = checked_realloc(str, len+1); //increase size by 1
                  *(str + (len-1)*sizeof(char)) = c;
                  *(str + (len)*sizeof(char)) = '\0';
                }
                current_command->output = str;
             }
            current_command->status = 0; //no more input possible for this command   
            break;
          } 
/*
          //subshell stuff
          case '(':
          {
            if (current_command && current_command->type == SIMPLE_COMMAND)
              error(1, 0, "syntax error");
            else
            {
              command_t cmd = checked_malloc(sizeof(struct command));
              cmd->type = SUBSHELL_COMMAND;
              cmd->status = -1;
              cmd->input = 0; 
              cmd->output = 0;
              
              subshell_parent = current_command;

              shouldGetAnotherByte = false;
              char* str = checked_malloc(sizeof(char));
              *str = '\0';
              int len = 0;

              command_t subshell_cmd = checked_malloc(sizeof(struct command));
              cmd->type = SIMPLE_COMMAND;
              cmd->status = -1;
              cmd->input = 0; 
              cmd->output = 0;
              while (true)
              {
                c = (char)get_next_byte(get_next_byte_argument);
                if ((int)c < 0) break; //eof
                if (c == ')')
                {
                  shouldGetAnotherByte = true;
                  break;
                }
                //if (c == '(')
                  // do something
                len++;
                str = checked_realloc(str, len+1);
                *(str + (len-1)*sizeof(char)) = c;
                *(str + (len)*sizeof(char)) = '\0';
              }
              subshell_cmd->u.word = &str;

              cmd->u.subshell_command = subshell_cmd; //point the outer at the new cmd
            }
            break;
          } */
            
          case '(':
          case ')': //just ignore
          {
            break;
          }
          
          
          //special characters
          case ';':
          case '|':
          case '&':
          case '\n':
          {
            //need to get which command type first
            enum command_type t;
            if (c == '|') //need to check if it is a pipe or an OR
            {
                char next = (char)get_next_byte(get_next_byte_argument);
                if (next == '|') //OR
                    t = OR_COMMAND;
                else
                {
                    shouldGetAnotherByte = false;
                    c = next;
                    t = PIPE_COMMAND;
                }
            }
            else if (c == '&') //check for syntax error here
            {
                char next = (char)get_next_byte(get_next_byte_argument);
                if (next != '&')
                    error(1, 0, "syntax error in input");
                else
                    t = AND_COMMAND;
            }
            else //c == ;
            {
                char next = (char)get_next_byte(get_next_byte_argument);
                shouldGetAnotherByte = false;
                c = next;
                if ((int)next < 0) //eof
                    break; //don't make another node, just exit.
                else //continue
                    t = SEQUENCE_COMMAND;
            }

            if (!current_command) //if NULL, just continue
              break;
            else if (current_command->type == SIMPLE_COMMAND)
            {
              //grow upwards; the current command becomes the leftside
              command_t cmd = checked_malloc(sizeof(struct command));
              cmd->type = t;
              cmd->status = 0;
              cmd->input = 0;
              cmd->output = 0;
              cmd->u.command[0] = top_command; //leftside points to the current
              current_command->status = 0; //done parsing it

              current_command = cmd;
              top_command = cmd; //new "top" bc growing upwards
              
              break;
            }
            else if (current_command->type == SUBSHELL_COMMAND)
            {
              command_t cmd = checked_malloc(sizeof(struct command));
              cmd->type = t;
              cmd->status = 0;
              cmd->input = 0;
              cmd->output = 0;
              cmd->u.command[0] = subshell_parent; //leftside points to the current
              
              current_command = cmd;
              //modify the subshell parent to point to this new command
              subshell_parent->u.subshell_command = current_command;
            }
            else //just continue otherwise
              break;
          }
          default: break;
        } //END SWITCH
        //goto next byte in the loop
        if(shouldGetAnotherByte)
        {
          char next_byte = (char)get_next_byte(get_next_byte_argument);
          printf("C=  %i\n", next_byte);
          c = next_byte;
        }
        else
            continue; //already have the next byte
    }//END WHILE
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
            if (the_command->status != 2) //first time being visited
            {
                the_command->status = 2; //2 means don't check this anymore
                return the_command; 
            }
            else
                return NULL;
        }
        case SUBSHELL_COMMAND: //we are in a subshell, so need to execute the subcommands
        {
            command_t subcmd = the_command->u.subshell_command;
            if (!subcmd || subcmd->status !=0) 
                return NULL;
            else
            {
                struct command_stream new;
                new.c = *subcmd;
                command_stream_t n = &new;
                the_command->status = 2;
                return read_command_stream(n);
            }
        }
        case SEQUENCE_COMMAND:
        case AND_COMMAND:
        case OR_COMMAND:
        case PIPE_COMMAND:
        {
            if (the_command->status == 0)
            {
                the_command->status = 1;
                return the_command;
            }
            else
                return NULL;
        }


        default:
        {
            //This gets called if the stream is empty (reached the end)
            return NULL;
        }
    }
    return 0; //should never reach here (default should catch all)
}
