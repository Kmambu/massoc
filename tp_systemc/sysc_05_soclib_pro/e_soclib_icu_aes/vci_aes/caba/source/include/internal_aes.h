#ifndef _INTERNAL_AES_H
#define _INTERNAL_AES_H

#include <cassert>
#include <cstring>
#include <openssl/aes.h>

unsigned char userkey[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
unsigned char ivec[16];

void decrypt_data(unsigned char *data, unsigned int len)
{
    AES_KEY aes_key;
    unsigned char output[len];

    /* init ivec */
    for (unsigned i = 0; i < 16; i++)
        ivec[i] = i;

    /* init key */
    int ret = AES_set_decrypt_key(userkey, 128, &aes_key);
    assert(ret >= 0);

    /* compute */
    AES_cbc_encrypt(data, output, len,
            &aes_key, ivec, AES_DECRYPT);

    memcpy(data, output, len);
}

void encrypt_data(unsigned char *data, unsigned int len)
{
    AES_KEY aes_key;
    unsigned char output[len];

    /* init ivec */
    for (unsigned i = 0; i < 16; i++)
        ivec[i] = i;

    /* init key */
    int ret = AES_set_encrypt_key(userkey, 128, &aes_key);
    assert(ret >= 0);

    /* compute */
    AES_cbc_encrypt(data, output, len,
            &aes_key, ivec, AES_ENCRYPT);

    memcpy(data, output, len);
}

#endif
