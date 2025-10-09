#include <stdbool.h>

#include "../argsparse.h"

int main(int argc, char *argv[])
{
    DEFINE_ARGSPARSE("test1", parse);

    argsparse_add_opt(&parse, "input", STR, "DEFAULT_INPUT_FILE", "This is input file");
    argsparse_add_opt(&parse, "number", INT, 10000000, "This is integer");
    argsparse_add_opt(&parse, "boolean", BOOL, true, "This is boolean");

    argsparse_parse(&parse, argc, argv);

    printf("input: %s\n", (char *)argsparse_get_arg(&parse, "input"));
    printf("number: %d\n", (int)argsparse_get_arg(&parse, "number"));
    printf("boolean: %d\n", (int)argsparse_get_arg(&parse, "boolean"));

    argsparse_exit(&parse);

    return 0;
}
