# Argument Parser in C

This is a header only argument parser in C.

## Get Start

```c
#include "argsparse.h"
```

## APIs

Now we only support 26 options, which might be enough.

```c
/* Define the new argument parser. */
DEFINE_ARGSPARSE(<program name>, parse);

/*
 * Add the new argument to the parser.
 * @type can be STR, BOOL, INT
 */
int argsparse_add_opt(struct argsparse_struct *parse,
                      const char *option_name, unsigned int type,
                      unsigned long init, const char *help);

/* Parse the arguments. */
int argsparse_parse(struct argsparse_struct *parse, int argc, char *argv[]);

unsigned long argsparse_get_arg(struct argsparse_struct *parse,
                                const char *name);

void argsparse_exit(struct argsparse_struct *parse);
```

## Example

See [this](./tests/test.c).

```bash
$ ./test --help
Usage test1 [--boolean] [--number <INTEGER>] [--input <STRING>]
Options:
  --boolean   This is boolean
  --number    This is integer
  --input     This is input file

```
