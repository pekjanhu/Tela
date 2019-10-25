/*
/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1996 Pekka Janhunen
 */

/*
   This is a dummy replacement of the GNU readline library.
   Use this file if you can't get GNU readline to work on your machine.
   Traditionally, HP, IBM and Cray have been problematic.
   Only the function 'readline' is implemented in this file, other
   functions have empty bodies.

   The external variable rl_instream points to the string "noreadline".
   This is used to signal Tela (std.ct:UsingReadline()) that readline
   is not there.
   
   Installation (with Tela):
   - Compile with cc -c noreadline.c (or gcc if you are using GNU).
     You will not need any options.
   - Move noreadline.o to your object file directory, if any
   - Edit makeinc, put noreadline.o in place of READLINELIB
   - Make
*/

#include <stdio.h>
#include <string.h>

#define MAXLINE 1024		/* maximum input line length */


/* **************************************************** */
/*                                                      */
/*       Typedef's from keymaps.h                       */
/*                                                      */
/* **************************************************** */

typedef int Function ();
typedef void VFunction ();
typedef char *CPFunction ();
typedef char **CPPFunction ();

/* A keymap contains one entry for each key in the ASCII set.
   Each entry consists of a type and a pointer.
   POINTER is the address of a function to run, or the
   address of a keymap to indirect through.
   TYPE says which kind of thing POINTER is. */
typedef struct _keymap_entry {
  char type;
  Function *function;
} KEYMAP_ENTRY;

/* This must be large enough to hold bindings for all of the characters
   in a desired character set (e.g, 128 for ASCII, 256 for ISO Latin-x,
   and so on). */
#define KEYMAP_SIZE 256

/* I wanted to make the above structure contain a union of:
   union { Function *function; struct _keymap_entry *keymap; } value;
   but this made it impossible for me to create a static array.
   Maybe I need C lessons. */

typedef KEYMAP_ENTRY KEYMAP_ENTRY_ARRAY[KEYMAP_SIZE];
typedef KEYMAP_ENTRY *Keymap;


/* **************************************************************** */
/*                                                                  */
/*          Well Published Variables                                */
/*                                                                  */
/* **************************************************************** */

/* The name of the calling program.  You should initialize this to
   whatever was in argv[0].  It is used when parsing conditionals. */
char *rl_readline_name;

/* The line buffer that is in use. */
char *rl_line_buffer;

/* The location of point, and end. */
int rl_point, rl_end;

/* The name of the terminal to use. */
char *rl_terminal_name;

/* The input and output streams. */
FILE *rl_instream=(FILE*)"noreadline", *rl_outstream;

/* The basic list of characters that signal a break between words for the
   completer routine.  The initial contents of this variable is what
   breaks words in the shell, i.e. "n\"\\'`@$>". */
char *rl_basic_word_break_characters;

/* The list of characters that signal a break between words for
   rl_complete_internal.  The default list is the contents of
   rl_basic_word_break_characters.  */
char *rl_completer_word_break_characters;

/* List of characters which can be used to quote a substring of the line.
   Completion occurs on the entire substring, and within the substring   
   rl_completer_word_break_characters are treated as any other character,
   unless they also appear within this list. */
char *rl_completer_quote_characters;

/* List of characters that are word break characters, but should be left
   in TEXT when it is passed to the completion function.  The shell uses
   this to help determine what kind of completing to do. */
char *rl_special_prefixes;

/* Pointer to the generator function for completion_matches ().
   NULL means to use filename_entry_function (), the default filename
   completer. */
Function *rl_completion_entry_function;

/* If rl_ignore_some_completions_function is non-NULL it is the address
   of a function to call after all of the possible matches have been
   generated, but before the actual completion is done to the input line.
   The function is called with one argument; a NULL terminated array
   of (char *).  If your function removes any of the elements, they
   must be free()'ed. */
Function *rl_ignore_some_completions_function;

/* Pointer to alternative function to create matches.
   Function is called with TEXT, START, and END.
   START and END are indices in RL_LINE_BUFFER saying what the boundaries
   of TEXT are.
   If this function exists and returns NULL then call the value of
   rl_completion_entry_function to try to match, otherwise use the
   array of strings returned. */
