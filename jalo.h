#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <lualib.h>
#include <lauxlib.h>
#include "http.h"

#define BUFFER_SIZE 1024

typedef struct {
    int client_fd;
    char *buffer;
    size_t buffer_size;
} JaloClient;


typedef struct {
    char *str;
    size_t len_str;
    HTTP_Out_Types type;
}JaloOutput;


typedef struct {
    char *name;
    JaloOutput (*func)(HTTP_Request);
} JaloEndpoint;


typedef struct {
    int server_fd;
    struct sockaddr_in address;

    JaloClient clients[5000]; // TODO: dynamic allocation and size increment
    JaloEndpoint endpoints[5000]; // TODO: dynamic allocation and size increment

    size_t client_counter;
    size_t endpoint_counter;

    lua_State *L;
} JaloServe;


JaloServe jalo_init(void);
void jalo_run(JaloServe *js, int port); void jalo_deinit(JaloServe *js);
JaloOutput jalo_string_output(char *str);
// TODO: use variadics in string_output too
JaloOutput jalo_file_output(char *str);
JaloOutput jalo_html_output(char *file_name);
void jalo_register(JaloServe *js, char *name, JaloOutput (*func)(HTTP_Request));
void jalo_execute_file(JaloServe *js ,char *file_name);
void jalo_execute(JaloServe *js ,char *code);
JaloOutput jalo_render_template(JaloServe *js, char *file_name);
