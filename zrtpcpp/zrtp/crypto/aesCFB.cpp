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

/**
 * @author  Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#include <string.h>

#include <zrtp/crypto/aesCFB.h>
#include <cryptcommon/aescpp.h>

void aesCfbEncrypt(uint8_t *key, int32_t keyLength, uint8_t* IV, uint8_t *data, int32_t dataLength)
{
    AESencrypt* saAes = new AESencrypt();

    if (keyLength == 16)
        saAes->key128(key);
    else if (keyLength == 32)
        saAes->key256(key);
    else
        return;

    // Note: maybe copy IV to an internal array if we encounter strange things.
    // the cfb encrypt modifies the IV on return. Same for output data (inplace encryption)
    saAes->cfb_encrypt(data, data, dataLength, IV);
    delete saAes;
}


void aesCfbDecrypt(uint8_t *key, int32_t keyLength, uint8_t* IV, uint8_t *data, int32_t dataLength)
{
    AESencrypt* saAes = new AESencrypt();
    if (keyLength == 16)
        saAes->key128(key);
    else if (keyLength == 32)
        saAes->key256(key);
    else
        return;

    // Note: maybe copy IV to an internal array if we encounter strange things.
    // the cfb encrypt modifies the IV on return. Same for output data (inplace encryption)
    saAes->cfb_decrypt(data, data, dataLength, IV);
    delete saAes;
}
