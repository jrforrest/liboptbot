#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "liboptbot.h"

/*! Initializer for CLI arg
 *
 *  @return The new CLI arg, or NULL if it could not be initialized
 */
struct cli_arg* init_cli_arg(void) {
  struct cli_arg* cli_arg;

  cli_arg = malloc(sizeof(struct cli_arg));
  checkmem(cli_arg);

  cli_arg->values = NULL;
  cli_arg->times_set = 0;
  cli_arg->allow_multiple = false;
  cli_arg->description = NULL;
  cli_arg->big = NULL;
  cli_arg->little = '\0';
  cli_arg->takes_value = false;
  cli_arg->values_size = 0;
  cli_arg->values_length = 0;


  return cli_arg;

  error:
    return NULL;
}

/*! Prints a command line option, for debugging and testing purposes
 *
 *  @param [cli_arg] The argument to print
 */
void print_cli_arg(struct cli_arg* cli_arg) {
  int i;

  printf("Description: %s\n", cli_arg->description);
  printf("Little: %c\n", cli_arg->little);
  printf("Big: %s\n", cli_arg->big);
  printf("Takes Value: %d\n", cli_arg->takes_value);
  printf("Times Set: %d\n", cli_arg->times_set);
  printf("Values\n");
  for(i = 0; i < cli_arg->values_length; i++)
    printf("  %d: %s\n", i, cli_arg->values[i]);
}

/* Destructor for args
 *
 * @param [cli_arg] The argument to destroy
 */
void destroy_cli_arg(struct cli_arg* cli_arg) {
  int i;
  free(cli_arg->description);
  free(cli_arg->big);
  for(i = 0; i < cli_arg->values_length; i++)
    free(cli_arg->values[i]);
  free(cli_arg->values);
  free(cli_arg);
}

/*! Initializer for argument lists
 *
 *  This creates an argument list which can later be populated with
 *  the arguments for your program.
 *
 *  @return The initialized list, or NULL if it could not be created
 */
struct cli_arg_list* init_cli_arg_list(void) {
  struct cli_arg_list* list;
  list = malloc(sizeof(struct cli_arg_list));
  list->head = NULL;
  list->error = none;
  list->argc = 0;
  list->argv = NULL;
  list->argv_size = 0;
  list->message = (char*)malloc(sizeof(char) * OPTBOT_ERROR_MSG_SIZE);
  return list;
}

/*! Pushes a value onto an array of strings
 *
 *  @param [in,out] [len] The length of the array
 *  @param [in,out] [size] The size of the array
 *  @param [in,out] [ary] The address of the array of strings
 *  @param [value] The value to be pushed onto the array
 *  @return Operation successful?
 */
static bool str_array_push(
  int* len, int* size, char*** ary, const char* value)
{
  if(*len == *size) {
    *size += ARRAY_INIT_SIZE;
    *ary = (char**)realloc(*ary, sizeof(char*) * ((*size) * ARRAY_INIT_SIZE));
    checkmem(*ary)

    /* Initialize to zero so I can get some sweet segfaults later */
    memset(*ary + *len, 0, *size - *len);
  }

  (*ary)[*len] = strdup(value);
  checkmem((*ary)[*len]);
  (*len)++;

  return true;

  error:
    return false;
}

/*! Adds the given value to the values list of the given arg
 *
 *  @param [arg] The argument to which the value should be added.  This string
 *    is copied for storage with the argument.
 *  @param [value] The value to add to the given argument
 *  @return Was the operation successful?
 */
static bool add_to_values(struct cli_arg* arg, const char* value) {
  return str_array_push(
    &arg->values_length, &arg->values_size, &arg->values, value);
}

/*! Adds the given value to the leftover argv for the given list
 *
 * @param [list] The argument list to operate on
 * @param [value] The value to push onto the lists argv
 * @return Operation successful?
 */
