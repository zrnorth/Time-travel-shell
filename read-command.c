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
    int num_commands;
};

typedef enum //used for a tokenizer function
{
    NONE,
    WORD,
    SEMICOLON,
    NEWLINE,
    PIPE,
    AND,
    OR,
    BEGIN_SUBSHELL,
    END_SUBSHELL,
    INPUT,
    OUTPUT
} TOKEN_TYPE;

//returns a stream of tokenized commands

typedef struct
{
    char* token_str;
    TOKEN_TYPE type;
} token ;

typedef struct
{
    token* tok;
    int length;
} token_list;

void syntax_error() //call this when there is some sort of syntax error
{
    printf("Syntax error in input file\n");
    exit(1);
}

//helper function to append a token to the token list.
//DOES NOT MODIFY THE LENGTH INT. need to do that manually.
token* append_token_to_list (token* old_list, int old_length, token new)
{
    //make a new list one larget
    old_list = checked_realloc(old_list, ((old_length+1) * sizeof(token)));
    *(old_list + old_length) = new; //append the new value
    return old_list;
}

char* str_cat_char(char* string, char new_char) //helper function to append a char to a str
{

    int length = strlen(string)+1;
    string = checked_realloc(string, length);
    string[length-1] = new_char;
    string[length] = 0;
    return string;
}


