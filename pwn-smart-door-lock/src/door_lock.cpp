#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdarg>
#include <cjson/cJSON.h>
#include "door_lock.h"
char *session_nums = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

/*
{
    "session": "a1b2c3d4e5",
    "request": "add_finger",
    "req_args": [
        "john_doe",
        "password123",
    ]
}
*/
bool parse_json(char *payload, char **session, char **request, char **req_args) {
    if (payload == nullptr) return false;

    
    cJSON *json = cJSON_Parse(payload);
    if (json == nullptr) {
        return false;
    }
    
    cJSON *session_item = cJSON_GetObjectItemCaseSensitive(json, "session");
    cJSON *request_item = cJSON_GetObjectItemCaseSensitive(json, "request");
    cJSON *req_args_item = cJSON_GetObjectItemCaseSensitive(json, "req_args");
    
    if (!cJSON_IsString(session_item) || session_item->valuestring == nullptr ||
        !cJSON_IsString(request_item) || request_item->valuestring == nullptr) {
        cJSON_Delete(json);
        return false;
    }
    
    // 深拷贝字符串，防止 JSON 对象释放后数据丢失
    *session = strdup(session_item->valuestring);
    *request = strdup(request_item->valuestring);
    
    if (!cJSON_IsArray(req_args_item)) {
        // 清理已分配的内存
        free(*session);
        free(*request);
        cJSON_Delete(json);
        return false;
    }
    
    int index = 0;
    cJSON *arg = nullptr;
    cJSON_ArrayForEach(arg, req_args_item) {
        if (cJSON_IsString(arg) && arg->valuestring != nullptr) {
            req_args[index++] = strdup(arg->valuestring);
            if (index >= 2) { // 仅提取前两个参数
                break;
            }
        } else {
            // 清理已分配内存
            free(*session);
            free(*request);
            if (req_args[0]) free(req_args[0]);
            if (req_args[1]) free(req_args[1]);
            cJSON_Delete(json);
            return false;
        }
    }
    
    // 如果要求必须有两个参数，可检查 index 是否等于2
    // if(index != 2) { ... }
    
    cJSON_Delete(json);
    return true;
}

