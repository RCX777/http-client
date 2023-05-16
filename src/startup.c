/* Copyright (c) 2023, Robert-Ioan Constantinescu */

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "startup.h"

#define PROGNAME "HttpClient"
#define VERSION  "1.0.1"
#ifdef linux
    #define PLATFORM "Linux"
#endif
#ifdef _WIN32
    #define PLATFORM "Windows"
#endif
#ifdef __APPLE__
    #define PLATFORM "MacOS"
#endif
#ifndef PLATFORM
    #define PLATFORM "Unknown platform"
#endif

void startup(void)
{
    time_t t;
    time(&t);
    char timestr[BUFSIZ];
    strcpy(timestr, ctime(&t));
    timestr[strlen(timestr) - 1] = 0;

    printf(PROGNAME" "VERSION" (%s) on "PLATFORM"\n", timestr);
}
