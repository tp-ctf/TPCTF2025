#include "cgi_lib.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdlib.h>

#define SOCKET_PATH "/tmp/tpctf.sock"

bool isSafeCommandArg(char *a1) {
    int result; // eax
    char v2; // dl
    char *v3; // eax
  
    result = 0;
    if ( a1 )
    {
      v2 = *a1;
      if ( *a1 )
      {
        v3 = a1;
        while ( 1 )
        {
          if ( v2 > 60 )
          {
            if ( v2 == 94 )
              return 0;
            if ( v2 > 94 )
            {
              if ( v2 == 96 )
                return 0;
              if ( v2 == 124 )
                return 0;
            }
            else if ( v2 == 62 )
            {
              return 0;
            }
          }
          else if ( v2 >= 59 || v2 == 36 || v2 == 38 || v2 == 10 )
          {
            return 0;
          }
          v2 = *++v3;
          if ( !*v3 )
            return 1;
        }
      }
    }
    return result;
}

// 通信实现
char* send_cmd(int a, int b, char *c) {
    struct sockaddr_un addr;
    char *buffer = (char*)malloc(4097);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        return NULL;
    }

    // 发送协议：调用号(a) + 数据长度(b) + 数据内容(c)
    write(fd, &a, sizeof(int));
    write(fd, &b, sizeof(int));
    write(fd, c, b);
    int len;
    read(fd, &len, sizeof(int));
    read(fd, buffer, len);
    close(fd);
    if (len <= 0) return NULL;
    buffer[len] = '\0';
    return buffer;
}
