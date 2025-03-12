#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sqlite3.h>
#include <stdbool.h>

#define SOCKET_PATH "/tmp/tpctf.sock"
#define MAX_CLIENTS 5
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

// Handler函数示例
void handle_login(int  len, const void *data,char **resp) {
    // 0-128:username
    // 128-256:password
    char *username = malloc(129);
    char *password = malloc(129);
    memcpy(username, data, 128);
    memcpy(password, data+128, 128);
    if (username && password) {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc = sqlite3_open("/root/login.db", &db);
        if (rc) {
            sqlite3_close(db);
            strcpy(*resp, "Failed to open database");
            goto login_final;
        }
        char *buffer = malloc(strlen(username)+128);
        sqlite3_snprintf(strlen(username)+128, buffer, "SELECT password FROM users WHERE username='%q'", username);

        
        rc = sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            strcpy(*resp, "Failed to prepare statement");
            goto login_final;
        }

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            const unsigned char *db_password = sqlite3_column_text(stmt, 0);
            if (!strcmp((const char*)db_password, password)) {
                strcpy(*resp, "success");
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                goto login_final;
            } else {
                strcpy(*resp, "incorrect_password");
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                goto login_final;
            }
        } else {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            strcpy(*resp, "incorrect_username");
            goto login_final;
        }
    } else {
        strcpy(*resp, "Invalid data");
        goto login_final;
    }  
    login_final:
    free(username);
    free(password);
    return; 

}

void handle_reset_password(int  len, const void *data,char **resp) {
    // 0-128:username
    // 128-256:password
    // 256-384:resetpass
    // 384-512:comfirmpass
    char *username = malloc(129);
    char *password = malloc(129);
    char *resetpass = malloc(129);
    char *comfirmpass = malloc(129);
    memcpy(username, data, 128);
    memcpy(password, data+128, 128);
    memcpy(resetpass, data+256, 128);
    memcpy(comfirmpass, data+384, 128);
    if (username && password && resetpass && comfirmpass) {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc = sqlite3_open("/root/login.db", &db);
        if (rc) {
            sqlite3_close(db);
            strcpy(*resp, "Failed to open database");
            goto reset_final;
        }
        char *buffer = malloc(strlen(username)+128);
        sqlite3_snprintf(strlen(username)+128, buffer, "SELECT password FROM users WHERE username='%q'", username);

        
        rc = sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            strcpy(*resp, "Failed to prepare statement");
            goto reset_final;
        }

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            const unsigned char *db_password = sqlite3_column_text(stmt, 0);
            if (strcmp((const char*)db_password, password) == 0) {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                if (strcmp(resetpass, comfirmpass) == 0) {
                    sqlite3 *db;
                    sqlite3_stmt *stmt;
                    int rc = sqlite3_open("/root/login.db", &db);
                    if (rc) {
                        sqlite3_close(db);
                        strcpy(*resp, "Failed to open database");
                        goto reset_final;
                    }
                    char *buffer = malloc(strlen(username)+128);
                    sqlite3_snprintf(strlen(username)+128, buffer, "UPDATE users SET password='%q' WHERE username='%q'", resetpass, username);

                    
                    rc = sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
                        sqlite3_close(db);
                        strcpy(*resp, "Failed to prepare statement");
                        goto reset_final;
                    }

                    rc = sqlite3_step(stmt);
                    if (rc == SQLITE_DONE) {
                        sqlite3_finalize(stmt);
                        sqlite3_close(db);
                        strcpy(*resp, "success");
                        goto reset_final;
                    } else {
                        sqlite3_finalize(stmt);
                        sqlite3_close(db);
                        strcpy(*
                        resp, "Failed to update password");
                        goto reset_final;
                    }
                } else {
                    strcpy(*resp, "password_mismatch");
                    goto reset_final;
                }
            } else {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                strcpy(*resp, "incorrect_password");
                goto reset_final;
            }
        } else {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            strcpy(*resp, "incorrect_username");
            goto reset_final;
        }
    } else {
        strcpy(*resp, "Invalid data");
        goto reset_final;
    }
    reset_final:
    free(username);
    free(password);
    free(resetpass);
    free(comfirmpass);
    return;
}

