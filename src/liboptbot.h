#ifndef __ARG_LIST_INC__
#define __ARG_LIST_INC__

#include <stdbool.h>
#include <stdio.h>

// Some basic error handling tools
#ifdef DEBUG
#define errmsg(m, ...) fprintf(stderr, "%s:%d: " m "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define errmsg(m, ...)
#endif
#define checkmem(obj) \
  if(!obj) { errmsg("You're out of memory hoss!"); goto error; }

#define OPTBOT_ERROR_MSG_SIZE 512

/* For simplified error reporting.  Requires an error goto.
 *
 * @note message must be no longer than OPTBOT_ERROR_MSG_SIZE chars
 */
#define error_check(list, condition, error_type, msg, ...) \
  if(! condition) { \
    list->error = error_type; \
    snprintf(list->message, OPTBOT_ERROR_MSG_SIZE - 1, \
      msg, ##__VA_ARGS__); \
    goto error;\
  }

#define ARRAY_INIT_SIZE 10

/*! A command line argument */
struct cli_arg {
  char* description; /* A brief description of this argument */
  char* big; /* The long option that this argument takes, sans dashes */
  char little; /* The short option that this argument takes */
  bool takes_value; /* Does this argument take a value? */
  bool allow_multiple; /* Can this argument be set multiple times? */
  int times_set; /* The number of times this option was set */
  int values_length; /* The number of values held in values */
  int values_size; /* The number of values allocated in values */
  char** values; /* The value that have been assigned to this argument */
};

/* User error types */
enum cli_arg_error {
  none, /* No error */
  set_twice, /* An attempt was made to set the same option twice */
  invalid_opt, /* An invalid option was given */
  value_required, /* An  which requires a value was not given one */
  empty_list, /* An empty argument list was given where it is not allowed */
  out_of_memory, /* Internal object initialization failed */
};

struct cli_arg* init_cli_arg(void);
void destroy_cli_arg(struct cli_arg*);
void print_cli_arg(struct cli_arg*);

struct cli_arg_list_node {
  struct cli_arg* arg;
  struct cli_arg_list_node* next;
};

struct cli_arg_list {
  int argc; /* The number of positional params left over after parsing */
  char** argv; /* The positional params left over after parsing */
  int argv_size;
  struct cli_arg_list_node* head;
  enum cli_arg_error error; /* The last error that occured */
  bool devour_flag; /* enables the -- option */
  char* message; /* An error string for the last error that occured */
};

enum arg_type {little, big};

struct cli_arg_list* init_cli_arg_list(void);
void destroy_cli_arg_list(struct cli_arg_list*);
void print_cli_arg_list(struct cli_arg_list*);

struct cli_arg* big_opt_arg(struct cli_arg_list*, const char*);
struct cli_arg* little_opt_arg(const struct cli_arg_list*, char);
bool add_arg(struct cli_arg_list*, char, const char*, const char*, bool);
bool parse_command_line(struct cli_arg_list*, int, const char**);

void print_help(struct cli_arg_list*);
void write_help(struct cli_arg_list*, FILE*);

#endif