static bool add_to_argv(struct cli_arg_list* list, const char* value) {
  return str_array_push( &list->argc, &list->argv_size, &list->argv, value);
}


/*! Gets the last node from an argument list
 *
 *  @param [list] The list to search
 *  @return The last node in the list, or NULL if it's empty
 */
static struct cli_arg_list_node* cli_arg_list_last(
  const struct cli_arg_list* list)
{
  struct cli_arg_list_node* list_head = list->head;
  if(!list_head) return NULL;

  while(list_head->next != NULL)
    list_head = list_head->next;
  return list_head;
}

/*! Destructor for argument list node
 *
 *  @note This shouldn't be called on its own, lest those lower in the list
 *    be orphaned.
 *  @param [node] The node to be destroyed
 */
static void destroy_cli_arg_list_node(struct cli_arg_list_node* node) {
  if(!node) return;
  destroy_cli_arg(node->arg);
  free(node);
}

/*! Deletes the given node from the given list
 *
 *  This method removes the node from the given list, and safely destroys
 *  it along with it's associated argument.  The list is kept intact.
 *
 *  @param [list] The list to delete from
 *  @param [node] The node to delete from the list
 *  @return True if the node was deleted, false if the node could not be found
 *    in the list
 */
bool cli_arg_list_delete_node(struct cli_arg_list* list,
  struct cli_arg_list_node* node)
{
  struct cli_arg_list_node* head = list->head;
  struct cli_arg_list_node* prev = list->head;

  while(head) {
    if(head == node) {
      if(node == list->head) {
        list->head = node->next;
      } else {
        prev->next = node->next;
      }
      destroy_cli_arg_list_node(head);
      return true;
    } else {
      prev = head;
      head = head->next;
    }
  }

  return false;
}

/*! Destructor for argument lists
 *
 *  @param [list] The list to be destroyed
 */
void destroy_cli_arg_list(struct cli_arg_list* list) {
  int i;

  while(list->head)
    cli_arg_list_delete_node(list, list->head);
  free(list->message);

  for(i = 0; i < list->argc; i++){
    free(list->argv[i]);
  }
  free(list->argv);

  free(list);
}

/*! Initializer for list nodes
 *
 *  @return A pointer to the initialized node, or NULL if it could
 *    not be created.
 */
static struct cli_arg_list_node* init_cli_arg_list_node(void) {
  struct cli_arg_list_node* node;
  node = malloc(sizeof(struct cli_arg_list_node));
  checkmem(node);

  node->arg = NULL;
  node->next = NULL;

  return node;

  error:
    return NULL;
}

/*! Prints all of the arguments in a list
 *
 *  @param list The list to print out
 */
void print_cli_arg_list(struct cli_arg_list* list) {
  struct cli_arg_list_node* list_head = list->head;
  if(!list->head) return;

  while(list_head) {
    print_cli_arg(list_head->arg);
    list_head = list_head->next;
  }
}

/*! Adds an argument to the given list
 *
 *  @return True if the node could be added, false if an error occured
 */
static bool add_cli_arg(struct cli_arg_list* list, struct cli_arg* arg) {
  struct cli_arg_list_node* next = init_cli_arg_list_node();
  struct cli_arg_list_node* last = cli_arg_list_last(list);
  checkmem(next);

  next->arg = arg;
  if(last) {
    last->next = next;
  } else {
    list->head = next;
  }

  return true;

  error:
    return false;
}

/*! Searches an argument list for the argument denoted by opt and type
 *
 *  @param [type] The type of argument that opt contains.  This will be
 *    one of big or little.
 *  @param [list] The list to search
 *  @param [opt] The string denoting the option that should be searched for
 *  @return The argument matching the given parameters, or NULL if none could
 *    be found in
 */
