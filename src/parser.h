/* Copyright (c) 2023, Robert-Ioan Constantinescu */

#pragma once

#include <stdio.h>  // For size_t

extern char prompt[];

typedef struct command {
    char *(*fp)(size_t argc, char **argv);
    char name[BUFSIZ];
} command_t;

char *parser_loop(command_t cmdv[], size_t cmdc);