void handle_register(int  len, const void *data,char **resp) {
    // 0-128:username
    // 128-256:password
    // 256-384:comfirmpass
    char *username = malloc(129);
    char *password = malloc(129);
    char *comfirmpass = malloc(129);
    memcpy(username, data, 128);
    memcpy(password, data+128, 128);
    memcpy(comfirmpass, data+256, 128);
    if (username && password && comfirmpass) {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc = sqlite3_open("/root/login.db", &db);
        if (rc) {
            sqlite3_close(db);
            strcpy(*resp, "Failed to open database");
            goto register_final;
        }
        char *buffer = malloc(strlen(username)+128);
        sqlite3_snprintf(strlen(username)+128, buffer, "SELECT password FROM users WHERE username='%q'", username);

        
        rc = sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            strcpy(*resp, "Failed to prepare statement");
            goto register_final;
        }

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            strcpy(*resp, "username_exists");
            goto register_final;
        } else {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            if (strcmp(password, comfirmpass) == 0) {
                sqlite3 *db;
                sqlite3_stmt *stmt;
                int rc = sqlite3_open("/root/login.db", &db);
                if (rc) {
                    sqlite3_close(db);
                    strcpy(*resp, "Failed to open database");
                    goto register_final;
                }
                char *buffer = malloc(strlen(username)+strlen(password)+128);
                sqlite3_snprintf(strlen(username)+strlen(password)+128, buffer, "INSERT INTO users (username, password) VALUES ('%q', '%q')", username, password);

                
                rc = sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
                if (rc != SQLITE_OK) {
                    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                    strcpy(*resp, "Failed to prepare statement");
                    goto register_final;
                }

                rc = sqlite3_step(stmt);
                if (rc == SQLITE_DONE) {
                    sqlite3_finalize(stmt);
                    sqlite3_close(db);
                    strcpy(*resp, "success");
                    goto register_final;
                } else {
                    sqlite3_finalize(stmt);
                    sqlite3_close(db);
                    strcpy(*resp, "Failed to insert user");
                    goto register_final;
                }
            } else {
                strcpy(*resp, "password_mismatch");
                goto register_final;
            }
        }
    } else {
        strcpy(*resp, "Invalid data");
        goto register_final;
    }
    register_final:
    free(username);
    free(password);
    free(comfirmpass);
    return;
}

void handle_add_session(int  len, const void *data,char **resp) {
    // 0-24：session_id
    // 24-64: redirect_url
    // 64-128: cookie_value
    // 128-256: username
    char *session_id = malloc(25);
    char *redirect_url = malloc(41);
    char *cookie_value = malloc(65);
    char *username = malloc(129);
    memcpy(session_id, data, 24);
    memcpy(redirect_url, data+24, 40);
    memcpy(cookie_value, data+64, 64);
    memcpy(username, data+128, 128);
    if (session_id && redirect_url && cookie_value) {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc = sqlite3_open("/root/login.db", &db);
        if (rc) {
            sqlite3_close(db);
            strcpy(*resp, "Failed to open database");
            goto add_session_final;
        }
        char *buffer = malloc(strlen(session_id)+64);
        sqlite3_snprintf(strlen(session_id)+64, buffer, "INSERT INTO sessions (session_id, cookie, username) VALUES ('%q', '%q', '%q')", session_id, cookie_value,username);

        
        rc = sqlite3_prepare_v2(db, buffer, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            strcpy(*resp, "Failed to prepare statement");
            goto add_session_final;
        }

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            strcpy(*resp, "success");
            goto add_session_final;
        } else {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            strcpy(*resp, "Failed to insert session");
            goto add_session_final;
        }
    } else {
        strcpy(*resp, "Invalid data");
        goto add_session_final;
    }
    add_session_final:
    free(session_id);
    free(redirect_url);
    free(cookie_value);
    free(username);
    return;

}

