#include <iostream>
#include <memory>
using namespace std;
#define PAGE_SIZE 0x400
#define Rid unsigned short
#define slotid_t unsigned short
struct Record{ // 大小为 0x10
    short varchar_len;
    char* varchar;
    int GetSize(){
        return (int)varchar_len;
    }
};
struct Slot{ // 大小为 0x4
    unsigned short offset; // 每次记录 lower 指针的 index
    unsigned short size; // 给一个字节的溢出机会，可以进一步往后溢出 offset，从而给一个任意地址写
};
// slot_id 从前往后增长，Record 从后往前填，最终漏洞出在 lower higher 计算失误从而 overwrite slot 造成一个堆溢出
class TablePage {
 public:
  // 页面初始化
  TablePage(){
    Init();
  }
  void Init(){
    page_ = (char*)malloc(PAGE_SIZE);
    lower_ = page_;
    upper_ = page_ + PAGE_SIZE;
    slots_ = (Slot*)page_;
  }

  // 插入记录，返回插入的槽号
  slotid_t InsertRecord(Record* record){
    int size = record->GetSize();
    // 判断能否插入
    if(GetFreeSpaceSize() < size + sizeof(Slot)){
      return -1;
    }
    // 在前面插入一个 slot，从后往前插入一个 record
    Slot s{
      .offset = (unsigned short)(upper_ - size - page_), // this is wrong
      .size = (unsigned short)size
    };
    memcpy(lower_,(const void*)&s,sizeof(Slot));
    lower_ += sizeof(Slot);
    memcpy(upper_-size,record->varchar,size);
    upper_ -= size;
    return (slotid_t)(lower_ - sizeof(Slot) - page_) / sizeof(Slot);
  }
  // 获取记录
  std::shared_ptr<Record> GetRecord(Rid rid){
    if(rid >= GetSlotNum()){
      return nullptr;
    }
    Slot* slot = slots_ + rid;
    // assemble record
    Record* record = new Record;
    record->varchar_len = slot->size;
    record->varchar = page_+slot->offset;
    return std::shared_ptr<Record>(record);
  }
  // edit
  bool EditRecord(Rid rid, Record* record){
    if(rid >= GetSlotNum()){
      return false;
    }
    Slot* slot = slots_ + rid;
    if(slot->size < record->GetSize()){
      return false;
    }
    memcpy(page_+slot->offset,record->varchar,record->GetSize());
    return true;
  }

  // 获取页面剩余空间大小
  size_t GetFreeSpaceSize() const{
    return (size_t)upper_ - (size_t)lower_ + 1;
  }
  unsigned short GetSlotNum() const{
    return (unsigned short)(lower_ - page_) / sizeof(Slot);
  }
  char* GetPage(){
    return page_;
  }
 private:
  char* page_;
  char* lower_;        // 页面 lower 指针
  char* upper_;        // 页面 upper 指针
  Slot *slots_;             // 槽位数组
};