std::string current_timestamp() {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    // 转换为 time_t 类型
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    // 将 time_t 转换为结构体 tm（本地时间）
    std::tm *local_time = std::localtime(&now_time);
    // 使用 stringstream 和 put_time 格式化时间
    std::stringstream ss;
    ss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

unsigned int * change_finger_format(char* finger) {

    //finger format: "[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]"
    unsigned int *new_finger = (unsigned int *)malloc(20*sizeof(unsigned int));
    if (finger[0] != '[' ||finger[strlen(finger)-1] != ']') {
        free(new_finger);
        return NULL;
    }
    //处理掉finger的逗号分隔，并录入其中的数字，最多录入20个数字
    char* p = finger + 1;
    int i = 0;
    char* cur_num = p;
    while (*p != ']' && i < 20) {
        if (*p == ',') {
            *p=0;
            new_finger[i++] = (unsigned int)atoi(cur_num);
            cur_num = p+1;
        }
        else if (*p >'9' || *p <'0')
        {
            free(new_finger);
            return NULL;
        }
        p++;
    }
    if (i < 20) {
        *p = 0;
        new_finger[i++] = (unsigned int)atoi(cur_num);
    }
    if(i == 20)
    {
        return new_finger;
    }
    else {
        free(new_finger);
        return NULL;
    }
}

mqtt_lock::mqtt_lock(const char *id, const char *host, int port) : mosqpp::mosquittopp(id)
{
    /* set connection */
	int keepalive = 60;
	tls_opts_set(1,"tlsv1",NULL);
	tls_set("/etc/mosquitto/certs/ca.crt",NULL,NULL,NULL,NULL);
	tls_insecure_set(true);
	connect(host, port, keepalive);

    /* inital session & token */
    session_id = NULL;
    auth_token = NULL;
    
    /* set lock inital */
    lock_door();
    /* open logger create read write */
    strcpy(log_file,"/etc/mosquitto/smart_lock.log");
    logger = fopen(log_file, "w+");
    if (logger == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    int status = log("logger created:%s\n",log_file);

    /* read fingers */ 
    FILE* finger_file = fopen("/etc/mosquitto/fingers_credit","r");
    if (finger_file == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    char line[512];
    fingers *finger_pos = NULL;
    max_finger_id = 1;
    while (fgets(line, sizeof(line), finger_file)) {
        line[strcspn(line, "\n")] = 0;
        struct fingers *new_finger = (struct fingers*)malloc(sizeof(struct fingers));
        new_finger->finger_id = max_finger_id++;
        new_finger->next = NULL;
        new_finger->retry_count = 0;

        if (new_finger == NULL) {
            log("Error allocating memory!\n");
            exit(1);
        }
        if (finger_list == NULL)
        {
            finger_list = new_finger;
            finger_pos = new_finger;
        } else {
            finger_pos->next = new_finger;
            finger_pos = new_finger;
        }
        if( edit_finger(new_finger,(char*)line)){
            continue;
        }
        else {
            free(new_finger);
            continue;
        }
    }
    fclose(finger_file);

    /* inital subscribe*/
    subscribe(NULL, "auth_token");
    subscribe(NULL, "manager");
    subscribe(NULL, "logger");
};

mqtt_lock::~mqtt_lock()
{
    fclose(logger);
    struct fingers *cur_finger = finger_list;
    while (cur_finger != NULL) {
        struct fingers *temp = cur_finger;
        cur_finger = cur_finger->next;
        free(temp);
    }
}


bool mqtt_lock::add_finger(char*finger_str) {
    struct fingers *new_finger = (struct fingers*)malloc(sizeof(struct fingers));
    if (new_finger == NULL) {
        log("Error allocating memory!\n");
        return false;
    }
    new_finger->finger_id = max_finger_id++;
    new_finger->next = NULL;
    new_finger->retry_count = 0;

    if (finger_list == NULL)
    {
        finger_list = new_finger;
    } else {
        fingers *cur_finger = finger_list;
        while (cur_finger->next != NULL) {
            cur_finger = cur_finger->next;
        }
        cur_finger->next = new_finger;
    }
    if ( edit_finger(new_finger,finger_str)){
        return true;
    }
    else {
        free(new_finger);
        return false;
    }
}

bool mqtt_lock::edit_finger(fingers* cur_finger,char *finger_str) 
{
    unsigned int *format_finger = change_finger_format(finger_str);
    if (!format_finger) {
        return false;
    }
    for (int i = 0; i < 20; i++) {
        cur_finger->finger[i] = format_finger[i];
    }
    free(format_finger);
    return true;
}

bool mqtt_lock::remove_finger(unsigned int finger_id) {
    if (finger_list == NULL) {
        return false;
    }
    if (finger_list->finger_id == finger_id) {
        if (finger_list->next == NULL) {
            return false;
        }
        struct fingers *temp = finger_list;
        finger_list = finger_list->next;
        free(temp);
        return true;
    }
    struct fingers *cur_finger = finger_list;
    while (cur_finger->next != NULL) {
        if (cur_finger->next->finger_id == finger_id) {
            struct fingers *temp = cur_finger->next;
            cur_finger->next = cur_finger->next->next;
            free(temp);
            return true;
        }
        cur_finger = cur_finger->next;
    }
    return false;
}

bool mqtt_lock::check_finger(fingers* finger,char* finger_str) {
    unsigned int *format_finger = change_finger_format(finger_str);
    if (!format_finger) {
        return false;
    }
    // check finger
    // 对每一个finger计算相似度，相似度达到90%以上认为匹配成功
    // 计算相似度的方法为对于20个整数中的每一个整数，统计与之相应位置的指纹小的数除以大的数之后的数，对这些数进行加权平均值，如果大于0.9则认为匹配成功
    float similarity = 0;
    for (int i = 0; i < 20; i++) {
        if (finger->finger[i] > format_finger[i]) {
            similarity += (float)format_finger[i]/(float)finger->finger[i];
        } else {
            similarity += (float)finger->finger[i]/(float)format_finger[i];
        }
    }
    similarity /= 20.0;
    similarity*=100.0;

    free(format_finger);
    log("finger_id: %d,finger similarity:%%%.9f\n",finger->finger_id,similarity);
    return similarity >= 90.0;
}

bool mqtt_lock::log(const char *fmt,...) {
	va_list va;
	char *s;
	size_t len;
    if (!logger) {
        return false;
    }
	len = strlen(fmt) + 500;
	s = (char*)malloc(len*sizeof(char));
    if(!s){
        return false;
    }
    va_start(va, fmt);
    vsnprintf(s, len, fmt, va);
    va_end(va);
	s[len-1] = '\0'; /* Ensure string is null terminated. */
    fwrite(s, sizeof(char), strlen(s), logger);
	free(s);    
    fflush(logger);
    return true;
}

bool mqtt_lock::download_log() {
    FILE *read_log_file = fopen(log_file, "r");
    if (read_log_file == NULL) {
        log("Error opening file!\n");
        return false;
    }
    char line[512];
    while (fgets(line, sizeof(line), read_log_file)) {
        publish(NULL, "logfile", strlen(line), line);
    }
    publish(NULL, "logfile",4,"\nEOF");
    fclose(read_log_file);
    return true;
}

bool mqtt_lock::clear_log() {
    fclose(logger);
    logger = fopen(log_file, "w+");
    if (logger == NULL) {
        log("Error opening file!\n");
        return false;
    }
    publish(NULL,"logfile",15,"logger cleared\n");
    return true;
}

bool mqtt_lock::lock_door() {
    lock_status.lock = true;
    lock_status.timestamp = current_timestamp();
    log("door locked:%s\n",lock_status.timestamp.c_str());
    return true;
}

bool mqtt_lock::unlock_door() {
    lock_status.lock = false;
    lock_status.timestamp = current_timestamp();
    log("door unlocked:%s\n",lock_status.timestamp.c_str());
    return true;
}

void mqtt_lock::on_message(const struct mosquitto_message *message)
{

	if(!strcmp(message->topic, "auth_token")){
		if (auth_token) {
            unsubscribe(NULL, auth_token);
            // log("close subncribe:%s\n",auth_token);
            free(auth_token);
        }
        auth_token = (char*)malloc(0x11);
        char * payload = (char*)message->payload;
        for (int i = 0; i<0x10;i++) {
            if ((payload[i] <= '9' && payload[i] >= '0') || (payload[i] <= 'Z' && payload[i] >= 'A') || (payload[i] <= 'z' && payload[i] >= 'a')) {
                auth_token[i] = payload[i];
            } else {
                log("auth_token error: token must be num or letter\n");
                free(auth_token);
                auth_token = NULL;
                return;
            }
        }
        auth_token[0x10] = 0;
        log("auth_token:%s\n",auth_token);
        char re_auth_token[20];
        snprintf(re_auth_token, 20, "re_%s", auth_token);

        subscribe(NULL, auth_token);
        
        publish(NULL, re_auth_token, 11, "finger tap\n");
        // log("open subncribe:%s\n",auth_token);
        
        return;

	}
    else if(!strcmp(message->topic, "manager")) {
        /*
        {
            "session": "a1b2c3d4e5",
            "request": "add_finger",
            "req_args": [
                "john_doe",
                "password123",
            ]
        }*/
        // add_finger edit_finger remove_finger lock_door unlock_door
        char *payload = (char*)message->payload;
        char *session = nullptr;
        char *request = nullptr;
        char *req_args[2] = {nullptr, nullptr};
        bool paese_res = parse_json(payload, &session, &request, req_args);
        if (!paese_res) {
            log("json parse error\n");
            return;
        }
        if (!session_id || strcmp(session,session_id)) {
            log("session id mismatch\n");
            goto END;
        }
        char output[1024];
        if (!strcmp(request,"add_finger")) {
            if (req_args[0] && req_args[0][0]== '[' && req_args[0][strlen(req_args[0])-1] == ']') {
                if (add_finger(req_args[0])) {
                    snprintf(output,1024,"new finger id:%d\n",max_finger_id-1);
                    publish(NULL,session_id,strlen(output),output);
                    goto END;
                } 
            }
            snprintf(output,1024,"add finger failed\n");
            publish(NULL,session_id,strlen(output),output);
            goto END;
        }
        else if (!strcmp(request,"edit_finger")) {
            if(!req_args[0] || !req_args[1]) {
                publish(NULL,session_id,19,"edit finger failed\n");
                goto END;
            }
            if (req_args[1][0] != '[' || req_args[1][strlen(req_args[1])-1] != ']') {
                publish(NULL,session_id,19,"edit finger failed\n");
                goto END;
            }
            unsigned int finger_id = atoi(req_args[0]);
            for (fingers * finger = finger_list; finger != NULL; finger = finger->next) {
                if (finger->finger_id == finger_id) {
                    if (edit_finger(finger,req_args[1])) {
                        snprintf(output,1024,"changed finger id:%d\n",finger_id);
                        publish(NULL,session_id,strlen(output),output);
                        goto END;
                    } else {
                        publish(NULL,session_id,19,"edit finger failed\n");
                        goto END;
                    }   
                }
            }
            publish(NULL,session_id,19,"edit finger failed\n");
            goto END;
        }
        else if (!strcmp(request,"remove_finger")) {
            if (!req_args[0]) {
                publish(NULL,session_id,21,"remove finger failed\n");
                goto END;
            }
            unsigned int finger_id = atoi(req_args[0]);
            if (remove_finger(finger_id)) {
                snprintf(output,1024,"removed finger id:%d\n",finger_id);
                publish(NULL,session_id,strlen(output),output);
                goto END;
            } 
            else {
                publish(NULL,session_id,21,"remove finger failed\n");
                goto END;
            }
        }
        else if (!strcmp(request,"lock_door")) {
            if (lock_door()) {
                publish(NULL,session_id,18,"lock door success\n");
                goto END;
            } else {
                publish(NULL,session_id,17,"lock door failed\n");
                goto END;
            }
        }
        else if (!strcmp(request,"unlock_door")) {
            if (unlock_door()) {
                publish(NULL,session_id,20,"unlock door success\n");
                goto END;
            } else {
                publish(NULL,session_id,19,"unlock door failed\n");
                goto END;
            }
        }
        END:
        if(session) free(session);
        if(request) free(request);
        if(req_args[0]) free(req_args[0]);
        if(req_args[1]) free(req_args[1]);
        return;
    }
    else if(!strcmp(message->topic, "logger")) {
        char * payload = (char*)message->payload;
        if (!auth_token){
            publish(NULL, "logfile", 15, "not authorized\n");
            return;
        }
        if (!strcmp(payload,"download")) {
            download_log();
        }
        else if (!strcmp(payload,"clear")) {
            clear_log();
        }
    }
    else if(auth_token && !strcmp(message->topic, auth_token)) {
        char * payload = (char*)message->payload;
        char re_auth_token[20];
        snprintf(re_auth_token, 20, "re_%s", auth_token);
        fingers* cur_finger = finger_list;
        while (cur_finger != NULL) {
            if (check_finger(cur_finger,payload)) {
                if (session_id) {
                    free(session_id);
                    unsubscribe(NULL, session_id);
                }
                session_id = (char*)malloc(0x11);
                for (int i = 0; i<0x10;i++) {
                    session_id[i] = session_nums[(rand()%62)];
                }
                session_id[0x10] = 0;
                char output_session[0x30];
                snprintf(output_session, 0x30, "login successed. session_id: %s\n", session_id);
                publish(NULL, re_auth_token, strlen(output_session), output_session);
                return;
            }
            cur_finger = cur_finger->next;
        }
        publish(NULL, re_auth_token, 13, "login failed\n");
    }
}


void mqtt_lock::on_connect(int rc)
{
	log("Connected with code %d.\n", rc);
}

void mqtt_lock::on_disconnect(int rc)
{
    log("Disconnected with code %d.\n", rc);
}

void mqtt_lock::on_publish(int mid)
{
    printf("Published:%d\n",mid);
}



void mqtt_lock::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
	printf("Subscription: %d\n",mid);
}

void mqtt_lock::on_unsubscribe(int mid)
{
    printf("Unsubscribed: %d\n",mid);
}
