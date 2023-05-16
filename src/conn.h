/* Copyright (c) 2023, Robert-Ioan Constantinescu */

#pragma once

#include <stdlib.h>

#include "parson.h"

#define HTTP_LINE_TERM      "\r\n"
#define HTTP_HEADER_TERM    "\r\n\r\n"
#define HTTP_CONTENT_LENSTR "Content-Length:  "

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);
void send_to_server(int sockfd, char *message);
void compute_message(char *message, const char *line);
char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, size_t cookies_count, char *token);
char *compute_post_request(char *host, char *url, char *content_type, JSON_Value *json, char *token);
char *compute_delete_request(char *host, char *url, char *query_params, char *token);