void handle_del_session(int  len, const void *data,char **resp) {
    // 0-24：session_id
    // 24-88: cookie
    char *session_id = malloc(25);
    char *cookie = malloc(65);
    memcpy(session_id, data, 24);
    memcpy(cookie, data+24, 64);
    if (session_id && cookie) {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc = sqlite3_open("/root/login.db", &db);
        if (rc) {
            sqlite3_close(db);
            strcpy(*resp, "Failed to open database");
            goto del_session_final;
        }
        char *buffer = malloc(strlen(session_id)+64);
        sqlite3_snprintf(strlen(session_id)+64, buffer, "DELETE from sessions where session_id='%q' and cookie='%q';", session_id, cookie);

        rc = sqlite3_exec(db, buffer, 0, 0, NULL);
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            strcpy(*resp, "Failed to delete session");
            goto del_session_final;
        }
        sqlite3_close(db);
        strcpy(*resp, "success");
    } else {
        strcpy(*resp, "Invalid data");
        goto del_session_final;
    }
    del_session_final:
    free(session_id);
    free(cookie);
    return;
}

void handle_iperfserver(int  len, const void *data,char **resp) {
    // 0-128:ip
    // 128-256:port
    char *ip = malloc(129);
    char *port = malloc(129);
    memcpy(ip, data, 128);
    memcpy(port, data+128, 128);
    if (ip && port) {
        if (!isSafeCommandArg(ip) || !isSafeCommandArg(port)) {
            strcpy(*resp, "Invalid data");
            goto iperfserver_final;
        }
        char *cmd = malloc(257);
        snprintf(cmd, 256, "iperf3 -s -B %s -p %s &", ip, port);
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            strcpy(*resp, "Failed to start iperf server");
            goto iperfserver_final;
        }
        pclose(fp);
        strcpy(*resp, "success");
    } else {
        strcpy(*resp, "Invalid data");
        goto iperfserver_final;
    }
    iperfserver_final:
    free(ip);
    free(port);
    return;
}

void handle_ping(int  len, const void *data,char **resp) {
    // 0-128:ip
    char *ip = malloc(129);
    memcpy(ip, data, 128);
    if (ip) {
        if (!isSafeCommandArg(ip)) {
            strcpy(*resp, "Invalid data");
            goto ping_final;
        }
        char *cmd = malloc(257);
        snprintf(cmd, 256, "ping -c 4 %s", ip);
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            strcpy(*resp, "Failed to start ping");
            goto ping_final;
        }
        char *buffer = malloc(4097);
        fread(buffer, 1, 4096, fp);
        pclose(fp);
        strcpy(*resp, buffer);
    } else {
        strcpy(*resp, "Invalid data");
        goto ping_final;
    }
    ping_final:
    free(ip);
    return;
}

void handle_traceroute(int  len, const void *data,char **resp) {
    // 0-128:ip
    char *ip = malloc(129);
    memcpy(ip, data, 128);
    if (ip) {
        if (!isSafeCommandArg(ip)) {
            strcpy(*resp, "Invalid data");
            goto traceroute_final;
        }
        char *cmd = malloc(257);
        snprintf(cmd, 256, "traceroute %s ", ip);
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            strcpy(*resp, "Failed to start traceroute");
            goto traceroute_final;
        }
        char *buffer = malloc(4097);
        fread(buffer, 1, 4096, fp);
        pclose(fp);
        strcpy(*resp, buffer);
    } else {
        strcpy(*resp, "Invalid data");
        goto traceroute_final;
    }
    traceroute_final:
    free(ip);
    return;
}

void handle_iperfclient(int  len, const void *data,char **resp) {
    // 0-128:ip
    // 128-256:port
    char *ip = malloc(129);
    char *port = malloc(129);
    memcpy(ip, data, 128);
    memcpy(port, data+128, 128);
    if (ip && port) {
        if (!isSafeCommandArg(ip) || !isSafeCommandArg(port)) {
            strcpy(*resp, "Invalid data");
            goto iperfclient_final;
        }
        char *cmd = malloc(257);
        snprintf(cmd, 256, "iperf3 -c %s -p %s", ip, port);
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            strcpy(*resp, "Failed to start iperf client");
            goto iperfclient_final;
        }
        char *buffer = malloc(4097);
        fread(buffer, 1, 4096, fp);
        pclose(fp);
        strcpy(*resp, buffer);
    } else {
        strcpy(*resp, "Invalid data");
        goto iperfclient_final;
    }
    iperfclient_final:
    free(ip);
    free(port);
    return;
}

