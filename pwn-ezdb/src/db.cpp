#include <iostream>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include "table_page.h"
using namespace std;
// main class here
#define TABLEPAGE_NUM 0x10
TablePage* pages[TABLEPAGE_NUM];

void init() {
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
}

void print_operations(){
    printf("------- Table Page Operations -------\n");
    printf("1. Create Table Page\n");
    printf("2. Remove Table Page\n");
    printf("3. Insert Record\n");
    printf("4. Get Record\n");
    printf("5. Edit Record\n");
    printf("6. Exit\n");
    printf(">>> ");
}
int main(){
    init();
    // set pages to zero
    for(int i = 0; i < TABLEPAGE_NUM; i++){
        pages[i] = nullptr;
    }
    // main loop
    while(true){
        print_operations();
        int op;
        scanf("%d", &op);
        switch(op){
            case 1:
                {
                    int idx;
                    printf("Index: ");
                    scanf("%d", &idx);
                    if(idx < 0 || idx >= TABLEPAGE_NUM){
                        printf("Invalid index\n");
                        break;
                    }
                    if(pages[idx] != nullptr){
                        printf("Table Page already exists\n");
                        break;
                    }
                    pages[idx] = new TablePage();
                    printf("Table Page created\n");
                }
                break;
            case 2:
                {
                    int idx;
                    printf("Table Page Index: ");
                    scanf("%d", &idx);
                    if(idx < 0 || idx >= TABLEPAGE_NUM){
                        printf("Invalid index\n");
                        break;
                    }
                    if(pages[idx] == nullptr){
                        printf("Table Page does not exist\n");
                        break;
                    }
                    // here
                    free(pages[idx]->GetPage());
                    delete pages[idx];
                    pages[idx] = nullptr;
                    printf("Table Page removed\n");
                }
                break;
            case 3:
                {
                    int idx;
                    printf("Index: ");
                    scanf("%d", &idx);
                    if(idx < 0 || idx >= TABLEPAGE_NUM){
                        printf("Invalid index\n");
                        break;
                    }
                    if(pages[idx] == nullptr){
                        printf("Table Page does not exist\n");
                        break;
                    }
                    Record* record = new Record();
                    printf("Varchar Length: ");
                    scanf("%hd", &record->varchar_len);
                    if(record->varchar_len ==0){
                        printf("Invalid varchar length\n");
                        delete record;
                        break;
                    }
                    record->varchar = new char[record->varchar_len];
                    printf("Varchar: ");
                    read(0, record->varchar, record->varchar_len);
                    slotid_t slotid = pages[idx]->InsertRecord(record);
                    // **remove varchar and record**
                    delete[] record->varchar;
                    delete record;
                    printf("Record inserted, slot id: %d\n", slotid);
                }
                break;
            case 4:
                {
                    int idx;
                    printf("Index: ");
                    scanf("%d", &idx);
                    if(idx < 0 || idx >= TABLEPAGE_NUM){
                        printf("Invalid index\n");
                        break;
                    }
                    if(pages[idx] == nullptr){
                        printf("Table Page does not exist\n");
                        break;
                    }
                    Rid rid;
                    printf("Slot ID: ");
                    scanf("%hd", &rid);
                    auto record = pages[idx]->GetRecord(rid); // 这个没被 delete 可以 mark 一下
                    if (record!=nullptr){
                        printf("Varchar Length: %d\n", record->varchar_len);
                        printf("Varchar: ");
                        write(1, record->varchar, record->varchar_len);
                        printf("\n");
                    }else{
                        printf("Record not found\n");
                    }
                }
                break;
            case 5:
                {
                    // edit record
                    int idx;
                    printf("Index: ");
                    scanf("%d", &idx);
                    printf("Slot ID: ");
                    Rid rid;
                    scanf("%hd", &rid);
                    if(idx < 0 || idx >= TABLEPAGE_NUM){
                        printf("Invalid index\n");
                        break;
                    }
                    if(pages[idx] == nullptr){
                        printf("Table Page does not exist\n");
                        break;
                    }
                    Record* record = new Record();
                    printf("Varchar Length: ");
                    scanf("%hd", &record->varchar_len);
                    record->varchar = new char[record->varchar_len];
                    printf("Varchar: ");
                    read(0, record->varchar, record->varchar_len);
                    if(pages[idx]->EditRecord(rid, record)){
                        printf("Record edited\n");
                    }else{
                        printf("Record Illegal edit\n");
                    }
                }
                break;
            default:
                return 0;
        }
    }
    return 0;
}