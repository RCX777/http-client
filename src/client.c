/* Copyright (c) 2023, Robert-Ioan Constantinescu */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "parson.h"

#include "parser.h"
#include "colors.h"

char *client_exit(size_t argc, char **argv);


int main(void)
{
    command_t cmds[] = {
        {.fp = client_exit, .name = "exit"}
    };

    char *error_msg = parser_loop(cmds, sizeof(cmds) / sizeof(*cmds));
    int   status    = EXIT_SUCCESS;

    if (strcmp(error_msg, "Success")) {
        fprintf(stderr, RED"%s"reset"\n", error_msg);
        status = EXIT_FAILURE;
    }

    free(error_msg);

    return status;
}

char *client_exit(size_t argc, char **argv) {
    (void) argv;

    char *retmsg = malloc(BUFSIZ);

    if (argc != 1)
        sprintf(retmsg,
            "Warning: `%s`: Invalid number of arguments `%zu`!\n"
            "Usage: %s\n",
            argv[0],
            argc,
            argv[0]
        );
    else
        strcpy(retmsg, "Success");

    return retmsg;
}