void handle_gen_cert(int  len, const void *data,char **resp){
    //use acme.sh
    ///root/acme.sh
    //0-128:domain
    char *domain = malloc(129);
    memcpy(domain, data, 128);
    if (domain) {
        int result = 1;
        char command[1024] = {0};
        char certDir[128] = {0};
        char logLine[1024] = {0};
        FILE *fp = NULL;
        struct stat stat_buf;
        int certNumber = 1;
        bool certSuccess = false;
        if (!isSafeCommandArg(domain) ) {
            strcpy(*resp, "Invalid data");
            goto gen_cert_final;
        }
    
        // 构造并执行证书申请命令
        snprintf(command, sizeof(command),
                 "env CURL_CA_BUNDLE=/tmp/ca-bundle.crt HOME=/root /root/.acme.sh/acme.sh "
                 "--issue --server letsencrypt --force --log /tmp/acme.sh.log -d %s -w /var/www/htdocs",
                 domain);
        system(command);
    
        // 打开日志文件等待证书签发成功信息
        fp = fopen("/tmp/acme.sh.log", "r");
        if (fp == NULL) {
            strcpy(*resp, "Failed to open log file");
            goto gen_cert_cleanup;
        }
    
        while (fgets(logLine, sizeof(logLine), fp)) {
            if (strstr(logLine, "Cert success") != NULL) {
                certSuccess = true;
                break;
            }
        }
        fclose(fp);
        fp = NULL;
    
        if (!certSuccess) {
            strcpy(*resp, "Failed to issue certificate");
            goto gen_cert_cleanup;
        }
    
        // 查找可用的证书目录（例如 newcert-1 到 newcert-65）
        for (certNumber = 1; certNumber < 66; certNumber++) {
            snprintf(certDir, sizeof(certDir), "/var/cert/newcert-%d", certNumber);
            if (stat(certDir, &stat_buf) != 0) {
                // 目录不存在，使用该目录
                break;
            }
            if (certNumber == 65) {
                strcpy(*resp, "No available certificate directory");
                goto gen_cert_cleanup;
            }
        }
    
        // 创建证书目录
        if (mkdir(certDir, 0777) != 0) {
            strcpy(*resp, "Failed to create certificate directory");
            goto gen_cert_cleanup;
        }
    

        snprintf(command, sizeof(command),
                 "cp -rpf /root/.acme.sh/%s %s/", domain, certDir);
        system(command);
        strcpy(*resp, "success");
    
        // 拷贝证书文件到目标目录
        gen_cert_cleanup:
        snprintf(command, sizeof(command),
                 "cat /tmp/acme.sh.log >> /var/logs/acme.sh.log");
        system(command);
        remove("/tmp/acme.sh.log");
    
    } else {
        strcpy(*resp, "Invalid data");
        goto gen_cert_final;
    }
    gen_cert_final:
    free(domain);
    return;
}

void handle_renew_cert(int  len, const void *data,char **resp){
    //use acme.sh
    //0-128:domain
    //128-256: dir
    char *domain = malloc(129);
    char *dir = malloc(129);
    memcpy(domain, data, 128);
    memcpy(dir, data+128, 128);
    if (domain) {
        char command[1024] = {0};
        char buffer[1024] = {0};
        FILE *fp = NULL;
        bool renewSuccess = false;
    
        // 参数安全性检查
        if (!isSafeCommandArg(domain) || !isSafeCommandArg(dir)) {
            strcpy(*resp, "Invalid data");
            goto renew_cert_final;
        }
    
        // 调用 acme.sh 执行续期，日志输出到 /tmp/acme.sh.log
        snprintf(command, sizeof(command),
                 "env CURL_CA_BUNDLE=/tmp/ca-bundle.crt HOME=/root /root/.acme.sh/acme.sh "
                 "--renew --server letsencrypt --force --log /tmp/acme.sh.log -d %s",
                 domain);
        system(command);
    
        // 打开日志文件，判断续期是否成功（查找 _on_issue_success）
        fp = fopen("/tmp/acme.sh.log", "r");
        if (fp) {
            while (fgets(buffer, sizeof(buffer), fp)) {
                if (strstr(buffer, "_on_issue_success")) {
                    renewSuccess = true;
                    break;
                }
            }
            fclose(fp);
        }
    
        if (!renewSuccess) {
            strcpy(*resp, "Failed to renew certificate");
            goto renew_cleanup;
        }
    
        // 复制续期后的证书到指定目录 /var/cert/<dir>/
        snprintf(command, sizeof(command),
                 "cp -rpf /root/.acme.sh/%s /var/cert/%s/",
                 domain, dir);
        system(command);
    
    
        // 标记续期成功
        strcpy(*resp, "success");
    
        renew_cleanup:
        // 归档日志并清理临时日志文件
        snprintf(command, sizeof(command),
                 "cat /tmp/acme.sh.log >> /var/logs/acme.sh.log");
        system(command);
        remove("/tmp/acme.sh.log");
    }
    renew_cert_final:
    free(domain);
    return;
}

