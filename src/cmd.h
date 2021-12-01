#ifndef CMD_INCLUDE_H_
#define CMD_INCLUDE_H_
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#define CMD_ARGV_MAX 8

struct cmd_args {
        char *argv[CMD_ARGV_MAX];
        uint8_t argc;
};

/// parameter data only valid in callback
typedef void (cmd_cb)(struct cmd_args *args);


static void cmd_make_argv(struct cmd_args *p, char *s)
{
    uint8_t n = 0;
    uint8_t have_arg = false;
    while (true) {
        char c = *s;
        if (c == '\0') {
            break;
        }

        if (isspace(c)) {
            *s++ = '\0';
            have_arg = false;
            continue;
        }

        if (!have_arg) {
            p->argv[n++] = s;
            if (n >= CMD_ARGV_MAX) {
                break;
            }
            have_arg = true;
        }

        s++;
    };

    p->argc = n;
    p->argv[n] = 0;
}

#endif