CPPFunction *rl_attempted_completion_function;

/* If non-zero, then this is the address of a function to call just
   before readline_internal () prints the first prompt. */
Function *rl_startup_hook;

/* If non-zero, then this is the address of a function to call when
   completing on a directory name.  The function is called with
   the address of a string (the current directory name) as an arg. */
Function *rl_directory_completion_hook;

/* Backwards compatibility with previous versions of readline. */
#define rl_symbolic_link_hook rl_directory_completion_hook

/* The address of a function to call periodically while Readline is
   awaiting character input, or NULL, for no event handling. */
Function *rl_event_hook;

/* Non-zero means that modified history lines are preceded
   with an asterisk. */
int rl_show_star;

/* Non-zero means to suppress normal filename completion after the
   user-specified completion function has been called. */
int rl_attempted_completion_over;

/* **************************************************************** */
/*                                                                  */
/*          Well Published Functions                                */
/*                                                                  */
/* **************************************************************** */

/* Read a line of input.  Prompt with PROMPT.  A NULL PROMPT means none. */
char *readline (prompt)
char *prompt;
{
	int L;
	char s[MAXLINE+1];
	if (prompt) {fputs(prompt,stdout); fflush(stdout);}
	fgets(s,MAXLINE,stdin);
	if (feof(stdin)) return 0;
	L = strlen(s);
	if (s[L-1] == '\n') s[L-1] = '\0';
	return strdup(s);
}

/* These functions are from complete.c. */
/* Return an array of strings which are the result of repeatadly calling
   FUNC with TEXT. */
char **completion_matches () {}
char *username_completion_function () {}
char *filename_completion_function () {}

/* These functions are from bind.c. */
/* rl_add_defun (char *name, Function *function, int key)
   Add NAME to the list of named functions.  Make FUNCTION
   be the function that gets called.
   If KEY is not -1, then bind it. */
int rl_add_defun () {}
int rl_bind_key () {} int rl_bind_key_in_map () {}
int rl_unbind_key () {} int rl_unbind_key_in_map () {}
int rl_set_key () {}
int rl_macro_bind () {} int rl_generic_bind () {} int rl_variable_bind () {}
int rl_translate_keyseq () {}
Function *rl_named_function () {} Function *rl_function_of_keyseq () {}
int rl_parse_and_bind () {}
Keymap rl_get_keymap () {} Keymap rl_get_keymap_by_name () {}
void rl_set_keymap () {}
char **rl_invoking_keyseqs () {} char **rl_invoking_keyseqs_in_map () {}
void rl_function_dumper () {}
int rl_read_init_file () {}

/* Functions in funmap.c */
void rl_list_funmap_names () {}
void rl_initialize_funmap () {}

/* Functions in display.c */
void rl_redisplay () {}
int rl_message () {} int rl_clear_message () {}
int rl_reset_line_state () {}
int rl_character_len () {}
int rl_show_char () {}
int crlf () {} int rl_on_new_line () {}
int rl_forced_update_display () {}





/* History -- the names of functions that you can call in history. */

/* The structure used to store a history entry. */
typedef struct {
  char *line;
  char *data;
} HIST_ENTRY;

/* A structure used to pass the current state of the history stuff around. */
typedef struct _hist_entry {
  HIST_ENTRY **entries;		/* Pointer to the entries themselves. */
  int offset;			/* The location pointer within this array. */
  int length;			/* Number of elements within this array. */
  int size;			/* Number of slots allocated to this array. */
} HISTORY_STATE;

/* For convenience only.  You set this when interpreting history commands.
   It is the logical offset of the first history element. */
int history_base;

/* Begin a session in which the history functions might be used.  This
   just initializes the interactive variables. */
void using_history () {}

/* Return the current HISTORY_STATE of the history. */
HISTORY_STATE *history_get_history_state () {}

/* Set the state of the current history array to STATE. */
void history_set_history_state () {}