static struct cli_arg* cli_arg_list_find(enum arg_type type,
  const struct cli_arg_list* list, const char* opt)
{
  /* NOTE: opt will be a pointer to a single character, rather
     than a null-terminated string if type is set to little. */
  struct cli_arg_list_node* list_head = list->head;
  if(! list_head) return NULL;
  while(list_head) {
    if(type == big) {
      if(strcmp(list_head->arg->big, opt) == 0) break;
    } else if(type == little) {
      if(list_head->arg->little == *opt) break;
    }
    list_head = list_head->next;
  }
  return list_head ? list_head->arg : NULL;
}

/*! A helper function for getting the argument from a list that uses
 *  the given little_opt
 *
 *  @param [list] The list to search for the little option in
 *  @param [little_opt] The little option to identify the argument by
 *  @return The option matching the given little_opt.  NULL if no
 *    match was found.
 */
struct cli_arg* little_opt_arg(const struct cli_arg_list* list, char little_opt) {
  return cli_arg_list_find(little, list, &little_opt);
}

/*! A helper function for getting the argument from a list that uses the
 *  given big_opt
 *
 *  @param [list] The list to search for the argument
 *  @param [big_opt] The big argument to search for
 *  @return The argument matching the given big_opt.  NULL if no match
 *    was found.
 */
struct cli_arg* big_opt_arg(struct cli_arg_list* list, const char* big_opt) {
  return cli_arg_list_find(big, list, big_opt);
}

/*! Convenience method for adding an argument to the given arg_list
 *
 *  @param [arg_list] The list that the argument should be added to
 *  @param [little] The little option for the new argument
 *  @param [big] The big option for the new argument
 *  @param [description] The description of the argument
 *  @param [takes_value] Does this parameter take a value?
 */
bool add_arg(struct cli_arg_list* arg_list, char little, const char* big,
  const char* description, bool takes_value)
{
  struct cli_arg* cli_arg = init_cli_arg();
  checkmem(cli_arg);

  cli_arg->little = little;

  if(big) {
    cli_arg->big = strdup(big);
    checkmem(cli_arg->big);
  }

  if(description) {
    cli_arg->description = strdup(description);
    checkmem(cli_arg->description)
  }

  cli_arg->takes_value = takes_value;

  /* This should be swapped out for a more descriptive macro if
   add_cli_arg returns false for anything other than memory issues */
  checkmem(add_cli_arg(arg_list, cli_arg));

  return true;

  error:
    arg_list->error = out_of_memory;
    arg_list->message = "Failed to allocate memory.";
    return false;
}

/*! Parses a little option string into the given list
 *
 *  Searches the given list for an argument with an argument with a little
 *  option matching the given opt_str.  If opt_str is longer than one character,
 *  this function either operates recursively treating each succesive character
 *  as a new little option to be assigned, or, when an argument is encountered
 *  that requires a value, treating all remaining characters as a value.  If
 *  no remaining characters are available for an argument that requires a
 *  value, the next parameter is used as a value if it is not null.
 *
 *  @param [list] The list to be searched for a matching argument
 *  @param [opt_str] The option string to match arguments against
 *  @param [next] The value which should be assigned to the last argument
 *    should it require one
 *  @param [out] [ate_next] Was next consumed?
 *  @return True if all of the options in the given opt_str were successfully
 *    set, false otherwise
 */
static bool parse_little(struct cli_arg_list* list,
  const char* opt_str, const char* next, bool* ate_next)
{
  struct cli_arg* arg = little_opt_arg(list, opt_str[0]);
  bool arg_added = false;

  error_check(list, arg, invalid_opt, "%s is not a valid option!", opt_str);

  arg->times_set++;

  if(arg->takes_value && strlen(opt_str) > 1) {
    checkmem(add_to_values(arg, opt_str + 1));
    arg_added = true;
  } else if(arg->takes_value && next) {
    checkmem(add_to_values(arg, next));
    arg_added = true;
    *ate_next = true;
  } else if(strlen(opt_str) > 1) {
    return parse_little(list, opt_str + 1, next, ate_next);
  }

  error_check(list, ! (arg->takes_value &&! arg_added), value_required,
    "-%c requires a value!", arg->little)

  return true;

  error:
    return false;
}

