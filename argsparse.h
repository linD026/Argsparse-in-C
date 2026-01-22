#ifndef __PAD_LOGS_H__
#define __PAD_LOGS_H__

#include <stdio.h>

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#define logger_out_stream stdout
#define logger_err_stream stderr

#define pr_fmt ""

#define pr_info(fmt, ...)                                      \
    do {                                                       \
        fprintf(logger_out_stream, pr_fmt fmt, ##__VA_ARGS__); \
    } while (0)

#define pr_err(fmt, ...)                                       \
    do {                                                       \
        fprintf(logger_err_stream, pr_fmt fmt, ##__VA_ARGS__); \
    } while (0)

#ifdef CONFIG_DEBUG
#define pr_debug pr_info
#else
#define pr_debug(...)
#endif

#define BUG_ON(cond, fmt, ...)                                     \
    do {                                                           \
        if (unlikely(cond)) {                                      \
            pr_err("BUG ON: " #cond ", " fmt "\n", ##__VA_ARGS__); \
            exit(-1);                                              \
        }                                                          \
    } while (0)

#define WARN_ON(cond, fmt, ...)                                    \
    ({                                                             \
        int __w_r_ret = !!(cond);                                  \
        if (unlikely(__w_r_ret))                                   \
            pr_err("WARN ON:" #cond ", " fmt "\n", ##__VA_ARGS__); \
        unlikely(__w_r_ret);                                       \
    })

#endif /* __PAD_LOGS_H__*/

#ifndef __ARGSPARSE_H__
#define __ARGSPARSE_H__

#include <assert.h>
#include <stdlib.h> /* for atoi() */
#include <errno.h>
#include <getopt.h>
#include <string.h> /* for memcpy() */

#define ARG_OPT_INT_TYPE 0x0001
#define ARG_OPT_STR_TYPE 0x0002
#define ARG_OPT_BOOL_TYPE 0x0004
#define NR_ARG_OPT_TYPE 3

struct arg_opt_struct {
    const char *name;
    unsigned int type;
    const char *help;
    unsigned long data;
    unsigned int idx;
    struct arg_opt_struct *next;
};

struct argsparse_struct {
    const char *program_name;
    unsigned int nr_args;
    struct arg_opt_struct *head;
};

#define DEFINE_ARGSPARSE(_program_name, name)                       \
    struct argsparse_struct name = { .program_name = _program_name, \
                                     .nr_args = 0,                  \
                                     .head = NULL }

static inline int __argsparse_add_opt(struct argsparse_struct *parse,
                                      const char *name, unsigned int type,
                                      unsigned long init, const char *help)
{
    struct arg_opt_struct *opt = NULL;

    opt = (struct arg_opt_struct *)malloc(sizeof(struct arg_opt_struct));
    if (!opt)
        return -ENOMEM;

    opt->name = name;
    opt->type = type;
    opt->idx = parse->nr_args++;
    opt->data = init;
    opt->help = help;

    opt->next = parse->head;
    parse->head = opt;

    return 0;
}

#define argsparse_add_opt(parse, name, type, init, help)        \
    ({                                                          \
        assert(__builtin_constant_p(name));                     \
        assert(__builtin_constant_p(help));                     \
        __argsparse_add_opt(parse, name, ARG_OPT_##type##_TYPE, \
                            (unsigned long)init, help);         \
    })

static inline void argparse_show_helper(struct argsparse_struct *parse)
{
    printf("Usage %s ", parse->program_name);
    for (struct arg_opt_struct *curr = parse->head; curr != NULL;
         curr = curr->next) {
        const char *end = "";

        if (curr->next)
            end = " ";

        if (curr->type & ARG_OPT_BOOL_TYPE)
            printf("[--%s]%s", curr->name, end);
        else {
            const char *val = NULL;
            if (curr->type & ARG_OPT_INT_TYPE)
                val = "INTEGER";
            else if (curr->type & ARG_OPT_STR_TYPE)
                val = "STRING";
            printf("[--%s <%s>]%s", curr->name, val, end);
        }
    }
    printf("\nOptions:\n");

    for (struct arg_opt_struct *curr = parse->head; curr != NULL;
         curr = curr->next) {
        printf("  --%-10s%s\n", curr->name, curr->help);
    }
}

static inline int argsparse_parse(struct argsparse_struct *parse, int argc,
                                  char *argv[])
{
    int i = 0;
    int help_val = 0;
    int opt;
    int opt_index;
    struct option *options = NULL;

    /* for help option */
    parse->nr_args++;

    options = (struct option *)malloc(parse->nr_args * sizeof(struct option));
    if (!options)
        return -ENOMEM;

    i = 0;
    for (struct arg_opt_struct *curr = parse->head; curr != NULL;
         curr = curr->next, i++) {
        struct option *op = &options[i];

        op->name = curr->name;
        if (curr->type & ARG_OPT_BOOL_TYPE)
            op->has_arg = no_argument;
        else
            op->has_arg = required_argument;
        op->flag = 0;
        op->val = 'a' + curr->idx;
    }

    /* help option */
    options[parse->nr_args - 1].name = "help";
    options[parse->nr_args - 1].has_arg = no_argument;
    options[parse->nr_args - 1].flag = 0;
    help_val = 'a' + parse->nr_args - 1;
    options[parse->nr_args - 1].val = help_val;

    /* parse the option */
#define OPT_STRING "abcdefghijklmnopqrstuvwxyz"
    while ((opt = getopt_long(argc, argv, OPT_STRING, options, &opt_index)) !=
           -1) {
        int idx = opt - 'a';
        struct arg_opt_struct *tmp = NULL;

        if (help_val == options[opt_index].val) {
            argparse_show_helper(parse);
            exit(0);
        }

        BUG_ON(!optarg, "option: %s is null", options[opt_index].name);

        for (struct arg_opt_struct *curr = parse->head; curr != NULL;
             curr = curr->next, i++) {
            if (curr->idx == idx) {
                tmp = curr;
                break;
            }
        }

        if (!tmp)
            return -EINVAL;

        if (tmp->type & ARG_OPT_BOOL_TYPE) {
            tmp->data = 1;
        } else if (tmp->type & ARG_OPT_INT_TYPE) {
            tmp->data = (typeof(tmp->data))atoll(optarg);
        } else if (tmp->type & ARG_OPT_STR_TYPE) {
            size_t size = strlen(optarg);
            tmp->data = (unsigned long)malloc(size + 1);
            if (!tmp->data)
                return -ENOMEM;
            strncpy((char *)tmp->data, optarg, size + 1);
            ((char *)tmp->data)[size] = '\0';
        }
    }

#undef OPT_STRING

    free(options);

    return 0;
}

static inline unsigned long argsparse_get_arg(struct argsparse_struct *parse,
                                              const char *name)
{
    for (struct arg_opt_struct *curr = parse->head; curr != NULL;
         curr = curr->next) {
        if (!(strncmp(curr->name, name, strlen(curr->name))))
            return curr->data;
    }

    return 0;
}

static inline void argsparse_exit(struct argsparse_struct *parse)
{
    for (struct arg_opt_struct *curr = parse->head; curr != NULL;
         curr = curr->next) {
        struct arg_opt_struct *tmp = curr->next;
        free(curr);
        curr = tmp;
        if (!curr)
            break;
    }
}

#endif /* __ARGSPARSE_H__ */
