
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	uint32_t a, b, c, d;
} md5_context;


// from RFC 1321, Section 3.4:
#define F(X, Y, Z) (((X) & (Y)) | ((~(X)) & (Z)))
#define G(X, Y, Z) (((X) & (Z)) | (Y & (~(Z))))
#define H(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define I(X, Y, Z) ((Y) ^ ((X) | (~(Z))))

static
uint32_t rotl(uint32_t x, int s) { return (x << s) | (x >> (32 - s)); }
#define STEP(OP, a, b, c, d, k, s, i) do{ \
	a = b + rotl(a + OP(b, c, d) + X[k] + i, s); \
	}while(0)

#define TO_I32(x,i) ((x[i]) | (x[i+1]<<8) | (x[i+2]<<16) | (x[i+3]<<24))
static
void md5_block(md5_context* ctx, const uint8_t m[64]) {
	assert(ctx != NULL);

	uint32_t X[16] = {
		TO_I32(m,0), TO_I32(m,4), TO_I32(m,8), TO_I32(m,12),
		TO_I32(m,16), TO_I32(m,20), TO_I32(m,24), TO_I32(m,28),
		TO_I32(m,32), TO_I32(m,36), TO_I32(m,40), TO_I32(m,44),
		TO_I32(m,48), TO_I32(m,52), TO_I32(m,56), TO_I32(m,60)
	};

	uint32_t a = ctx->a;
	uint32_t b = ctx->b;
	uint32_t c = ctx->c;
	uint32_t d = ctx->d;

	STEP(F, a, b, c, d, 0,  7, 0xd76aa478);
	STEP(F, d, a, b, c, 1, 12, 0xe8c7b756);
	STEP(F, c, d, a, b, 2, 17, 0x242070db);
	STEP(F, b, c, d, a, 3, 22, 0xc1bdceee);
	STEP(F, a, b, c, d, 4,  7, 0xf57c0faf);
	STEP(F, d, a, b, c, 5, 12, 0x4787c62a);
	STEP(F, c, d, a, b, 6, 17, 0xa8304613);
	STEP(F, b, c, d, a, 7, 22, 0xfd469501);
	STEP(F, a, b, c, d,  8,  7, 0x698098d8);
	STEP(F, d, a, b, c,  9, 12, 0x8b44f7af);
	STEP(F, c, d, a, b, 10, 17, 0xffff5bb1);
	STEP(F, b, c, d, a, 11, 22, 0x895cd7be);
	STEP(F, a, b, c, d, 12,  7, 0x6b901122);
	STEP(F, d, a, b, c, 13, 12, 0xfd987193);
	STEP(F, c, d, a, b, 14, 17, 0xa679438e);
	STEP(F, b, c, d, a, 15, 22, 0x49b40821);
	STEP(G, a, b, c, d,  1,  5, 0xf61e2562);
	STEP(G, d, a, b, c,  6,  9, 0xc040b340);
	STEP(G, c, d, a, b, 11, 14, 0x265e5a51);
	STEP(G, b, c, d, a,  0, 20, 0xe9b6c7aa);
	STEP(G, a, b, c, d,  5,  5, 0xd62f105d);
	STEP(G, d, a, b, c, 10,  9, 0x02441453);
	STEP(G, c, d, a, b, 15, 14, 0xd8a1e681);
	STEP(G, b, c, d, a,  4, 20, 0xe7d3fbc8);
	STEP(G, a, b, c, d,  9,  5, 0x21e1cde6);
	STEP(G, d, a, b, c, 14,  9, 0xc33707d6);
	STEP(G, c, d, a, b,  3, 14, 0xf4d50d87);
	STEP(G, b, c, d, a,  8, 20, 0x455a14ed);
	STEP(G, a, b, c, d, 13,  5, 0xa9e3e905);
	STEP(G, d, a, b, c,  2,  9, 0xfcefa3f8);
	STEP(G, c, d, a, b,  7, 14, 0x676f02d9);
	STEP(G, b, c, d, a, 12, 20, 0x8d2a4c8a);
	STEP(H, a, b, c, d,  5,  4, 0xfffa3942);
	STEP(H, d, a, b, c,  8, 11, 0x8771f681);
	STEP(H, c, d, a, b, 11, 16, 0x6d9d6122);
	STEP(H, b, c, d, a, 14, 23, 0xfde5380c);
	STEP(H, a, b, c, d,  1,  4, 0xa4beea44);
	STEP(H, d, a, b, c,  4, 11, 0x4bdecfa9);
	STEP(H, c, d, a, b,  7, 16, 0xf6bb4b60);
	STEP(H, b, c, d, a, 10, 23, 0xbebfbc70);
	STEP(H, a, b, c, d, 13,  4, 0x289b7ec6);
	STEP(H, d, a, b, c,  0, 11, 0xeaa127fa);
	STEP(H, c, d, a, b,  3, 16, 0xd4ef3085);
	STEP(H, b, c, d, a,  6, 23, 0x04881d05);
	STEP(H, a, b, c, d,  9,  4, 0xd9d4d039);
	STEP(H, d, a, b, c, 12, 11, 0xe6db99e5);
	STEP(H, c, d, a, b, 15, 16, 0x1fa27cf8);
	STEP(H, b, c, d, a,  2, 23, 0xc4ac5665);
	STEP(I, a, b, c, d,  0,  6, 0xf4292244);
	STEP(I, d, a, b, c,  7, 10, 0x432aff97);
	STEP(I, c, d, a, b, 14, 15, 0xab9423a7);
	STEP(I, b, c, d, a,  5, 21, 0xfc93a039);
	STEP(I, a, b, c, d, 12,  6, 0x655b59c3);
	STEP(I, d, a, b, c,  3, 10, 0x8f0ccc92);
	STEP(I, c, d, a, b, 10, 15, 0xffeff47d);
	STEP(I, b, c, d, a,  1, 21, 0x85845dd1);
	STEP(I, a, b, c, d,  8,  6, 0x6fa87e4f);
	STEP(I, d, a, b, c, 15, 10, 0xfe2ce6e0);
	STEP(I, c, d, a, b,  6, 15, 0xa3014314);
	STEP(I, b, c, d, a, 13, 21, 0x4e0811a1);
	STEP(I, a, b, c, d,  4,  6, 0xf7537e82);
	STEP(I, d, a, b, c, 11, 10, 0xbd3af235);
	STEP(I, c, d, a, b,  2, 15, 0x2ad7d2bb);
	STEP(I, b, c, d, a,  9, 21, 0xeb86d391);

	ctx->a += a;
	ctx->b += b;
	ctx->c += c;
	ctx->d += d;

	memset(X, 0, sizeof(X));
}

