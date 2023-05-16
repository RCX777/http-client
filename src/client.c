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
char *token   = NULL;

char *client_exit(size_t argc, char **argv);
char *client_post_register(size_t argc, char **argv);
char *client_post_login(size_t argc, char **argv);
char *client_get_logout(size_t argc, char **argv);
char *client_get_access(size_t argc, char **argv);
char *client_get_books(size_t argc, char **argv);
char *client_add_book(size_t argc, char **argv);
char *client_del_book(size_t argc, char **argv);
int main(void)
{
    startup(); // Print startup message

    command_t cmds[] = {
        {.fp = client_exit,          .name = "exit"},
        {.fp = client_post_register, .name = "register"},
        {.fp = client_post_login,    .name = "login"},
        {.fp = client_get_logout,    .name = "logout"},
        {.fp = client_get_access,    .name = "enter_library"},
        {.fp = client_get_books,     .name = "get_books"},
        {.fp = client_add_book,      .name = "add_book"},
        {.fp = client_del_book,      .name = "delete_book"}
    };

    char *error_msg = parser_loop(cmds, sizeof(cmds) / sizeof(*cmds));
    int   status    = EXIT_SUCCESS;

    if (strcmp(error_msg, "Success")) {
        fprintf(stderr, RED"%s"reset"\n", error_msg);
        status = EXIT_FAILURE;
    }

    free(error_msg);
    free(cookie);
    free(token);

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

char *client_get_access(size_t argc, char **argv)
{
    char *message              = NULL;
    char  response[BUFSIZ * 4] = { };
    char *retmsg               = NULL;

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

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, &cookie, 1, NULL);
    send_to_server(sockfd, message);
    read(sockfd, response, BUFSIZ * 4);
    char *error = strstr(response, "{\"error\":");
    if (error) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg, "Warning: {\"%s\n", error + sizeof("{\"error\":"));
    } else {
        char *p = strstr(response, "token");

        if (p) {
            strtok(p, "\":");
            char *cp = strtok(NULL, "\":");
            token = malloc(BUFSIZ);
            strcpy(token, cp);
        }

        printf("You have "GRN"succesfully"reset" gained access to the library!\n");
        char *eou = strstr(prompt, ") ");
        strcpy(eou, ") @ "MAG"library"reset" >>> ");
    }

    free(message);
    close(sockfd);
    sockfd = 0;
    return retmsg;
}

char *client_get_books(size_t argc, char **argv)
{
    char *message              = NULL;
    char  response[BUFSIZ * 4] = { };
    char *retmsg               = NULL;

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

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

    message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, NULL, 0, token);
    send_to_server(sockfd, message);
    read(sockfd, response, BUFSIZ * 4);
    char *error = strstr(response, "{\"error\":");
    if (error) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg, "Warning: {\"%s\n", error + sizeof("{\"error\":"));
    } else {
        char *json_response = strstr(response, "[");
        JSON_Value *json = json_parse_string_with_comments(json_response);
        char *serialized_string = json_serialize_to_string_pretty(json);
        puts(serialized_string);
        json_free_serialized_string(serialized_string);
        json_value_free(json);
    }

    free(message);
    close(sockfd);
    sockfd = 0;
    return retmsg;
}

char *client_add_book(size_t argc, char **argv)
{
    char *message              = NULL;
    char  response[BUFSIZ * 4] = { };
    char *retmsg               = NULL;

    char title[BUFSIZ];
    char author[BUFSIZ];
    char genre[BUFSIZ];
    char page_count[BUFSIZ];
    char publisher[BUFSIZ];

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

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

    if (!token) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg,
            "Warning: `%s`: You must have entered the "MAG"library"reset" first!\n",
            argv[0]
        );
        return retmsg;
    }

    printf("To proceed with "GRN"adding a book"reset", please provide your data:\n");
    printf(CYN"Title"reset"\t\t= ");
    fgets(title, BUFSIZ, stdin); // to solve an input bug
    fgets(title, BUFSIZ, stdin); strtok(title, "\n");
    printf(CYN"Author"reset"\t\t= ");
    fgets(author, BUFSIZ, stdin); strtok(author, "\n");
    printf(CYN"Genre"reset"\t\t= ");
    fgets(genre, BUFSIZ, stdin); strtok(genre, "\n");
    printf(CYN"Page Count"reset"\t= ");
    fgets(page_count, BUFSIZ, stdin); strtok(page_count, "\n");
    printf(CYN"Publisher"reset"\t= ");
    fgets(publisher, BUFSIZ, stdin); strtok(publisher, "\n");

    JSON_Value *root = json_value_init_object();
    JSON_Object *obj = json_value_get_object(root);
    json_object_set_string(obj, "title", title);
    json_object_set_string(obj, "author", author);
    json_object_set_string(obj, "genre", genre);
    json_object_set_string(obj, "page_count", page_count);
    json_object_set_string(obj, "publisher", publisher);

    message = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json", root, token);
    send_to_server(sockfd, message);
    read(sockfd, response, BUFSIZ);

    char *error = strstr(response, "{\"error\":");
    if (error) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg, "Warning: {\"%s\n", error + sizeof("{\"error\":"));
    }

    close(sockfd);
    sockfd = 0;
    json_value_free(root);
    free(message);
    return retmsg;
}

char *client_del_book(size_t argc, char **argv)
{
    char *message              = NULL;
    char  response[BUFSIZ * 4] = { };
    char *retmsg               = NULL;
    char  id[BUFSIZ];
    char  book[BUFSIZ * 2];


    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

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

    if (!token) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg,
            "Warning: `%s`: You must have entered the "MAG"library"reset" first!\n",
            argv[0]
        );
        return retmsg;
    }

    printf("To proceed with "RED"deleting a book"reset", please provide your data:\n");
    printf(CYN"Book ID "reset"= ");
    fgets(id, BUFSIZ, stdin); strtok(id, "\n");
    sprintf(book, "/api/v1/tema/library/books/%s", id);


    message = compute_delete_request(HOST, book, NULL, token);
    send_to_server(sockfd, message);
    read(sockfd, response, BUFSIZ);

    char *error = strstr(response, "{\"error\":");
    if (error) {
        retmsg = malloc(BUFSIZ);
        sprintf(retmsg, "Warning: {\"%s\n", error + sizeof("{\"error\":"));
    } else {
        printf("Book `%s` succesfully "RED"deleted"reset"!\n", id);
    }

    close(sockfd);
    sockfd = 0;
    free(message);
    return retmsg;
}