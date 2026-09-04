#include "http.h"


HTTP_Request parse_http(char *test){
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

HTTP_Response create_http(HTTP_Out_Types type, char *str, size_t len ){
    char *header = "HTTP/1.1 200 OK\r\n" \
                    "Content-Type: "  ;
    char *header2;
    switch(type){
        case HTML:
            header2 =  "text/html; charset=UTF-8\r\n";
            break;
        case JPG: header2 =  "image/jpeg\r\n"; break;
        default:
            printf("The file Format isn't recognized or implemented\n");
            exit(0);
    }
    char *header3 = "Content-Length: ";

    HTTP_Response hresponse = {.data = str, .size = len};
    sprintf(hresponse.others,"%s%s%s%ld\r\n\r\n",header,header2, header3, len);
    return hresponse;
}

int compare_endpoint(char *name, HTTP_Request *hr){
    int len1 = strlen(name);
    int len2 = strlen(hr->path);
    int len3 = (len1 > len2) ? len1 : len2;

    // TODO: won't work for multiple * and isn't rigid 
    for (int i = 0; i < len3; i++){
        if (!(name[i] == hr->path[i])){
            if(name[i] == '*'){
                // strcpy(hr->path,hr->path + i);
                return 1;
            }
            return 0;
        }
    }
    return 1;
}

