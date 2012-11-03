#include <stdio.h>
#include "../src/liboptbot.h"

int main(int argc, const char** argv) {
 struct cli_arg_list* arg_list = init_cli_arg_list();

  add_arg(arg_list, 'v', "verbose", "Should this program use "
    "verbose output?", false);
  add_arg(arg_list, 'n', "ninja", "Be a ninja.", false);
  add_arg(arg_list, 'h', "help", "I HAVE NO IDEA WHAT I'M DOING.", false);
  add_arg(arg_list, 'p', "pirate", "Be a pirate.", false);
  add_arg(arg_list, 'N', "name", "Your name", true);

  if(!parse_command_line(arg_list, argc, argv)) {
    fputs(arg_list->message, stderr);
    return 1;
  }

  if(little_opt_arg(arg_list, 'h')->times_set)
    print_help(arg_list);

  destroy_cli_arg_list(arg_list);

  return 0;
}
