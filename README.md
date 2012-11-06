LIBOPTBOT
========

A fairly advanced option parser for C

Features
--------

* Parses long and short options
* Collects values given to options
* Makes use of arguments not identified as options or arguments to options easy
* Allows for a -- flag, which escapes all arguments proceeding it
* Automatic help generation
* Allows multiple uses of options
* Allows compacting of single character options
* Thread safety possible via thread-local objects

Warning
-------

Minor version changes may break the interface until this hits v 1.  C libraries
have a long and illustrious history of terrible function names, but I think I
over-did it here, so I'll probably fix some of those.

License
-------

LGPL for now.  I might release future versions under a different license as I
learn more about the various types.

Example
-------

See /examples for a more distilled one.  Here's the long version.  (Watch for
typos, I haven't compiled this.)

```C
#include <liboptbot.h>
/* Other includes go here... */

int main(int argc, char** argv) {
  int i;
  /* Initialize our arg list */
  struct cli_arg_list* arg_list = init_cli_arg_list();

  /* cli_arg represents a single argument created with add_arg.  We'll use this
     later to avoid looking arguments up twice */
  struct cli_arg* arg_temp = NULL;

  /* This is the only memory check you should really need to do.  optbot should
     use its internal errors for everything else, unless you're doing something
     weird to initialize structs that are usually only used internally. */
  if(!arg_list) return EXIT_FAILURE;

  /* Specify the arguments we expect.  That boolean at the end tells
     optbot whether or not we expect a value for that option.  We give
     descriptions for each argument so we can print a help page later. */
  add_arg(arg_list, 'v', "verbose", "Enable verbose output?", false);
  add_arg(arg_list, 'd', "debug", "Enable debug output?", false);
  add_arg(arg_list, 'f', "file", "File to output to", true);
  add_arg(arg_list, 'h', "help", "Print help and exit", false);
  add_arg(arg_list, 'r', "record", "Record some data", true);

  /* This allows us to pass -- on the command line, which causes optbot to treat
     everything following it as values, rather than attempting to parse the
     arguments as parameters. */
  arg_list->devour_flag = true;

  retry:
  /* parse_command_line returns a boolean denoting success state */
  if(! parse_command_line(arg_list, argc, argv)) {
    /* There are several error types specified in liboptbot.h
       All errors should set arg_list->message, but we can give the user a
       custom message (or take other action) by doing something like this: */
    if(arg_list->error == out_of_memory) {
      fprintf(stderr, "Memory allocation failed! Attempting to make more.\n");
      /* invoke_ram_fairy() is a dummy method.  I was going to include it in
         this release but was too busy playing nethack to finish it. */
      if(! invoke_ram_fairy()) return EXIT_FAILURE;
      goto retry;
    } else {
      /* This message is not printed by liboptbot.
         You're responsible for doing so. */
      printf(arg_list->message);
    }
  }

  /* We can lookup options from the list by big option like so:
     Caveats:
       1.  Check output of this function if you see a segfault.  It returns
           NULL if no opt exists with that name and I've let this one get
           me more than once.
       2.  This runs in linear time and uses string comparisons for lookup.
           Don't abuse it or you know, bad linear time lookup type things
           will happen.   If anyone wants to use this for rsync you'll need
           to re-implement it with a hashtable, for obvious reasons. */
  if(big_opt_arg(arg_list, "verbose")->times_set > 0) {
    /* enable verbose output... */
    if(big_opt_arg(arg_list, "verbose")->times_set > 1)
      /* enable really verbose output... */
  }

  /* Hey check this out, we can lookup by little arg too!  The comparison is
     a bit faster, but this still runs in linear time. */
  if(little_opt_arg(arg_list, 'h')->times_set > 0) {
    /* This is kind of cool, we get to avoid writing that annoying help output
       with a massive string this way.  This is why those descriptions you
       pass to add_arg() are important, by the way.  Leave them blank if
       you're not going to use this I suppose, but they at least provide
       some in-source documentation.  Oh, this goes to stderr by the way. */
    print_help(arg_list);

    /* Oh, you can also do write_help(arg_list, stdout) if
       you're the kind of weirdo that doesn't write to stderr.  If you're
       a total loon you can give any arbitrary file in place of stdout. */
  }

  if(little_opt_arg(arg_list, 'd')->times_set > 0)
    /* This is handy for debugging options passed to your program, but
       may be a little overly-verbose.  There's also print_cli_arg if you
       only need to look at one argument.  This goes to stderr by the way. */
    print_cli_arg_list(arg_list);

  if(little_opt_arg(arg_list, 'f')->times_set > 0)
    /* Because we specified -f as requiring a parameter, we can be confident
       the value exists.  Well, confident insofar as your faith in the bug-free
       status of my code, which probably shouldn't be that great considering
       my crippling alcoholism. */
    set_output_file(arg->values[0]);

  /* Here's what accessing multiple parameters for an argument will look like.*/
  arg = little_opt_arg(arg_list, 'r');
  for(i = 0; i < arg->values_length; i++) {
    record_data(arg->values[i]);
  }

  /* We can also get whatever arguments were passed but not parsed or
     assigned as values to params */
  for(i = 0; i < arg_list->argc; i++)
    printf("Param %d: %s\n", i + 1; arg_list->argv[i]);

  /* Do our other main() things... */

  /* This operates on everything allocated internally for the arg_list */
  destroy_cli_arg_list(arg_list);

  /* arg->value should segfault now, by the way. */

  return EXIT_SUCCESS;
}

Alright, that was fun.  So what can our command line args look like?

The basics
~$ ./a.out -v -d positional_param -f file.txt -r recordone --r recordtwo

Long Opts
~$ ./a.out --verbose --file file.txt

Mixed (note the lack of space after -f and its arg, that's allowed for little
opts only)
~$ ./a.out -d -ffile.txt --verbose

Combined little opts (note that any option taking an argument has to
come last in the chain.  Otherwise anything after it gets parsed as
the argument)
~$ ./a.out -vdffile.txt

We can re-use any argument. This would make our output double verbose
according to the example above.
~$ ./a.out -v --verbose

With the devour flag in action, -v and -d will be availible in
arg_list->argv.  They will not be treated as parameters.
~$ ./a.out -ffile.txt -- -v -d
```

