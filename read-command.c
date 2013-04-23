// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */


//Command_stream needs to have a head an



struct command_stream
{
	command_t c;
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

void syntax_error(int line) //call this when there is some sort of syntax error
{
    fprintf(stderr, "%i: Syntax error in input file\n", line);
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
        case '`': syntax_error(__LINE__);
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
                syntax_error(__LINE__);
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
                syntax_error(__LINE__);
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
    //DEBUG
    /*
    for (b = 0; b < strlen(the_string); b++)
        printf("%c", *(the_string+b));
    */
    //remove trailing "\n" or ';' or '\r's, just to make things simpler later

    return the_string;
}


//helper function that removes leading and trailing whitespace, and 
//returns a syntax error if there is more than one word in a word token.
//Used to help with the input / output pipes (can only be 1 word)
char*
trim_whitespace(char* input, bool inside_spaces_allowed)
{
    char* end;
    while (isspace(*input)) input++; //ignore leading spaces
    if (*input == 0)
        syntax_error(__LINE__); //this indicates all spaces (bad)

    end = input + strlen(input) - 1;
    while (end > input && isspace(*end))
        end--; //get rid of trailing spaces

    *(end+1) = 0; //nullterminate
    if (!inside_spaces_allowed)
    {
        unsigned int i;

        for (i = 0; i < strlen(input); i++) //iterate through the new string
        {
            if (input[i] == ' ') //inside space
                syntax_error(__LINE__);
        }
    }
    //else, all good
    return input;
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
    *fbptr = trim_whitespace(tok->token_str, true);

    cmd->u.word = fbptr;

    return cmd;
}

command_t
init_compound_cmd(command_t leftside, enum command_type type)
{
    command_t cmd = checked_malloc(sizeof(struct command));
    cmd->type = type;
    cmd->status = 0; 
    cmd->input = 0;
    cmd->output = 0;
    //need to point the leftside to the given command
    if (type == SUBSHELL_COMMAND)
        cmd->u.subshell_command = leftside;
    else
    {
        cmd->u.command[0] = leftside;
        cmd->u.command[1] = NULL;
    }
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
    
    int depth = 0; //for subshells
    command_t prev_cmd = NULL; //used to track which commands are parsed
    command_t top_cmd = NULL;  //keep track of the top of the command tree.
    command_t top_seq_cmd = NULL;
    command_t last_word_parsed = NULL;

    int i;
    for (i = 0; i < t_list->length; i++)
    {
        switch(t_list->tok[i].type)
        {
        case NONE: //should never have a token with NONE type; error
            syntax_error(__LINE__);
            break;
        case WORD:
        {
            //TODO: unsure if this is correct syntaxwise
            if (prev_cmd && prev_cmd->type == SIMPLE_COMMAND)
                syntax_error(__LINE__); //can't have 2 simple commands in a row
            else
            {
                command_t cmd = init_simple_cmd(&(t_list->tok[i])); //create a simple command
                last_word_parsed = cmd;
                if (!prev_cmd)
                {
                    prev_cmd = cmd;
                    top_cmd = cmd;
                    last_word_parsed = cmd;
                    break;
                }
                else if (prev_cmd->type == PIPE_COMMAND) //this has a higher precedence
                {
                    prev_cmd->u.command[1] = cmd;
                    prev_cmd->status = 0;
                    prev_cmd = cmd;
                    last_word_parsed = cmd;
                }
                else if (prev_cmd->type == AND_COMMAND || //lower precedence, so need to check
                         prev_cmd->type == OR_COMMAND  ||
                         prev_cmd->type == SEQUENCE_COMMAND)
                {
                    if (i < t_list->length - 1)
                    {
                        TOKEN_TYPE next_token_type = t_list->tok[i+1].type;
                        if (next_token_type == PIPE) //we need to construct this first, then.
                        {
                            command_t pipe_command = init_compound_cmd(cmd, PIPE_COMMAND);
                            int search = i+2; //we're moving to the "rightside"
                            while (t_list->tok[search].type == NEWLINE) 
                                search++; //skip the newlines
                            if (t_list->tok[search].type == WORD) 
                            {
                                command_t rightside = init_simple_cmd(&(t_list->tok[search]));
                                pipe_command->u.command[1] = rightside;
                                pipe_command->status = 0; //complete the pipe
                                last_word_parsed = rightside;
                                cmd = pipe_command; //change current command
                                i = search; //jump ahead
                            }
                            else syntax_error(__LINE__);
                        }
                        //else, do nothing, because syntax doesn't fit.
                    }
                    prev_cmd->u.command[1] = cmd; //rightside points to new word/pipe
                    prev_cmd->status = 0; //indicates a "complete" statement
                    prev_cmd = cmd;
                    if (!top_cmd) //first item in new tree
                        top_cmd = cmd;
                }
                else if (prev_cmd->type == SUBSHELL_COMMAND)
                {
                    //TODO: need a way out of the tree
                    prev_cmd->u.subshell_command = cmd;
                    prev_cmd = cmd;
                    top_cmd = cmd;
                }
                else //some sort of error
                    syntax_error(__LINE__);
            }
            break;
        }
        case NEWLINE: //basically the same as a semicolon except we can ignore in many cases
        {
            if (!prev_cmd) //ignore leading newlines.
                break;
            if (prev_cmd->type == OR_COMMAND || prev_cmd->type == AND_COMMAND)
                break; //just whitespace.
            while (i < t_list->length - 1 && t_list->tok[i+1].type == NEWLINE) 
                i++; //ignore multiple newlines.
            if (i == t_list->length - 1) break; //ignore trailing newlines.

            TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;
            TOKEN_TYPE next_token_type = t_list->tok[i+1].type;
            if (prev_token_type == INPUT || prev_token_type == OUTPUT)
                syntax_error(__LINE__);
            if (next_token_type != BEGIN_SUBSHELL &&
                next_token_type != END_SUBSHELL   &&
                next_token_type != WORD)
                    syntax_error(__LINE__);
            //else, this is a valid newline and we want to treat it like a semicolon
            command_t cmd;
            if (top_seq_cmd)
                cmd = init_compound_cmd(top_seq_cmd, SEQUENCE_COMMAND);
            else //first seq command
                cmd = init_compound_cmd(top_cmd, SEQUENCE_COMMAND);
            top_seq_cmd = cmd;
            top_cmd = NULL; //initiate a new tree
            prev_cmd = cmd;
            last_word_parsed = NULL; //because syntax
            break;
        }
        
        case SEMICOLON:
        {
            //only valid if previous command was a simple command or sequence
            if (!prev_cmd) syntax_error(__LINE__);
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
                command_t cmd;
                if (top_seq_cmd)
                    cmd = init_compound_cmd(top_seq_cmd, SEQUENCE_COMMAND);
                else //first seq command
                    cmd = init_compound_cmd(top_cmd, SEQUENCE_COMMAND);
                top_seq_cmd = cmd;
                top_cmd = NULL; //initiate a new tree
                prev_cmd = cmd;
                last_word_parsed = NULL;
                break;
            }
            else syntax_error(__LINE__);

        }

        case PIPE:
        case AND:
        case OR:
        {
            
            if (!prev_cmd || i == t_list->length - 1) syntax_error(__LINE__);
            TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;
            TOKEN_TYPE next_token_type = t_list->tok[i+1].type;


            if ((prev_token_type == WORD ||
                 prev_token_type == END_SUBSHELL) &&

                (next_token_type == NEWLINE ||
                 next_token_type == WORD    ||
                 next_token_type == BEGIN_SUBSHELL))
            {
                command_t cmd;
                if (t_list->tok[i].type == PIPE)
                    cmd = init_compound_cmd(top_cmd, PIPE_COMMAND);
                else if (t_list->tok[i].type == AND)
                    cmd = init_compound_cmd(top_cmd, AND_COMMAND);
                else if (t_list->tok[i].type == OR)
                    cmd = init_compound_cmd(top_cmd, OR_COMMAND);
                else //huh wtf
                    syntax_error(__LINE__);

                if (top_seq_cmd) //the current top cmd is a sequence cmd
                    top_seq_cmd->u.command[1] = cmd; //set the top val to be this new cmd.
                top_cmd = cmd;
                prev_cmd = cmd;
                break;
                
            }
            else syntax_error(__LINE__);
        }
        case BEGIN_SUBSHELL:
        {
            //need to init a subshell and continue with new tree
            if (i == t_list->length - 1) 
                syntax_error(__LINE__);
            if (i > 0)
            {
                TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;
                if (prev_token_type != SEMICOLON && prev_token_type != NEWLINE &&
                    prev_token_type != BEGIN_SUBSHELL) //only 3 allowed preceding tokens
                        syntax_error(__LINE__);
            }
            //need to create a new tree
            command_t cmd = init_compound_cmd(NULL, SUBSHELL_COMMAND);
            top_cmd = NULL; 
            prev_cmd = cmd;
            depth++;
            break;
        }
        case END_SUBSHELL:
        {
            //close off the subshell and insert it into the sequence
            prev_cmd = top_cmd;
            if (top_seq_cmd)
            {
                top_seq_cmd->u.command[1] = top_cmd; //pt to the top of the subshell tree
                top_cmd = NULL;
            }
            depth--;
            break;
        }

        case INPUT:
        {
            if (!prev_cmd || i == t_list->length - 1 || !last_word_parsed) syntax_error(__LINE__);
            TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;

            if (prev_token_type != WORD) //only valid syntax
                syntax_error(__LINE__);
            //need to get the input first, and then check for output.
            //also, we want to trim the whitespace from the word (because eggert does)
            
            i++;
            TOKEN_TYPE input_type = t_list->tok[i].type;
            if (input_type != WORD || last_word_parsed->input) syntax_error(__LINE__);
            else
            {
                //a little hackish; trim the leading / trailing spaces
                last_word_parsed->input = trim_whitespace(t_list->tok[i].token_str, false);

            }
            if (t_list->tok[i+1].type == OUTPUT) //we have an output as well
            {
                i++; //move to the output token
                i++; //move to the output word
                TOKEN_TYPE output_type = t_list->tok[i].type;
                if (output_type != WORD || last_word_parsed->output) syntax_error(__LINE__);
                else
                    last_word_parsed->output = trim_whitespace(t_list->tok[i].token_str, false);
            }
            //else, just continue. allowed to specify an input and no output.
            last_word_parsed = NULL; //reset
            break;
        }
        case OUTPUT:
        {
            if (!prev_cmd || i == t_list->length - 1) syntax_error(__LINE__);
            
            TOKEN_TYPE prev_token_type = t_list->tok[i-1].type;
            
            if (prev_token_type != WORD || prev_cmd->type != SIMPLE_COMMAND)
                syntax_error(__LINE__);
            //need to traverse to "bottom right" of the tree
            //we want to trim the whitespace from the word (because eggert does)
            *last_word_parsed->u.word = trim_whitespace(*last_word_parsed->u.word, true);
            i++; //move to the output word
            TOKEN_TYPE output_type = t_list->tok[i].type;
            if (output_type != WORD || last_word_parsed->output) syntax_error(__LINE__);
            else
                last_word_parsed->output = trim_whitespace(t_list->tok[i].token_str, false);
            last_word_parsed = NULL;
            break;
        }

        } //end switch
    }//end for
    struct command_stream temp;
    command_stream_t r;
    r = checked_malloc(sizeof(struct command_stream));
    *r = temp; //init to blank.
    if (depth != 0) syntax_error(__LINE__); //indicates not all parenthesis closed
    if (top_seq_cmd)
        r->c = top_seq_cmd;
    else if (top_cmd->type != SIMPLE_COMMAND && top_cmd->u.command[1] == NULL)
        syntax_error(__LINE__);//gotta check if both sides full
    else
        r->c = top_cmd;
    return r;
}

