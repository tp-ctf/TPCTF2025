// gen_id
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <openssl/evp.h>
#include <openssl/bio.h> // Include the missing header file
#include <ctype.h>
#include <openssl/buffer.h> // Include the missing header file



const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64Encode(unsigned char* s, char* result) {
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (s[i]) {
        char_array_3[j++] = s[i++];
        if (j == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(j = 0; (j < 4) ; j++)
                *result++ = base64_chars[char_array_4[j]];
            j = 0;
        }
    }

    if (j) {
        for(i = j; i < 3; i++)
            char_array_3[i] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (i = 0; (i < j + 1); i++)
            *result++ = base64_chars[char_array_4[i]];

        while((j++ < 3))
            *result++ = '=';
    }

    *result = '\0';
}

void enc(unsigned char *plaintext, char *output) {
    unsigned char key[] = "041d5a442badb629e011d81e39f00d7b";
    unsigned char iv[] = "07ab5a8ae5741075";
    unsigned char *encData;
    int outlen, encLen;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    int len = strlen((char *)plaintext);
    unsigned char *input;
    input = malloc(len+1);
    encData = malloc(len + 16);
    memcpy(input, plaintext, len);
    input[len] = '\x00';
    for (int i = 0; i < len; i++)
        input[i] = tolower(input[i]);

    EVP_EncryptUpdate(ctx, encData, &outlen, input, len);
    encLen = outlen;
    EVP_EncryptFinal_ex(ctx, encData + outlen, &outlen);
    encLen += outlen;

    EVP_CIPHER_CTX_free(ctx);

    encData[encLen] = '\0';
    char* enc_str = malloc(2*encLen +1);
    //output = malloc((encLen*4)/4 + 5);
    //base64_encode(encData, encLen, output);
    printf("<html><body>");
    for (int i = 0; i < encLen; i++)
    {
        sprintf(enc_str + i * 2, "%02x", encData[i]);
    }
    printf("%s",enc_str);
    printf("</body></html>");
}

int main()
{
    printf("Content-Type: text/html\n\n");
    unsigned char* buf = getenv("QUERY_STRING");
    //buf = "password=password";
    unsigned char* seed = malloc(strlen(buf)+1);
    strcpy(seed, buf);
    if (seed && strlen(seed))
    {
        if(!strcmp(seed,"id")){
            
            goto ID;
        }
        else {
            if (!strncmp(seed,"password=",9))
            {
                int len = 0;
                seed += 9;
                unsigned char * pos = seed;
                while(*pos)
                {
                    //decode URIComponent
                    if (*pos == '%')
                    {
                        int a1 = pos[1];
                        int a2 = pos[2];
                        if (a1 >= '0' && a1 <= '9')
                        {
                            a1 -= '0';
                        }
                        else if (a1 >= 'A' && a1 <= 'F')
                        {
                            a1 = a1 - 'A' + 10;
                        }
                        else if (a1 >= 'a' && a1 <= 'f')
                        {
                            a1 = a1 - 'a' + 10;
                        }
                        else
                        {
                            exit(1);
                        }
                        if (a2 >= '0' && a2 <= '9')
                        {
                            a2 -= '0';
                        }
                        else if (a2 >= 'A' && a2 <= 'F')
                        {
                            a2 = a2 - 'A' + 10;
                        }
                        else if (a2 >= 'a' && a2 <= 'f')
                        {
                            a2 = a2 - 'a' + 10;
                        }
                        else
                        {
                            exit(1);
                        }
                        seed[len++] = a1 * 16 + a2;
                        pos += 3;
                    }
                    else
                    {
                        seed[len++] = *pos++;
                    }
                }
                seed[len] = 0;
                char * ret ;
                enc(seed,ret);
            }
        }
    }
    else
    {
        ID:;
        struct timeval tv;
        gettimeofday(&tv, NULL);
        long long seed = tv.tv_sec * 1000LL + tv.tv_usec / 1000;  // milliseconds
        srand(seed);
        unsigned char id[9];
        for (int i = 0; i < 8; i++)
        {
            id[i] = rand() % 256;
        }
        id[8]=0;
        char id_str[20];
        base64Encode(id, id_str);
        
        printf("<html><body>");
        //printf("%s", id_str);
        char id_hex[32];
        for (int i = 0; i < 12; i++)
        {
            sprintf(id_hex + i * 2, "%02x", id_str[i]);
        }
        id_hex[24] = 0;
        printf("%s", id_hex);
        printf("</body></html>");
    }

}