void md5_init(md5_context* ctx) {
	assert(ctx != NULL);
	memset(ctx, 0, sizeof(md5_context));
	// initialization values from RFC 1321, Section 3.3:
	ctx->a = 0x67452301;
	ctx->b = 0xEFCDAB89;
	ctx->c = 0x98BADCFE;
	ctx->d = 0x10325476;
}

#define TO_U8(x,o,i) do{ \
	o[i] = (x) & 0xFF; \
	o[i+1] = ((x) >> 8) & 0xFF; \
	o[i+2] = ((x) >> 16) & 0xFF; \
	o[i+3] = ((x) >> 24) & 0xFF; }while(0)

void md5_digest(md5_context* ctx, void* buffer, size_t size) {
	uint8_t* bytes = (uint8_t*)buffer;
	uint64_t message_bits = size * 8;
	ssize_t rem_size = size;
	while (rem_size > 64) {
		md5_block(ctx, bytes);
		bytes += 64;
		rem_size -= 64;
	}
	uint8_t scratch[64];
	memset(scratch, 0, 64);
	memcpy(scratch, bytes, rem_size);
	if (rem_size == 64) {
		md5_block(ctx, scratch);
		memset(scratch, 0, 64);
		scratch[0] = 0x80;
	} else {
		scratch[rem_size] = 0x80;
		if (64 - (rem_size + 1) < 8) {
			md5_block(ctx, scratch);
			memset(scratch, 0, 64);
		}
	}
	TO_U8(message_bits, scratch, 56);
	TO_U8(message_bits>>32, scratch, 60);
	md5_block(ctx, scratch);
	memset(scratch, 0x00, 64);
}

