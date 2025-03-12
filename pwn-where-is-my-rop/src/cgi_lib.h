#ifndef cgi_LIB_H
#define cgi_LIB_H

#include <stdint.h>
#include <stdbool.h>


// 安全校验函数
bool isSafeCommandArg(char *arg);
bool isSafeFileName(char *path);

// 通信函数
char *send_cmd(int a, int b,char *c);

#endif