Author
------

I'm Jack Forrest (jack@jrforrest.net).  I'm still learning C, so I'm definitely
open to any critiques or feedback offered.


Feature Requests
---------------

Send em' to jack@jrforrest.net with liboptbot in the subject. Let me
know if you need anything on the list above and I'll prioritize it if possible.

Documentation
------------

run `doxygen ./doxygen.conf` to populate the ./doc directory with html and
latex folders.

Bugs
----

I'm sure there's plenty.  Email bugs@jrforrest.net to report for now.

Contributing
------------

If you've got a feature to contribute, ask me first if I'd include it,  Email me
a patch for review once you've written it.  Cover your functionality with
integration level tests at the very least, and include doxygen headers on
everything.

I encourage you to fork this project if I decline your feature.

Tests
-----

`make test` runs the few integration level tests I have for this lib. (You'll
need libcheck on your system.)  See /test/README for more info.

Style
-----

My style is weird, sorry.  Submit a patch converting it to TBS if you want.
Adhere to it otherwise.  Tabs are soft, 2 spaces.  Keep lines < 81 chars.

I'm not going to codify my crazy-ass bracket/function definition style, just
do as I do in /lib and /test.

Listen To
---------

The making of this library would not be possible without
some auditory inspiration.  Thanks goes to:

* The Blue Scholars
* Emancipator
* The Tumbleweed Wanderers
* Ha Ha Tonka
* Vendetta Red
* R.L. Burnside
* Saint James Electronic
* Flying Lotus

Disclaimer
----------

The author and contributors of this project may not
be held responsible if our code breaks your shit.

THANK YOU AND ENJOY
