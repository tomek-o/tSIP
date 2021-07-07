/*
 * Copyright 2006 - 2018, Werner Dittmann
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <openssl/crypto.h>
#include <openssl/aes.h>
#include <string.h>

#include <zrtp/crypto/aesCFB.h>

// extern void initializeOpenSSL();


void aesCfbEncrypt(uint8_t* key, int32_t keyLength, uint8_t* IV, uint8_t *data,
                   int32_t dataLength)
{
    AES_KEY aesKey;
    int usedBytes = 0;

//    initializeOpenSSL();

    memset(&aesKey, 0, sizeof( AES_KEY ) );
    if (keyLength == 16) {
        AES_set_encrypt_key(key, 128, &aesKey);
    }
    else if (keyLength == 32) {
        AES_set_encrypt_key(key, 256, &aesKey);
    }
    else {
        return;
    }
    AES_cfb128_encrypt(data, data, dataLength, &aesKey,
                       IV, &usedBytes, AES_ENCRYPT);
}


void aesCfbDecrypt(uint8_t* key, int32_t keyLength, uint8_t* IV, uint8_t *data,
                   int32_t dataLength)
{
    AES_KEY aesKey;
    int usedBytes = 0;

//    initializeOpenSSL();

    memset(&aesKey, 0, sizeof( AES_KEY ) );
    if (keyLength == 16) {
        AES_set_encrypt_key(key, 128, &aesKey);
    }
    else if (keyLength == 32) {
        AES_set_encrypt_key(key, 256, &aesKey);
    }
    else {
        return;
    }
    AES_cfb128_encrypt(data, data, dataLength, &aesKey,
                       IV, &usedBytes, AES_DECRYPT);
}