void md5_output(md5_context* ctx, uint8_t out[16]) {
	TO_U8(ctx->a, out, 0);
	TO_U8(ctx->b, out, 4);
	TO_U8(ctx->c, out, 8);
	TO_U8(ctx->d, out, 12);
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test(char* input, char* md5_expected) {
	md5_context ctx;
	md5_init(&ctx);
	md5_digest(&ctx, input, strlen(input));

	uint8_t md5_actual[16] = {0};
	md5_output(&ctx, md5_actual);


	if (memcmp(md5_actual, md5_expected, 16) != 0) {
		printf("Wrong\n");
		exit(0);
	}
}

char flags[][17]={"\x38\xf8\x8a\x3b\xc5\x70\x21\x0f\x8a\x8d\x95\x58\x5b\x46\xb0\x65", "\x83\x05\x5a\xe8\x0c\xdc\x8b\xd5\x93\x78\xb8\x62\x8d\x73\x3f\xcb", "\xfa\x7d\xaf\xfb\xd7\xac\xec\x13\xb0\x69\x5d\x93\x5a\x04\xbc\x0f", "\xc2\x9c\xc0\xfd\x38\x01\xc7\xfd\xd3\x15\xc7\x82\x99\x9b\xd4\xcb", "\x2b\xa2\xd0\x1a\xf1\x2d\x9b\xe3\x1a\x2b\x44\x32\x3c\x1a\x4f\x47", "\xdd\xee\xba\xf0\x02\x52\x7a\x9e\xad\x78\xbd\x16\x68\x45\x73\xcc", "\xbf\x95\xb8\x99\x34\xa1\xb5\x55\xe1\x09\x0f\xec\xdf\xd3\xda\x9f", "\xb6\x42\x2c\x30\xb0\x29\x38\x53\x5f\x8e\x64\x8d\x60\xa8\x7b\x94", "\x08\xc1\xb7\x66\x43\xaf\x8d\xd5\x0c\xb0\x6d\x7f\xdd\x3c\xf8\xed", "\x42\xd6\x97\x19\xf9\x70\x88\xf0\x65\x40\xf4\x12\xdc\x17\x06\xfb", "\xa1\xf2\x3d\xa6\x16\x15\x40\x0e\x7b\xd9\xea\x72\xd6\x35\x67\xeb", "\x4e\x24\x6f\x0a\x5d\xd3\xce\x59\x46\x5f\xf3\xd0\x2e\xc4\xf9\x84", "\xb8\xcf\x25\xf9\x63\xe8\xe9\xf4\xc3\xfd\xda\x34\xf6\xf0\x1a\x35", "\x2d\x98\xd8\x20\x83\x5c\x75\xa9\xf9\x81\xad\x4d\xb8\x26\xbf\x8e", "\x70\x2e\xad\x08\xa3\xdd\x56\xb3\x13\x4c\x7c\x38\x41\xa6\x52\xaa", "\xd2\xd5\x57\xb6\x13\x66\x2b\x92\xf3\x99\xd6\x12\xfb\x91\x59\x1e", "\xe4\x42\x2b\x63\x20\xed\x98\x9e\x7e\x3c\xb9\x7f\x36\x9c\xba\x38", "\x71\x80\x35\x86\xc6\x70\x59\xdd\xa3\x25\x25\xce\x84\x4c\x50\x79", "\x83\xb3\x71\x80\x1d\x0a\xde\x07\xb5\xc4\xf5\x1e\x8c\x62\x15\xe2", "\xb0\xd1\xb4\x88\x5b\xc2\xfd\xc5\xa6\x65\x26\x69\x24\x48\x6c\x5f", "\x79\x2c\x9e\x7f\x05\xc4\x07\xc5\x6f\x3b\xec\x4c\xa7\xe5\xc1\x71", "\x38\x55\xe5\xa5\xbb\xc1\xcb\xe1\x8a\x6e\xab\x5d\xd9\x7c\x06\x3c", "\x88\x6d\x45\xe0\x45\x1b\xbb\xa7\xc0\x34\x1f\xe9\x0a\x95\x4f\x34", "\x3a\x43\x7c\xbe\x65\x91\xea\x34\x89\x64\x25\x85\x6e\xae\x7b\x65", "\x34\x30\x49\x67\xa0\x67\x30\x8a\x76\x70\x1f\x05\xc0\x66\x85\x51", "\xd6\xaf\x7c\x4f\xed\xcf\x2b\x67\x77\xdf\x8e\x83\xc9\x32\xf8\x83", "\xdf\x88\x93\x1e\x7e\xef\xdf\xcc\x2b\xb8\x0d\x4a\x4f\x57\x10\xfb", "\xcb\x0f\xc8\x13\x75\x5a\x45\xce\x59\x84\xbf\xba\x15\x84\x7c\x1e"};
char flag[100];
int i;

int main() {
	
	printf("Flag: \n");
	scanf("%29s",flag);
	
	for (i=0;i<28;i++){
		test(flag+i,flags[i]);
	}
	
	printf("Correct\n");
}
