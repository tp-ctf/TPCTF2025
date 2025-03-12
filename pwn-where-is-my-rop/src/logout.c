#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgi_lib.h"
enum {
    MODE_LOGIN = 1,
    MODE_RESET_PASSWORD = 2,
    MODE_REGISTER = 3,
    MODE_ADD_SESSION = 4,
    MODE_DEL_SESSION = 5,
    MODE_IPERFSERVER = 6,
    MODE_PING = 7,
    MODE_TRACEROUTE = 8,
    MODE_IPERFCLIENT = 9,
    MODE_GEN_CERT = 10,
    MODE_RENEW_CERT = 11,
    MODE_REVOKE_CERT = 12
};

int main() {
    char * sessionID;
    char * sessionID_start;
    char* sessionID_end;
    char * CookieStr;
    char* cookie;
    char *cookie_start;
    char* cookie_end;
    CookieStr = getenv("HTTP_COOKIE");
    // if (CookieStr == NULL || strlen(CookieStr) == 0) {
    //     goto loginfailed;
    // }
    //CookieStr = "session_id=416f3235417335766475493d; Cookie=VFf7xxD1Na94D1s2R31qIJRT2WyIwx5H0aFnMGeXgbRkS7aj1PP9YfROzFWl0Rso";
    //CookieStr = "Cookie=xcEb4n9Ad90LRYi3PNxkeyZlm5fPMRP9uTbm4YmYXahmyDgCQRmsF9Ur5x7FDKFv; session_id=7251354753616f767a7a593d";
    sessionID_start = strstr(CookieStr, "session_id=");
    if (sessionID_start == NULL) {
        loginfailed:;
        printf("Content-type: text/html\n\n");
        printf("<html><head><meta http-equiv=\"refresh\" content=\"0;url=/login.html\"></head></html>");
        return 0;
    }
    sessionID_start += 11;
    sessionID_end = strstr(sessionID_start, ";");
    if (sessionID_end == NULL) {
        sessionID_end = sessionID_start + strlen(sessionID_start);
    }
    sessionID = malloc(sessionID_end - sessionID_start + 1);
    memcpy(sessionID, sessionID_start, sessionID_end - sessionID_start);
    


    cookie_start = strstr(CookieStr, "Cookie=");
    if (cookie_start == NULL) {
        goto loginfailed;
    }
    cookie_start += 7;
    cookie_end = strstr(cookie_start, ";");
    if (cookie_end == NULL) {
        cookie_end = cookie_start + strlen(cookie_start);
    }
    cookie = malloc(cookie_end - cookie_start + 1);
    memcpy(cookie, cookie_start, cookie_end - cookie_start);
    if (strlen(sessionID) != 24 || strlen(cookie) != 64) {
        goto loginfailed;
    }

    char *resp;
    char pack_struct[129];
    memset(pack_struct,0,129);
    memcpy(pack_struct,sessionID,24);
    memcpy(pack_struct+24,cookie,64);
    resp = send_cmd(5,128,pack_struct);

    printf("Status: 302 Found\n");
    printf("Location: /index.html\n");
    //printf("Status: 200 OK\n");
    //printf("X-Redirect-URL: /index.html\n\n");
    printf("Content-Type: text/html\n\n"); // 双换行结束头信息
    printf("<html><body>\n");
    printf("<h1>Redirecting to <a href=\"/\">/</a></h1>\n");
    printf("</body></html>\n");

}