/*! Parses a big option string into the matching parameter object
 *
 *  Searches through the given list for a parameter with a big option that
 *  matches the given opt_str, and attempts to set it.  next is used as
 *  the value for the argument if it is not NULL.
 *
 *  @param [list] The list to search for the given option
 *  @param [opt_str] The option in string form, sans dashes.
 *  @param [next] The value for the given option.  This is ignored if the
 *    parameter does not take a value.  Additionally, the value may be null,
 *    though it will cause an error if the argument expects a value.
 *  @param [out] [ate_next] Was next consumed?
 *  @return True if the option was set.  False otherwise
 */
static bool parse_big(struct cli_arg_list* list,
  const char* opt_str, const char* next, bool* ate_next)
{
  struct cli_arg* arg = big_opt_arg(list, opt_str);

  error_check(list, arg, invalid_opt, "--%s is not a valid option!", opt_str);

  arg->times_set++;

  if(arg->takes_value && next) {
    checkmem(add_to_values(arg, next));
    *ate_next = true;
  }

  error_check(list, ! (arg->takes_value && ! *ate_next), value_required,
    "--%s requires a value!", arg->big);

  return true;

  error:
    return false;
}

/*! Determines whether or not a string is a properly formatted argument
 *
 *  @param [str] The string to check
 *  @return Is the string a properly formed argument?
 */
static bool is_opt(const char* str) {
  int len = strlen(str);
  /* A single char can't be an option */
  if(len < 2) return false;
  /* An option's got to start with a dash */
  if(str[0] != '-') return false;
  /* A long opt has to have more than a single character after the dashes */
  if(str[1] == '-' && len < 4) return false;

  return true;
}

/*! Parses the command line into an arg list
 *
 * @param [in,out] [list] The argument list that will be populated
 *   with
 * @param [argc] The number of string arguments contained in argv
 * @param [argv] An array of command line arguments
 * @return True if the arguments were parsed successfully, false otherwise
 */
bool parse_command_line(struct cli_arg_list* list,
  int argc, const char** argv)
{
  int i;
  const char* next;
  bool ate_next = false;
  bool devour_mode = false;

  for(i = 0; i < argc; i++) {
    if(ate_next) {
      ate_next = false;
      continue;
    }

    if(strcmp(argv[i], "--") == 0 && list->devour_flag) {
      devour_mode = true;
      continue;
    }

    if(! is_opt(argv[i]) || devour_mode) {
      checkmem(add_to_argv(list, argv[i]));
      continue;
    }

    /* Is there a potential value following? */
    next = (i < argc - 1) && (! is_opt(argv[i + 1])) ? argv[i + 1] : NULL;

    if(argv[i][1] == '-') {
      if(! parse_big(list, argv[i] + 2, next, &ate_next)) goto error;
    } else {
      if(! parse_little(list, argv[i] + 1, next, &ate_next)) goto error;
    }
  }

  return true;

  error:
    if(list->error == none) {
      list->error = out_of_memory;
      list->message = "Failed to allocate memory.";
    }
    return false;
}

/*! Prints help to stderr
 *
 *  @param [list] The argument list to print
 */
void print_help(struct cli_arg_list* list) {
  write_help(list, stderr);
}

/*! Writes command line options to the given file with their
 *  descriptions
 *
 *  @param [list] The argument list to print
 *  @param [file] The file that the arguments should be written to.  This
 *    file must be open and writable.
 */
void write_help(struct cli_arg_list* list, FILE* file) {
  struct cli_arg_list_node* next = list->head;
  error_check(list, next, empty_list, "Can't print help for an empty list!");

  fprintf(file, "Options\n");
  while(next) {
    fprintf(file, "  -%c, --%s: %s\n",
      next->arg->little, next->arg->big, next->arg->description);
    next=next->next;
  }

  error:
    return;
}
