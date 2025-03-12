// login
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgi_lib.h"
// 生成随机Cookie值
char dec_authorization[512]  __attribute__((__no_reorder__));
__uint8_t remote_code  __attribute__((__no_reorder__)) = 0;
char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char *loc = "/login.html";
enum {
    MODE_LOGIN = 1,
    MODE_RESET_PASSWORD = 2,
    MODE_REGISTER = 3,
    MODE_ADD_SESSION = 4,
};

static int base64_char_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1; // 非法字符
}


void base64_decode(unsigned char *input ,unsigned char *decoded_buffer, int *decoded_length)  
{  
    int table[]={0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,62,0,0,0,
    		 63,52,53,54,55,56,57,58,
    		 59,60,61,0,0,0,0,0,0,0,0,
    		 1,2,3,4,5,6,7,8,9,10,11,12,
    		 13,14,15,16,17,18,19,20,21,
    		 22,23,24,25,0,0,0,0,0,0,26,
    		 27,28,29,30,31,32,33,34,35,
    		 36,37,38,39,40,41,42,43,44,
    		 45,46,47,48,49,50,51
    	       };  
    long len;  
    int i,j;  
  
    len=strlen(input);  
    if(strstr(input,"=="))  
        *decoded_length=len/4*3-2;  
    else if(strstr(input,"="))  
        *decoded_length=len/4*3-1;  
    else  
        *decoded_length=len/4*3;  
    
  
    for(i=0,j=0;i < len-2;j+=3,i+=4)  
    {  
        decoded_buffer[j]=((unsigned char)table[input[i]])<<2 | (((unsigned char)table[input[i+1]])>>4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
        decoded_buffer[j+1]=(((unsigned char)table[input[i+1]])<<4) | (((unsigned char)table[input[i+2]])>>2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
        decoded_buffer[j+2]=(((unsigned char)table[input[i+2]])<<6) | ((unsigned char)table[input[i+3]]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
    }  
    decoded_buffer[*decoded_length]='\0';
  
  
}  

void generate_random_cookie(char *cookie, size_t length) {
    srand(time(NULL));
    size_t charset_size = sizeof(charset) - 1;
    for (size_t i = 0; i < length; ++i) {
        int key = rand() % charset_size;
        cookie[i] = charset[key];
    }
    cookie[length] = '\0'; // 添加字符串结束符
}


void send_redirect(const char *url) {
        printf("Status: 200 OK\n");
        printf("X-Redirect-URL: %s\n\n",url);
        printf("Content-Type: text/html\n\n");

}
// 生成HTTP响应头以设置Cookie并重定向
void send_redirect_Cookie(char * cookie_value,const char *redirect_url ,char* session_id) {

    printf("Status: 200 OK\n");
    //printf("Status: 302 Found\n");
    printf("Set-Cookie: Cookie=%s; Path=/; HttpOnly\n", cookie_value);
    printf("Set-Cookie: session_id=%s; Path=/; HttpOnly;\n", session_id);
    //printf("Location: %s\n", redirect_url);
    printf("X-Redirect-URL: %s\n\n",redirect_url);
    printf("Content-Type: text/html\n\n"); // 双换行结束头信息

    printf("<html><body>\n");
    printf("<h1>Redirecting to <a href=\"%s\">%s</a></h1>\n", redirect_url, redirect_url);
    printf("</body></html>\n");
}


int is_base64(char *s) {
    const char *base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i;

    for (i = 0; s[i]; i++) {
        if (!strchr(base64chars, s[i])) {
            if (s[i] == '=' && s[i+1] == '\0') {
                return 1; // Allow = at the end for padding
            }
            else if(s[i] == '=' && s[i+1] == '='&& s[i+2] == '\0')
            {
                return 1; // Allow = at the end for padding
            }
            return 0; // If character not in base64chars, it's not base64.
        }
    }

    return 1; // If we get here, all characters were in the base64 set.
}


char*pack_packet(int remote_code,char*username,char*password,char*resetpass,char*comfirmpass) {
    char *pack_struct = malloc(520);
    memset(pack_struct,0,520);
    memcpy(pack_struct,username,128);
    memcpy(pack_struct+128,password,128);
    if (remote_code == MODE_RESET_PASSWORD) {
        memcpy(pack_struct+256,resetpass,128);
        memcpy(pack_struct+384,comfirmpass,128);
    }
    else if (remote_code == MODE_REGISTER) {
        memcpy(pack_struct+256,comfirmpass,128);
    }
    return pack_struct;
}

int main() {

    char *data;
    char *username;
    char *password;
    char *resetpass;
    char *comfirmpass; 

    char *location = malloc(strlen(loc)+100);
    strcpy(location,loc);
    char* uri_para = getenv("QUERY_STRING");
    if (uri_para)
    {
        if(!strcmp(uri_para,"login")) {
            remote_code = MODE_LOGIN;
            // while(1);
        }
        else if(!strcmp(uri_para,"reset_password")) {
            remote_code = MODE_RESET_PASSWORD;
        }
        else if(!strcmp(uri_para,"register")) {
            remote_code = MODE_REGISTER;
        }
    }
    if (remote_code == 0) {
        goto failed_incorrect;
    }

    char *method = getenv("REQUEST_METHOD");
    if (method && strcmp(method, "POST") == 0) {
        int content_length = atoi(getenv("CONTENT_LENGTH"));
        if (content_length <=0 || content_length >0x800000)
        {
            failed_incorrect:;
            strcat(location,"?error=incorrect");
            send_redirect(location);
            return 0;
        }
        
        data = malloc(content_length + 1);
        char *id;
        if (data) {
            fread(data, 1, content_length, stdin);
            data[content_length] = '\0';
            if (!strncmp(data,"id=",3)){
                id = data +3;
                if (strlen(id) != 24) {
                    goto failed_incorrect;
                }
            }
            else{
                goto failed_incorrect;
            }
        }

        //获取Authorization字段的内容
        char* authorization = getenv("HTTP_AUTHORIZATION");

        if (!strncmp(authorization,"Basic ",6)){
            authorization += 6;
            if(strlen(authorization) > 1024) {
                goto failed_incorrect;
            }
            char * enc_authorization = malloc(strlen(authorization)+1);
            //char * dec_authorization = malloc(strlen(authorization)+1);

            strcpy(enc_authorization,authorization);
            if(is_base64(enc_authorization)) {
                int dec_len = 512;
                base64_decode(enc_authorization,dec_authorization,&dec_len);
                char*p = dec_authorization;
                char* colon = strchr(p,':');
                if (!colon){
                    goto failed_incorrect;
                }
                *colon = 0;
                if (strlen(p) > 128) {
                    goto failed_incorrect;
                }
                username = p;
                p = colon + 1;
                if (remote_code == MODE_LOGIN) {
                    password = p;
                }
                else {
                    colon = strchr(p,':');
                    if (!colon){
                        goto failed_incorrect;
                    }
                    *colon = 0;
                    if (strlen(p) > 128) {
                        goto failed_incorrect;
                    }
                    password = p;
                    p = colon +1;
                    if (remote_code == MODE_REGISTER) {
                        comfirmpass = p;
                    }
                    else {
                        colon = strchr(p,':');
                        if (!colon){
                            goto failed_incorrect;
                        }
                        *colon = 0;
                        if (strlen(p) > 128) {
                            goto failed_incorrect;
                        }
                        resetpass = p;
                        p = colon +1;
                        comfirmpass = p;
                    }
                }
                char* pack_struct = pack_packet(remote_code,username,password,resetpass,comfirmpass);
                char * recvdata = send_cmd(remote_code,512,pack_struct);
        
                if (recvdata) {
                    if (!strcmp(recvdata,"success")) {
                        char pack_struct[257];
                        memset(pack_struct,0,64);
                        memcpy(pack_struct,id,24);
                        memcpy(pack_struct+24,"/cgi-bin/manager.cgi",20);
                        generate_random_cookie(pack_struct+64, 64);
                        memcpy(pack_struct+128,username,128);
                        recvdata = send_cmd(MODE_ADD_SESSION,256,pack_struct);
                        send_redirect_Cookie(pack_struct+64,"/cgi-bin/manager.cgi",id);
                    }
                    else if(!strcmp(recvdata,"incorrect_password")){
                        location = strcat(location,"?error=incorrect_password");
                        send_redirect(location);

                    }
                    else if (!strcmp(recvdata,"incorrect_username")) {
                        location = strcat(location,"?error=incorrect_username");
                        send_redirect(location);
                    }
                    else {
                        goto failed_incorrect;
                    }
                }
                else {
                    goto failed_incorrect;
                }

            }
        }
    }
    return 0;
}