command_t
read_command_stream (command_stream_t s)
{
    command_t the_command = s->c;
    if (!the_command) return NULL;
    switch (the_command->type)
    {
        case SIMPLE_COMMAND:
        case AND_COMMAND:
        case OR_COMMAND:
        case PIPE_COMMAND:
        {
            if (the_command->status != 2) //2 means do not visit anymore
            {
                the_command->status = 2;
                return the_command;
            }
            else return NULL; //already visited this leaf
        }
        case SUBSHELL_COMMAND: //TODO: implement
        {
            if (the_command->status != 2)
            {
                the_command->status = 2;
                return the_command;
            }
            else return NULL;
        }

        case SEQUENCE_COMMAND:
        {
            command_stream_t substream = checked_malloc(sizeof(struct command_stream));
            if (the_command->status == 0)
            {
                substream->c = the_command->u.command[0];
                command_t retval = read_command_stream(substream);
                if (!retval) 
                    the_command->status = 1; 
                else
                    return retval;
            }
            if (the_command->status == 1)
            {
                substream->c = the_command->u.command[1];
                command_t retval = read_command_stream(substream);
                if (!retval)
                    the_command->status = 2;
                else
                    return retval;
            }
            //status == 2
            return NULL;
        }

        default: return NULL; //should never get here but ya never knokw.
    }
    return 0; 
}
