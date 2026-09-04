#pragma once 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    HTML,
    PNG,
    JPG
}HTTP_Out_Types;

typedef enum{
     GET,
     POST,
     UNKNOWN,
     GET_ASSET,
}REQ_TYPE;

typedef struct{
    REQ_TYPE type;
    char path[50]; // TODO: use dynamic
}HTTP_Request;

typedef struct{
    char others[1000];
    char *data;
    size_t size;
}HTTP_Response;

HTTP_Request parse_http(char *test);
HTTP_Response create_http(HTTP_Out_Types type, char *str, size_t len );
int compare_endpoint(char *name, HTTP_Request *hr);
