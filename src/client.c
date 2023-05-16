/* Copyright (c) 2023, Robert-Ioan Constantinescu */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/in.h>

#include "parson.h"

#include "parser.h"
#include "colors.h"
#include "startup.h"
#include "conn.h"

#define HOST "34.254.242.81"
#define PORT 8080

int sockfd    = 0;
int logged_in = 0;

char *cookie  = NULL;

char *client_exit(size_t argc, char **argv);
char *client_post_register(size_t argc, char **argv);
char *client_post_login(size_t argc, char **argv);
char *client_get_logout(size_t argc, char **argv);
int main(void)
{
    startup(); // Print startup message

    command_t cmds[] = {
        {.fp = client_exit,          .name = "exit"},
        {.fp = client_post_register, .name = "register"},
        {.fp = client_post_login,    .name = "login"},
        {.fp = client_get_logout,    .name = "logout"}
    };

    char *error_msg = parser_loop(cmds, sizeof(cmds) / sizeof(*cmds));
    int   status    = EXIT_SUCCESS;

    if (strcmp(error_msg, "Success")) {
        fprintf(stderr, RED"%s"reset"\n", error_msg);
        status = EXIT_FAILURE;
    }

    free(error_msg);
    free(cookie);

    if (sockfd)
        close(sockfd);

    return status;
}

char *client_exit(size_t argc, char **argv)
{
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

char *client_post_register(size_t argc, char **argv)
{
    char *message          = NULL;
    char  response[BUFSIZ] = { };
    char  username[BUFSIZ] = { };
    char  password[BUFSIZ] = { };
    char *retmsg           = NULL;

    if (argc != 1) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg,
            "Warning: `%s`: Invalid number of arguments `%zu`!\n"
            "Usage: %s\n",
            argv[0],
            argc,
            argv[0]
        );
        return retmsg;
    }

    if (logged_in) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg,
            "Warning: `%s`: Already logged in!\n",
            argv[0]
        );
        return retmsg;
    }

    if (!sockfd) sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    printf("To proceed with "GRN"registration"reset", please provide your data:\n");
    printf(CYN"Username"reset" = ");
    scanf("%s", username);
    printf(CYN"Password"reset" = ");
    scanf("%s", password);

    JSON_Value *root = json_value_init_object();
    JSON_Object *obj = json_value_get_object(root);
    json_object_set_string(obj, "username", username);
    json_object_set_string(obj, "password", password);

    message = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", root, NULL);
    send_to_server(sockfd, message);
    read(sockfd, response, BUFSIZ);

    char *error = strstr(response, "{\"error\":");
    if (error) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg, "Warning: {\"%s\n", error + sizeof("{\"error\":"));
    } else {
        printf("Succesfully "GRN"registered"reset"! Use the `"GRN"login"reset"` command to access your account!\n");
    }

    close(sockfd);
    sockfd = 0;
    json_value_free(root);
    free(message);
    return retmsg;
}

char *client_post_login(size_t argc, char **argv)
{
    char *message          = NULL;
    char  response[BUFSIZ] = { };
    char  username[BUFSIZ] = { };
    char  password[BUFSIZ] = { };
    char *retmsg           = NULL;

    if (argc != 1) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg,
            "Warning: `%s`: Invalid number of arguments `%zu`!\n"
            "Usage: %s\n",
            argv[0],
            argc,
            argv[0]
        );
        return retmsg;
    }

    if (logged_in) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg,
            "Warning: `%s`: Already logged in!\n",
            argv[0]
        );
        return retmsg;
    }

    if (!sockfd) sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    printf("To proceed with "GRN"logging in"reset", please provide your data:\n");
    printf(CYN"Username"reset" = ");
    scanf("%s", username);
    printf(CYN"Password"reset" = ");
    scanf("%s", password);

    JSON_Value *root = json_value_init_object();
    JSON_Object *obj = json_value_get_object(root);
    json_object_set_string(obj, "username", username);
    json_object_set_string(obj, "password", password);

    message = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", root, NULL);
    send_to_server(sockfd, message);
    read(sockfd, response, BUFSIZ);

    char *error = strstr(response, "{\"error\":");
    if (error) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg, "Warning: {\"%s\n", error + sizeof("{\"error\":"));
    } else {
        sprintf(prompt, "("CYN"%s"reset") >>> ", username);
        logged_in = 1;
        char *p = strstr(response, "Set-Cookie: ");
        if (p) {
            strtok(p, " ;");
            char *cp = strtok(NULL, " ;");
            cookie = malloc(BUFSIZ);
            strcpy(cookie, cp);
        }
    }

    close(sockfd);
    sockfd = 0;
    json_value_free(root);
    free(message);
    return retmsg;
}

char *client_get_logout(size_t argc, char **argv)
{
    char *message          = NULL;
    char  response[BUFSIZ] = { };
    char *retmsg           = NULL;

    if (argc != 1) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg,
            "Warning: `%s`: Invalid number of arguments `%zu`!\n"
            "Usage: %s\n",
            argv[0],
            argc,
            argv[0]
        );
        return retmsg;
    }

    if (!logged_in) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg,
            "Warning: `%s`: You must be logged in first!\n",
            argv[0]
        );
        return retmsg;
    }

    if (!sockfd) sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, &cookie, 1, NULL);

    send_to_server(sockfd, message);

    read(sockfd, response, BUFSIZ);
    char *error = strstr(response, "{\"error\":");
    if (error) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg, "Warning: {\"%s\n", error + sizeof("{\"error\":"));
    } else {
        sprintf(prompt, ">>> ");
        logged_in = 0;
        free(cookie);
        cookie = 0;
    }

    free(message);
    close(sockfd);
    sockfd = 0;
    return retmsg;
}