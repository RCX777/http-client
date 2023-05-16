/* Copyright (c) 2023, Robert-Ioan Constantinescu */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "parson.h"
#include "conn.h"

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, HTTP_LINE_TERM);
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    int sockfd = socket(ip_type, socket_type, flag);
    struct sockaddr_in serv_addr = {
        .sin_family = ip_type,
        .sin_port = htons(portno)
    };
    inet_aton(host_ip, &serv_addr.sin_addr);
    /* connect the socket */
    connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    return sockfd;
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total       = strlen(message);
    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (!bytes) break;
        sent += bytes;
    } while (sent < total);
}

char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, size_t cookies_count, char *token)
{
    char *message = calloc(1, BUFSIZ);
    char  line[BUFSIZ];

    // write the method name, URL and protocol type
    if (query_params) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add headers and/or cookies, according to the protocol format
    if (cookies) {
        for (size_t i = 0; i < cookies_count; ++i) {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    if (token) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, JSON_Value *json, char *token)
{
    char *message = calloc(1, BUFSIZ);
    char  line[BUFSIZ];

    char *json_str = json_serialize_to_string(json);

    // write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add necessary headers (Content-Type and Content-Length are mandatory)
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

	sprintf(line, "Content-Length: %ld", strlen(json_str));
    compute_message(message, line);

    // add token
    if (token) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line); 
    }

    // add new line at end of header
    compute_message(message, "");
    
    // add the actual payload data
    compute_message(message, json_str);
    free(json_str);

    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params, char *token)
{
    char *message = calloc(1, BUFSIZ);
    char  line[BUFSIZ];

    // write the method name, URL and protocol type
    if (query_params) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add token
    if (token) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");
    return message;
}