/* Place STRING at the end of the history list.
   The associated data field (if any) is set to NULL. */
void add_history () {}

/* Returns the number which says what history element we are now
   looking at.  */
int where_history () {}
  
/* Set the position in the history list to POS. */
int history_set_pos () {}

/* Search for STRING in the history list, starting at POS, an
   absolute index into the list.  DIR, if negative, says to search
   backwards from POS, else forwards.
   Returns the absolute index of the history element where STRING
   was found, or -1 otherwise. */
int history_search_pos () {}

/* A reasonably useless function, only here for completeness.  WHICH
   is the magic number that tells us which element to delete.  The
   elements are numbered from 0. */
HIST_ENTRY *remove_history () {}

/* Stifle the history list, remembering only MAX number of entries. */
void stifle_history () {}

/* Stop stifling the history.  This returns the previous amount the
   history was stifled by.  The value is positive if the history was
   stifled, negative if it wasn't. */
int unstifle_history () {}

/* Add the contents of FILENAME to the history list, a line at a time.
   If FILENAME is NULL, then read from ~/.history.  Returns 0 if
   successful, or errno if not. */
int read_history () {}

/* Read a range of lines from FILENAME, adding them to the history list.
   Start reading at the FROM'th line and end at the TO'th.  If FROM
   is zero, start at the beginning.  If TO is less than FROM, read
   until the end of the file.  If FILENAME is NULL, then read from
   ~/.history.  Returns 0 if successful, or errno if not. */
int read_history_range () {}

/* Append the current history to FILENAME.  If FILENAME is NULL,
   then append the history list to ~/.history.  Values returned
   are as in read_history ().  */
int write_history () {}

/* Append NELEMENT entries to FILENAME.  The entries appended are from
   the end of the list minus NELEMENTs up to the end of the list. */
int append_history () {}

/* Make the history entry at WHICH have LINE and DATA.  This returns
   the old entry so you can dispose of the data.  In the case of an
   invalid WHICH, a NULL pointer is returned. */
HIST_ENTRY *replace_history_entry () {}

/* Return the history entry at the current position, as determined by
   history_offset.  If there is no entry there, return a NULL pointer. */
HIST_ENTRY *current_history () {}

/* Back up history_offset to the previous history entry, and return
   a pointer to that entry.  If there is no previous entry, return
   a NULL pointer. */
HIST_ENTRY *previous_history () {}

/* Move history_offset forward to the next item in the input_history,
   and return the a pointer to that entry.  If there is no next entry,
   return a NULL pointer. */
HIST_ENTRY *next_history () {}

/* Return a NULL terminated array of HIST_ENTRY which is the current input
   history.  Element 0 of this list is the beginning of time.  If there
   is no history, return NULL. */
HIST_ENTRY **history_list () {}

/* Search the history for STRING, starting at history_offset.
   If DIRECTION < 0, then the search is through previous entries,
   else through subsequent.  If the string is found, then
   current_history () is the history entry, and the value of this function
   is the offset in the line of that history entry that the string was
   found in.  Otherwise, nothing is changed, and a -1 is returned. */
int history_search () {}

/* Expand the string STRING, placing the result into OUTPUT, a pointer
   to a string.  Returns:

   0) If no expansions took place (or, if the only change in
      the text was the de-slashifying of the history expansion
      character)
   1) If expansions did take place
  -1) If there was an error in expansion.

  If an error ocurred in expansion, then OUTPUT contains a descriptive
  error message. */
int history_expand () {}

/* Return an array of tokens, much as the shell might.  The tokens are
   parsed out of STRING. */
char **history_tokenize () {}

/* Extract a string segment consisting of the FIRST through LAST
   arguments present in STRING.  Arguments are broken up as in
   the shell. */
char *history_arg_extract () {}

/* Return the number of bytes that the primary history entries are using.
   This just adds up the lengths of the_history->lines. */
int history_total_bytes () {}

/* Exported history variables. */
int history_stifled;
int history_length;
int max_input_history;
char history_expansion_char;
char history_subst_char;
char history_comment_char;
char *history_no_expand_chars;
int history_base;
