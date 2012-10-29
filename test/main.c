#include <check.h>
#include <string.h>
#include "../src/liboptbot.h"

START_TEST(test_little_opt) {
  const char* args[] = {"-v"};
  struct cli_arg_list* arg_list = init_cli_arg_list();
  struct cli_arg* arg;
  add_arg(arg_list, 'v', "verbose", "Whatever", false);

  fail_unless(parse_command_line(arg_list, 1, (const char**)args),
    "Command line could not be parsed!");
  printf("%s", arg_list->message);
  arg = little_opt_arg(arg_list, 'v');
  fail_if(arg == NULL, "-v not found");
  fail_unless(arg->set, "-v not set!");
}
END_TEST

START_TEST(test_big_opt) {
  const char* args[] = {"--verbose"};
  struct cli_arg_list* arg_list = init_cli_arg_list();
  struct cli_arg* arg;
  add_arg(arg_list, 'v', "verbose", "Whatever", false);

  fail_unless(parse_command_line(arg_list, 1, (const char**)args),
    "Command line could not be parsed!");
  arg = big_opt_arg(arg_list, "verbose");
  fail_if(arg == NULL, "--verbose not found");
  fail_unless(arg->set, "--verbose not set");
}
END_TEST

START_TEST(test_chained_little_opts) {
  const char* args[] = {"-vdffilename.txt"};
  struct cli_arg_list* arg_list = init_cli_arg_list();
  struct cli_arg* arg;
  add_arg(arg_list, 'v', "verbose", "Whatever", false);
  add_arg(arg_list, 'd', "debug", "Blah", false);
  add_arg(arg_list, 'f', "file", "...", true);

  fail_unless(parse_command_line(arg_list, 1, (const char**)args),
    "Command line could not be parsed!");

  arg = little_opt_arg(arg_list, 'v');
  fail_if(arg == NULL, "-v not found");
  fail_unless(arg->set, "-v not set");

  arg = little_opt_arg(arg_list, 'd');
  fail_if(arg == NULL, "-d not found");
  fail_unless(arg->set, "-d not set");

  arg = little_opt_arg(arg_list, 'f');
  fail_if(arg == NULL, "-f not found");
  fail_unless(arg->set, "-f not set");
  fail_unless(strcmp(arg->value, "filename.txt") == 0);
}
END_TEST

START_TEST(failure_on_unkown_opt) {
  const char* args[] = {"--unknown"};
  struct cli_arg_list* arg_list = init_cli_arg_list();

  add_arg(arg_list, 'v', "verbose", "Whatever", false);
  fail_if(parse_command_line(arg_list, 1, (const char**)args),
    "Parsed command line successfully");
  fail_unless(arg_list->error == invalid_opt,
    "Arg list error was not set properly");
  fail_unless(strlen(arg_list->message) > 0,
    "Arg list error message was not set properly");
}
END_TEST

START_TEST(value_assignments) {
  const char* args[] = {"--arg_a", "value1", "-b", "value_2", "-d", "no_value"};
  struct cli_arg_list* arg_list = init_cli_arg_list();

  add_arg(arg_list, 'a', "arg_a", "...", true);
  add_arg(arg_list, 'b', "arg_b", "...", true);
  add_arg(arg_list, 'd', "arg_d", "...", false);

  fail_unless(parse_command_line(arg_list, 6, (const char**)args),
    "Could not parse command line");

  fail_unless(strcmp(little_opt_arg(arg_list, 'a')->value, "value1") == 0,
    "--arg_a was not assigned the proper value");
}
END_TEST

Suite* optbot_suite(void) {
  Suite *suite = suite_create("liboptbot");

  TCase *main_case = tcase_create("Main");
  tcase_add_test(main_case, test_little_opt);
  tcase_add_test(main_case, test_big_opt);
  tcase_add_test(main_case, test_chained_little_opts);
  tcase_add_test(main_case, value_assignments);
  tcase_add_test(main_case, failure_on_unkown_opt);
  suite_add_tcase(suite, main_case);
  return suite;
}

int main(void) {
  int number_failed;
  Suite *s = optbot_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return(number_failed == 0) ? 0 : 1 ;

  return 0;
}
