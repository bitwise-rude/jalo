#include "jalo.h"

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
            HTTP_Response output = create_http(jo.type,jo.str,jo.len_str);

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
    for (int i =0; i< js->endpoint_counter; i++){
        if (strcmp(js->endpoints[i].name,name) == 0){
            js->endpoints[i].func = func;
            return;
        }
    }

    js->endpoints[js->endpoint_counter] = (JaloEndpoint){.name = name, .func = func};
    js->endpoint_counter ++;
}

void jalo_deinit(JaloServe *js){
    close(js->server_fd);
    lua_close(js->L);
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
JaloOutput jalo_render_template(JaloServe *js, char *file_name) {
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        printf("Unable To open HTML File %s \n", file_name);
        exit(0);
    }

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    char *temp = (char *) malloc(size + 1);
    fread(temp, size, 1, fp);
    temp[size] = '\0';
    fclose(fp);

    size_t lua_cap = size * 6 + 1024;
    char *lua_script = (char *) malloc(lua_cap);
    
    size_t out_idx = 0;
    out_idx += snprintf(lua_script + out_idx, lua_cap - out_idx, "local _buf = {}\n");

    size_t i = 0;
    while (i < size) {
        if (temp[i] == '{' && temp[i+1] == '%') {
            i += 2; // skip {%
            
            size_t code_start = i;
            while (i < size && !(temp[i] == '%' && temp[i+1] == '}')) {
                i++;
            }
            size_t code_len = i - code_start;
            if (i < size) i += 2; // skip %}

            out_idx += snprintf(lua_script + out_idx, lua_cap - out_idx, "%.*s\n", (int)code_len, temp + code_start);
        }
        else if (temp[i] == '{' && temp[i+1] == '{') {
            i += 2; // skip {{
            
            size_t expr_start = i;
            while (i < size && !(temp[i] == '}' && temp[i+1] == '}')) {
                i++;
            }
            size_t expr_len = i - expr_start;
            if (i < size) i += 2; // skip }}

            out_idx += snprintf(lua_script + out_idx, lua_cap - out_idx, 
                                "table.insert(_buf, tostring(%.*s))\n", (int)expr_len, temp + expr_start);
        }
        else {
            size_t text_start = i;
            while (i < size && !(temp[i] == '{' && (temp[i+1] == '{' || temp[i+1] == '%'))) {
                i++;
            }
            size_t text_len = i - text_start;

            if (text_len > 0) {
                out_idx += snprintf(lua_script + out_idx, lua_cap - out_idx, "table.insert(_buf, \"");
                
                for (size_t k = 0; k < text_len; k++) {
                    char c = temp[text_start + k];
                    if (out_idx + 10 >= lua_cap) break; 
                    
                    if (c == '"' || c == '\\') {
                        lua_script[out_idx++] = '\\';
                        lua_script[out_idx++] = c;
                    } else if (c == '\n') {
                        lua_script[out_idx++] = '\\';
                        lua_script[out_idx++] = 'n';
                    } else if (c == '\r') {
                        lua_script[out_idx++] = '\\';
                        lua_script[out_idx++] = 'r';
                    } else if (c == '\t') {
                        lua_script[out_idx++] = '\\';
                        lua_script[out_idx++] = 't';
                    } else {
                        lua_script[out_idx++] = c;
                    }
                }
                lua_script[out_idx] = '\0';
                
                out_idx += snprintf(lua_script + out_idx, lua_cap - out_idx, "\")\n");
            }
        }
    }

    out_idx += snprintf(lua_script + out_idx, lua_cap - out_idx, "return table.concat(_buf)\n");
    free(temp);

    if (luaL_dostring(js->L, lua_script) != LUA_OK) {
        printf("Template execution error: %s\n", lua_tostring(js->L, -1));
        free(lua_script);
        exit(0);
    }

    const char *rendered = lua_tostring(js->L, -1);
    size_t rendered_len = rendered ? strlen(rendered) : 0;

    char *final_output = (char *) malloc(rendered_len + 1);
    if (rendered) {
        strcpy(final_output, rendered);
    } else {
        final_output[0] = '\0';
    }

    lua_pop(js->L, 1);
    free(lua_script);

    return (JaloOutput){
        .str = final_output,
        .len_str = rendered_len,
        .type = HTML
    };
}


JaloOutput jalo_html_output(char *file_name){
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


JaloOutput _static (HTTP_Request hq) {
    return jalo_file_output(hq.path+1);
}

JaloOutput _favicon (HTTP_Request hq) {
    strcpy(hq.path,"/assets/logo.png");
    return _static (hq);
}

JaloOutput _home (HTTP_Request hr) {
    return jalo_html_output("assets/default.html");
}

void jalo_execute(JaloServe *js ,char *code){
    luaL_dostring(js->L, code);
}
void jalo_execute_file(JaloServe *js ,char *file_name){
    luaL_dofile(js->L, file_name);
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

    // jalo_register(&js,"/assets/*",_static);
    jalo_register(&js,"/favicon.ico",_favicon);
    // jalo_register(&js,"/",_home);

    // Initialize Lua
    js.L = luaL_newstate();
    if (js.L == NULL){
        printf("LUA can't initilzate\n");
        exit(0);
    }
    luaL_openlibs(js.L);
    return js;
}
