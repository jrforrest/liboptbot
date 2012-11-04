#include <check.h>
#include <string.h>
#include <stdlib.h>
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
  fail_unless(arg->times_set > 0, "-v not set!");

  destroy_cli_arg_list(arg_list);
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
  fail_unless(arg->times_set > 0, "--verbose not set");

  destroy_cli_arg_list(arg_list);
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
  fail_unless(arg->times_set > 0, "-v not set");

  arg = little_opt_arg(arg_list, 'd');
  fail_if(arg == NULL, "-d not found");
  fail_unless(arg->times_set > 0, "-d not set");

  arg = little_opt_arg(arg_list, 'f');
  fail_if(arg == NULL, "-f not found");
  fail_unless(arg->times_set > 0, "-f not set");
  fail_unless(strcmp(arg->values[0], "filename.txt") == 0);

  destroy_cli_arg_list(arg_list);
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

  destroy_cli_arg_list(arg_list);
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

  fail_unless(strcmp(little_opt_arg(arg_list, 'a')->values[0], "value1") == 0,
    "--arg_a was not assigned the proper value");
  destroy_cli_arg_list(arg_list);
}
END_TEST

START_TEST(multiple_args_with_values) {
  const char* args[] = {"--arg", "one", "-a", "two", "-a",
    "three", "--arg", "four", "-afive"};
  const char* expected_args[] = {"one", "two", "three", "four", "five"};
  struct cli_arg* arg;
  int i;

  struct cli_arg_list* arg_list = init_cli_arg_list();

  add_arg(arg_list, 'a', "arg", "A generic argument", true);
  fail_unless(parse_command_line(arg_list, 9, (const char**)args),
    "Could not parse command line");

  arg = little_opt_arg(arg_list, 'a');

  for(i = 0; i < 5; i++) {
    fail_unless(strcmp(arg->values[i], expected_args[i]) == 0,
      "Arg values do not match: %d: %s != %s",
      i, arg->values[i], expected_args[i]
    );
  }

  fail_unless(arg->values_length == 5,
    "arg->values_length is incorrect: expected 5, got %d", arg->values_length);
  destroy_cli_arg_list(arg_list);
}
END_TEST

START_TEST(lots_of_args_with_values) {
  char* args[100];
  int i;
  struct cli_arg_list* arg_list = init_cli_arg_list();
  struct cli_arg* arg;

  for(i = 0; i < 100; i++) {
    args[i] = malloc(sizeof(char) * 10);
    sprintf(args[i], "-dtestp%d", i);
  }

  add_arg(arg_list, 'd', "data", "...", true);
  fail_unless(parse_command_line(arg_list, 100, (const char**)args),
    "Could not parse command line");
  arg = little_opt_arg(arg_list, 'd');

  for(i = 0; i < 100; i++) {
    fail_unless(strcmp(arg->values[i], args[i] + 2) == 0,
      "Arg values do not match: %d: %s != %s",
      i, arg->values[i], args[i] + 2
    );
  }

  fail_unless(arg->values_length == 100,
    "arg->values_length is incorrect: expected, got %d", arg->values_length);
  destroy_cli_arg_list(arg_list);
}
END_TEST

START_TEST(leftover_argv) {
  char* argv[] = {"-d", "one", "-f", "garbage", "two"};
  struct cli_arg_list* arg_list = init_cli_arg_list();
  int i =0;

  add_arg(arg_list, 'd', "debug", "...", false);
  add_arg(arg_list, 'f', "file", "...", true);

  parse_command_line(arg_list, 5, (const char**)argv);

  for(i = 0; i< arg_list->argc; i++)
    printf("%d: %s\n", i, arg_list->argv[i]);
  printf("%s\n", little_opt_arg(arg_list, 'f')->values[0]);

  fail_unless(strcmp(arg_list->argv[0], "one") == 0);
  fail_unless(strcmp(arg_list->argv[1], "two") == 0);
  fail_unless(strcmp(little_opt_arg(arg_list, 'f')->values[0], "garbage") == 0);
  fail_unless(arg_list->argc == 2, "argc is %d, expected 2", arg_list->argc);
  destroy_cli_arg_list(arg_list);
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
  tcase_add_test(main_case, multiple_args_with_values);
  tcase_add_test(main_case, lots_of_args_with_values);
  tcase_add_test(main_case, leftover_argv);
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

  return(number_failed > 255 ? 255 : number_failed);

  return 0;
}