//tokenizes the string and returns it in a tidy list.
token_list*
tokenize_string (char* input)
{
    token_list* tl = checked_malloc(sizeof(token_list));
    tl->tok = NULL;
    tl->length = 0;


    //initialize our first token
    token curr;
    curr.token_str = NULL;
    curr.type = NONE;
    
    int str_length = strlen(input);
    int i;
    for (i = 0; i < str_length; i++)
    {
        char c = input[i];
        switch(c)
        {
        case '#': //comment
        {
            //its a syntax error if a comment is immediately preceded by a token.
            //these are the only valid instances:
            if (!input[i-1] || input[i-1] == ' ' || input[i-1] == '\n' || input[i-1] == '\t')
            {
                while (input[i] != '\n' && i < str_length)
                {
                    i++; //just continue until a newline is found, or EOF.
                }
            }
            else 
                syntax_error();
            break;
        }
        case ';':
        case '\n': //end of seq token
        {
            //first, append current token if it is not NULL (if it is just ignore)
            if (curr.type != NONE)
            {
                tl->tok = append_token_to_list(tl->tok, tl->length, curr);
                tl->length++;
            }
            //next, reinit to a SEQ token
            
            char* new_str = checked_malloc(2 * sizeof(char));
            new_str[0] = c;
            new_str[1] = '\0';
            
            curr.token_str = new_str; 
            if (c == ';') 
                curr.type = SEMICOLON;
            else
                curr.type = NEWLINE;
            //continue with next char
            break;
        }

        case '|': //need to check if pipe or OR
        {
            if (curr.type != NONE)
            {
                tl->tok = append_token_to_list(tl->tok, tl->length, curr);
                tl->length++;
            }
            if (i == str_length-1 || input[i+1] != '|') //PIPE
            {
                char* new_str = checked_malloc(2 * sizeof(char));
                new_str[0] = c;
                new_str[1] = '\0';
                curr.token_str = new_str;
                curr.type = PIPE;
            }
            else //AND
            {
                char* new_str = checked_malloc(3 * sizeof(char));
                new_str[0] = c;
                new_str[1] = input[i+1]; //should be '|', of course.
                new_str[2] = '\0';
                curr.token_str = new_str;
                curr.type = OR;
                i++; //to avoid reading twice
            }
            break;
        }

        case '&': //need to check if to &&s, else syntax error
        {
            if (curr.type != NONE)
            {
                tl->tok = append_token_to_list(tl->tok, tl->length, curr);
                tl->length++;
            }
            if (i == str_length-1 || input[i+1] != '&') //syntax error; need &&
                syntax_error();
            else
            {
                char* new_str = checked_malloc(3 * sizeof(char));
                new_str[0] = c;
                new_str[1] = input[i+1]; //should be '|', of course.
                new_str[2] = '\0';
                curr.token_str = new_str;
                curr.type = AND;
                i++; //so we don't read it twice
            }
            break;
        }

        case '(': //beginning of a subshell
        {
            if (curr.type != NONE)
            {
                tl->tok = append_token_to_list(tl->tok, tl->length, curr);
                tl->length++;
            }
            char* new_str = checked_malloc(2 * sizeof(char));
            new_str[0] = c;
            new_str[1] = '\0';
            curr.token_str = new_str;
            curr.type = BEGIN_SUBSHELL;
            break;
        }

        case ')': //beginning of a subshell
        {
            if (curr.type != NONE)
            {
                tl->tok = append_token_to_list(tl->tok, tl->length, curr);
                tl->length++;
            }
            char* new_str = checked_malloc(2 * sizeof(char));
            new_str[0] = c;
            new_str[1] = '\0';
            curr.token_str = new_str;
            curr.type = END_SUBSHELL;
            break;
        }
        
        case '<': //beginning of a subshell
        {
            if (curr.type != NONE)
            {
                tl->tok = append_token_to_list(tl->tok, tl->length, curr);
                tl->length++;
            }
            char* new_str = checked_malloc(2 * sizeof(char));
            new_str[0] = c;
            new_str[1] = '\0';
            curr.token_str = new_str;
            curr.type = INPUT;
            break;
        }
        case '>': //beginning of a subshell
        {
            if (curr.type != NONE)
            {
                tl->tok = append_token_to_list(tl->tok, tl->length, curr);
                tl->length++;
            }
            char* new_str = checked_malloc(2 * sizeof(char));
            new_str[0] = c;
            new_str[1] = '\0';
            curr.token_str = new_str;
            curr.type = OUTPUT;
            break;
        }
       
        case '\t':
        case ' ': //these cases we want to ignore if not inside a word
        {
            if (curr.type != WORD)
                break; //ignore
            //else, fall through to WORD handlinga
        }

        default: // WORD
        {
            if (curr.type != WORD) //need to commit the token
            {
                if (curr.type != NONE)
                {
                    tl->tok = append_token_to_list(tl->tok, tl->length, curr);
                    tl->length++;
                }
            
                curr.type = WORD;
                curr.token_str = checked_malloc(sizeof(char));
                *curr.token_str = '\0';
            }
            str_cat_char(curr.token_str, c);
            break;
        }
        } //end switch
    } //end for
    if (curr.type != NONE) //append the last token
    {

        tl->tok = append_token_to_list(tl->tok, tl->length, curr);
        tl->length++;
    }
    return tl;
}

//puts the entire stream into memory for ease
char*
create_command_string(int (*get_next_byte) (void *), void *gnba)
{
    char* the_string = checked_malloc(sizeof(char));
    *the_string = '\0';
    int b;
    while ((b = get_next_byte(gnba)) > 0) //read to eof
    {
        if ((char)b == '\r') continue; //ignore carriage returns
        the_string = str_cat_char(the_string, (char)b);
    }
    
    for (b = 0; b < strlen(the_string); b++)
        printf("%c", *(the_string+b));

    //remove trailing "\n" or ';' or '\r's, just to make things simpler later

    return the_string;
}

//helper function to initialize a simple command from a token
command_t
init_simple_cmd(token* tok)
{
    command_t cmd = checked_malloc(sizeof(struct command));
    cmd->type = SIMPLE_COMMAND;
    cmd->status = 0;
    cmd->input = 0;
    cmd->output = 0;
    //need a ptr ptr for the word
    char** fbptr = checked_malloc(sizeof(char*));
    *fbptr = tok->token_str;
    cmd->u.word = fbptr;

    return cmd;
}

