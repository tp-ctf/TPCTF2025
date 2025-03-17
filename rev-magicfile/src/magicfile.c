#include <magic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__asm__(".section .rodata\n" \
        ".global mgc\n" \
        "mgc: .incbin \"tpctf.magic.mgc\"\n");
extern char mgc[];
void* mgc_list[] = {mgc};
size_t size[] = {18283000};

int main() {
    char *flag = (char *)malloc(100);
    printf("Please input the flag: ");
    scanf("%100s", flag);

    if (strlen(flag) != 48) {
    fail:
        puts("Try again.");
        return 0;
    }

    magic_t magic = magic_open(MAGIC_NONE);
    magic_load_buffers(magic, mgc_list, size, 1);
    const char* mime = magic_buffer(magic, flag, strlen(flag));
    printf("%s\n", mime);
    magic_close(magic);
    return 0;
}
