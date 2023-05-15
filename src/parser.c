/* Copyright (c) 2023, Robert-Ioan Constantinescu */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "colors.h"

#include "parser.h"

static size_t parse_by_whitespace(char *buf, char **argv)
{
    size_t argc = 0;
    for (char *p = strtok(buf, " \t\n"); p; p = strtok(NULL, " \t\n"))
        argv[argc++] = p;
    return argc;
}

char *parser_loop(command_t cmdv[], size_t cmdc)
{
    char   lbuffer[BUFSIZ] = { };
    char  *cmdargv[BUFSIZ] = { };
    size_t cmdargc         = 0;

    size_t idx             = 0;

    char  *error_msg       = NULL;

    for (;;) {
        memset(lbuffer, 0, sizeof(lbuffer));
        memset(cmdargv, 0, sizeof(cmdargv));

        write(STDOUT_FILENO, ">>> ", sizeof(">>> "));
        read(STDIN_FILENO, lbuffer, sizeof(lbuffer));
        cmdargc = parse_by_whitespace(lbuffer, cmdargv);

        if (!cmdargc) continue;

        for (idx = 0; idx < cmdc; ++idx) {
            if (!strcmp(cmdargv[0], cmdv[idx].name)) {
                error_msg = cmdv[idx].fp(cmdargc, cmdargv);

                if (error_msg && strstr(error_msg, "Error:") == error_msg) {
                    return error_msg;
                } else if (error_msg && strstr(error_msg, "Warning:") == error_msg) {
                    fprintf(stderr, YEL"Warning"reset": %s", error_msg + sizeof("Warning:"));
                    free(error_msg);
                } else if (error_msg && !strcmp(error_msg, "Success")) {
                    return error_msg;
                }
                break;
            }

            printf(
                YEL"Warning"reset": Unknown command: "RED"%s"reset"\n"
                "Available commands are: \n",
                cmdargv[0]
            );

            for (idx = 0; idx < cmdc; ++idx) {
                printf(" -- "GRN"%s"reset"\n", cmdv[idx].name);
            }
        }
    }

    return NULL; // This point should not be reached!
}