command_t
init_compound_cmd(command_t leftside, enum command_type type)
{
    command_t cmd = checked_malloc(sizeof(struct command));
    cmd->type = type;
    cmd->status = -1; //incomplete statement; need a rightside.
    cmd->input = 0;
    cmd->output = 0;
    //need to point the leftside to the given command
    cmd->u.command[0] = leftside;
    return cmd;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
    //parse the stream into a string for ease of use
    char* str = create_command_string(get_next_byte, get_next_byte_argument);
    //create a list of tokens from the input string
    token_list* t_list = tokenize_string(str);
   
    //now we need to parse the tokens
    
    int num_commands = 0;
    command_t prev_cmd = NULL; //used to track which commands are parsed
    command_t top_cmd = NULL;  //keep track of the top of the command tree.

    int i;
    for (i = 0; i < t_list->length; i++)
    {
        //TODO: debug
        printf("\ntoken #: %i ... with type %i\n", i, t_list->tok[i].type);
        int j;
        for (j = 0; j < strlen(t_list->tok[i].token_str); j++)
            printf("%c", t_list->tok[i].token_str[j]);
        

        //TODO: end debug
        switch(t_list->tok[i].type)
        {
        case NONE: //should never have a token with NONE type; error
            syntax_error();
            break;
        case WORD:
        {
            //TODO: unsure if this is correct syntaxwise
            if (prev_cmd && prev_cmd->type == SIMPLE_COMMAND)
                syntax_error(); //can't have 2 simple commands in a row
            else
            {
                command_t cmd = init_simple_cmd(&(t_list->tok[i])); //create a simple command
                num_commands++;
                if (!prev_cmd) // first command in list
                {
                    prev_cmd = cmd;
                    top_cmd = cmd;
                    break;
                }
                else if (prev_cmd->type == AND_COMMAND || 
                         prev_cmd->type == OR_COMMAND  ||
                         prev_cmd->type == SEQUENCE_COMMAND ||
                         prev_cmd->type == PIPE_COMMAND)
                {
                    prev_cmd->u.command[1] = cmd; //rightside points to new word
                    prev_cmd->status = 0; //indicates a "complete" statement
                    prev_cmd = cmd;
                    
                }
                else if (prev_cmd->type == SUBSHELL_COMMAND)
                {
                    //TODO: need a way out of the tree
                    prev_cmd->u.subshell_command = cmd;
                    prev_cmd = cmd;
                }
                else //some sort of error
                    syntax_error();
            }
            break;
        }
        case NEWLINE: //basically the same as a semicolon except we can ignore in many cases
        {
            if (!prev_cmd || i == t_list->length - 1) //ignore leading / trailing newlines.
                break;
            TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;
            TOKEN_TYPE next_token_type = t_list->tok[i+1].type;
            if (prev_token_type == INPUT || prev_token_type == OUTPUT)
                syntax_error();
            if (prev_cmd->type == SEQUENCE_COMMAND)
                break; //not a syntax error, but ignore subsequent newlines after \n or ;
            if (next_token_type != BEGIN_SUBSHELL &&
                next_token_type != END_SUBSHELL   &&
                next_token_type != WORD)
                    syntax_error();
            //else, this is a valid newline and we want to treat it like a semicolon
            command_t cmd = init_compound_cmd(top_cmd, SEQUENCE_COMMAND);
            top_cmd = cmd;
            num_commands++;
            prev_cmd = cmd;
            break;
        }
        
        case SEMICOLON:
        {
            //only valid if previous command was a simple command or sequence
            if (!prev_cmd) syntax_error();
            if (i == 0 || i == t_list->length - 1) //we ignore trailing semicolons
                break;
            TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;
            TOKEN_TYPE next_token_type = t_list->tok[i+1].type;

            if ((prev_token_type == WORD || 
                 prev_token_type == END_SUBSHELL) && 

               ( next_token_type == WORD ||
                 next_token_type == BEGIN_SUBSHELL ||
                 next_token_type == NEWLINE))
            {
                //TODO: need a way "up" the tree
                command_t cmd = init_compound_cmd(top_cmd, SEQUENCE_COMMAND);
                top_cmd = cmd;
                num_commands++;
                prev_cmd = cmd;
                break;
            }
            else syntax_error();

        }

        case PIPE:
        case AND:
        case OR:
        {
            
            if (!prev_cmd || i == t_list->length - 1) syntax_error();
            TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;
            TOKEN_TYPE next_token_type = t_list->tok[i+1].type;


            if ((prev_token_type == WORD ||
                 prev_token_type == END_SUBSHELL) &&

                (next_token_type == NEWLINE ||
                 next_token_type == WORD    ||
                 next_token_type == BEGIN_SUBSHELL))
            {
                //TODO: up the tree
                command_t cmd;
                if (t_list->tok[i].type == PIPE)
                    cmd = init_compound_cmd(top_cmd, PIPE_COMMAND);
                else if (t_list->tok[i].type == AND)
                    cmd = init_compound_cmd(top_cmd, AND_COMMAND);
                else if (t_list->tok[i].type == OR)
                    cmd = init_compound_cmd(top_cmd, OR_COMMAND);
                else //huh wtf
                    syntax_error();
                top_cmd = cmd;
                num_commands++;
                prev_cmd = cmd;
                break;
            }
            else syntax_error();
        }
        case BEGIN_SUBSHELL:
            break;
        case END_SUBSHELL:
            break;
        case INPUT:
        {
            if (!prev_cmd || i == t_list->length - 1) syntax_error();
            TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;

            if (prev_token_type != WORD || prev_cmd->type != SIMPLE_COMMAND) //only valid syntax
                syntax_error();
            //need to get the input first, and then check for output.
            i++;
            TOKEN_TYPE input_type = t_list->tok[i].type;
            if (input_type != WORD || prev_cmd->input) syntax_error();
            else
                prev_cmd->input = t_list->tok[i].token_str;
            if (t_list->tok[i+1].type == OUTPUT) //we have an output as well
            {
                i++; //move to the output token
                i++; //move to the output word
                TOKEN_TYPE output_type = t_list->tok[i].type;
                if (output_type != WORD || prev_cmd->output) syntax_error();
                else
                    prev_cmd->output = t_list->tok[i].token_str;
            }
            //else, just continue. allowed to specify an input and no output.
            break;
        }
        case OUTPUT:
        {
            if (!prev_cmd || i == t_list->length - 1) syntax_error();
            
            TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;
            
            if (prev_token_type != WORD || prev_cmd->type != SIMPLE_COMMAND)
                syntax_error();
            i++; //move to the output word
            TOKEN_TYPE output_type = t_list->tok[i].type;
            if (output_type != WORD || prev_cmd->output) syntax_error();
            else
                prev_cmd->output = t_list->tok[i].token_str;
            break;
        }

        } //end switch
    }//end for
    struct command_stream temp;
    command_stream_t r;
    r = checked_malloc(sizeof(struct command_stream));
    *r = temp; //init to blank.
    r->c = *top_cmd;
    r->num_commands = num_commands;
    //printf("Top command: %s\n", *(top_cmd->u.word));
    printf("Number of commands: %i\n", num_commands);
    return r;
}

command_t
read_command_stream (command_stream_t s)
{
    command_t the_command = &(s->c);
    switch (the_command->type)
    {
        case SIMPLE_COMMAND:
        {
            if (the_command->status != 2) //2 means do not visit anymore
            {
                the_command->status = 2;
                printf("Returning a command with: %s\n", *(the_command->u.word));
                return the_command;
            }
            else return NULL; //already visited this leaf
        }
        case SUBSHELL_COMMAND: //TODO: implement
            return NULL;

        case SEQUENCE_COMMAND:
        case AND_COMMAND:
        case OR_COMMAND:
        case PIPE_COMMAND:
        {
            if (the_command->status == 0) //havent visited either child node
            {
                the_command->status = 1;
                return the_command;
            }
            else return NULL;
        }
        default: return NULL; //should never get here but ya never knokw.
    }
    return 0; 
}