void handle_revoke_cert(int  len, const void *data,char **resp){
    //use acme.sh
    //0-128:domain
    char *domain = malloc(129);
    memcpy(domain, data, 128);
    if (domain) {
        if (!isSafeCommandArg(domain)) {
            strcpy(*resp, "Invalid data");
            goto revoke_cert_final;
        }
    
        char command[1024];
        snprintf(command, sizeof(command),
                 "env CURL_CA_BUNDLE=/tmp/ca-bundle.crt HOME=/root /root/.acme.sh/acme.sh --revoke --log /tmp/acme.sh.log -d %s",
                 domain);
    
        if (system(command) != 0) {
            strcpy(*resp, "Failed to revoke certificate");
            goto revoke_cert_final;
        }
    
        FILE *stream = fopen("/tmp/acme.sh.log", "r");
        if (!stream) {
            strcpy(*resp, "Failed to open log file");
            goto revoke_cert_final;
        }
    
        char line[1024];
        int revoke_success = 0;
        while (fgets(line, sizeof(line), stream)) {
            if (strstr(line, "Revoke success.")) {
                revoke_success = 1;
                break;
            }
        }
        fclose(stream);
    
        if (revoke_success) {
            strcpy(*resp, "success");
        } else {
            strcpy(*resp, "Failed to revoke certificate");
        }
    
        snprintf(command, sizeof(command), "cat /tmp/acme.sh.log >> %s", "/var/logs/acme.sh.log");
        system(command);
        remove("/tmp/acme.sh.log");
    }
    revoke_cert_final:
    free(domain);
    return;
}



int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char c[4097];

    // 创建 UNIX 域流式套接字
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(1);
    }

    // 如果之前存在该 socket 文件，先删除它
    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    // 绑定 socket 到指定地址
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        close(server_fd);
        exit(1);
    }

    // 修改 socket 文件权限，允许所有用户访问（666 权限）
    if (chmod(SOCKET_PATH, 0666) == -1) {
        perror("chmod error");
        close(server_fd);
        exit(1);
    }

    // 开始监听连接
    if (listen(server_fd, 5) == -1) {
        perror("listen error");
        close(server_fd);
        exit(1);
    }


    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        int a;
        int  b;

        read(client_fd, &a, sizeof(int));
        read(client_fd, &b, sizeof(int ));
        read(client_fd, c, b);
        char *resp  = malloc(4097);

        // 根据调用号选择Handler
        switch (a) {
            case MODE_LOGIN: handle_login(b, c,&resp); break;
            case MODE_RESET_PASSWORD: handle_reset_password(b, c,&resp); break;
            case MODE_REGISTER: handle_register(b, c,&resp); break;
            case MODE_ADD_SESSION: handle_add_session(b, c,&resp); break;
            case MODE_DEL_SESSION: handle_del_session(b, c,&resp); break;
            case MODE_IPERFSERVER: handle_iperfserver(b, c,&resp); break;
            case MODE_PING: handle_ping(b, c,&resp); break;
            case MODE_TRACEROUTE: handle_traceroute(b, c,&resp); break;
            case MODE_IPERFCLIENT: handle_iperfclient(b, c,&resp); break;
            case MODE_GEN_CERT: handle_gen_cert(b, c,&resp); break;
            case MODE_RENEW_CERT: handle_renew_cert(b, c,&resp); break;
            case MODE_REVOKE_CERT: handle_revoke_cert(b, c,&resp); break;
            default: strcpy(resp, "Unknown command");
        }
        if (strlen(resp) == 0) {
            strcpy(resp,"ACK");
        }
        int send_len = strlen(resp);
        write(client_fd, &send_len, sizeof(int));
        write(client_fd, resp, strlen(resp));
        close(client_fd);
    }
    return 0;
}