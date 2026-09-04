#include "jalo.h"

static HTTP_Request parse_http(char *test){
   char temp[50] = {0};  
   HTTP_Request hr = {};

   int counter = 0;

   while (test[counter] != ' '){
       temp[counter] = test[counter];
       counter ++;
   }
   if (strcmp(temp,"GET")==0){
       hr.type = GET;
   }else if (strcmp(temp,"POST") == 0){
       hr.type = POST;
   }else{
       printf("Not Implemented Request %s", temp);
       hr.type = UNKNOWN;
   }

   counter++;
   size_t start = counter;

   while (test[counter] != ' '){
       counter ++;
   }
   memcpy(hr.path, test+start,counter-start+1);
   hr.path[counter-start] = '\0';

   return hr;
}

typedef struct{
    char others[1000];
    char *data;
    size_t size;
}HTTP_Response;


static HTTP_Response create_http(JaloOutput *jo){
    char *header = "HTTP/1.1 200 OK\r\n" \
                    "Content-Type: "  ;
    char *header2;
    switch(jo->type){
        case HTML:
            header2 =  "text/html; charset=UTF-8\r\n";
            break;
        case JPG:
            header2 =  "image/jpeg\r\n";
            break;
        default:
            printf("The file Format isn't recognized or implemented\n");
            exit(0);
    }
    char *header3 = "Content-Length: ";

    HTTP_Response hresponse = {.data = jo->str, .size = jo->len_str};
    sprintf(hresponse.others,"%s%s%s%ld\r\n\r\n",header,header2, header3, jo->len_str);
    return hresponse;
}

static int compare_endpoint(char *name, HTTP_Request *hr){
    int len1 = strlen(name);
    int len2 = strlen(hr->path);
    int len3 = (len1 > len2) ? len1 : len2;

    // TODO: won't work for multiple * and isn't rigid 
    for (int i = 0; i < len3; i++){
        if (!(name[i] == hr->path[i])){
            if(name[i] == '*'){
                return 1;
            }
            return 0;
        }
    }
    return 1;
}


static void handle_client(JaloServe *js, JaloClient jc){
    read(jc.client_fd, jc.buffer, jc.buffer_size);

    HTTP_Request hr = parse_http(jc.buffer);
    printf("[JALO] Requested %s.\n", hr.path);
    if (hr.type == POST){
        printf("[JALO] Post request Not Implemented\n");
        return;
    }else if (hr.type == UNKNOWN) {
        printf("[JALO] Unknown request\n");
        return;
    }

    for (size_t i = 0; i < js->endpoint_counter; i++){
        if (compare_endpoint(js->endpoints[i].name, &hr)){
            // directly returning FIX TODO
            JaloOutput jo = js->endpoints[i].func(hr);
            HTTP_Response output = create_http(&jo);

            write(jc.client_fd, output.others, strlen(output.others));
            write(jc.client_fd, output.data, output.size);

            printf("[JALO] 400 Delivered for %s\n", hr.path);
            free(output.data);
            return;
        }
    }
    printf("[JALO] 404 Haven't implemented callback for %s\n", hr.path);
}

void jalo_run(JaloServe *js, int port){
    int new_socket;

    js->address.sin_family = AF_INET;
    js->address.sin_addr.s_addr = INADDR_ANY;
    js->address.sin_port = htons(port);
    int addrlen = sizeof(js->address);

    if (bind(js->server_fd, (struct sockaddr *)&js->address, sizeof(js->address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(js->server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[JALO] Server is listening ...... at [%d]\n", port);

    while (1) {
        if ((new_socket = accept(js->server_fd, (struct sockaddr *)&js->address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // TODO: Make the following threaded
        /*********************/
            JaloClient jc = {.client_fd = new_socket,.buffer_size = BUFFER_SIZE};
            jc.buffer = (char *)malloc(BUFFER_SIZE);
            memset(jc.buffer,0,BUFFER_SIZE);
            handle_client(js,jc);
            close(jc.client_fd);
            free(jc.buffer);
        /*********************/
    }

}

void jalo_register(JaloServe *js, char *name, JaloOutput (*func)(HTTP_Request)){
    js->endpoints[js->endpoint_counter] = (JaloEndpoint){.name = name, .func = func};
    js->endpoint_counter ++;
}

void jalo_deinit(JaloServe *js){
    close(js->server_fd);
}

JaloOutput jalo_file_output(char *file_name) {
    FILE *fp = fopen(file_name, "rb");
    if (!fp) {
        printf("Unable To open HTML File %s \n",file_name);
        exit(0);
    }

    fseek(fp,0,SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    char *temp = (char *) malloc(size);

    fread(temp,size,sizeof(char),fp);

    return (JaloOutput){ 
        .str = temp,
        .len_str = size,
        .type = JPG, // TODO could be anything
    };

}

JaloOutput jalo_render(char *file_name){
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        printf("Unable To open HTML File %s \n",file_name);
        exit(0);
    }

    fseek(fp,0,SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    char *temp = (char *) malloc(size+1);
    fread(temp,size,1,fp);
    temp[size] = '\0'; 
    return (JaloOutput){ .str = temp,
        .len_str = size,
        .type = HTML
    };
}

JaloOutput jalo_string_output(char *str){
    size_t size = strlen(str);
    char *temp = (char *) malloc(size+1);
    memset(temp,0,size+1);
    strcpy(temp,str);
    return (JaloOutput){
        .str = temp,
        .len_str = size,
        .type = HTML
    };
}

JaloOutput _favicon (HTTP_Request hq) {
    return jalo_file_output("assets/logo.png");
}

JaloOutput _home (HTTP_Request hr) {
    return jalo_render("assets/default.html");
}

JaloServe jalo_init(void){
    JaloServe js = {.client_counter = 0,.endpoint_counter = 0};
    int opt = 1;

    if ((js.server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(js.server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    jalo_register(&js,"/favicon.ico",_favicon);
    jalo_register(&js,"/",_home);

    return js;
}
