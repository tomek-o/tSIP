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

/** Copyright (C) 2011
 *
 * @author  Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#include <string.h>

#include <zrtp/crypto/twoCFB.h>
#include <cryptcommon/twofish.h>

static int initialized = 0;

void twoCfbEncrypt(uint8_t* key, int32_t keyLength, uint8_t* IV, uint8_t *data, int32_t dataLength)
{
    Twofish_key keyCtx;
    int usedBytes = 0;

    if (!initialized) {
        Twofish_initialise();
        initialized = 1;
    }

    memset(&keyCtx, 0, sizeof(Twofish_key));
    Twofish_prepare_key(key, keyLength, &keyCtx);

    Twofish_cfb128_encrypt(&keyCtx, (Twofish_Byte*)data, (Twofish_Byte*)data,
			   (size_t)dataLength, (Twofish_Byte*)IV, &usedBytes);
}


void twoCfbDecrypt(uint8_t* key, int32_t keyLength, uint8_t* IV, uint8_t *data, int32_t dataLength)
{
    Twofish_key keyCtx;
    int usedBytes = 0;

    if (!initialized) {
        Twofish_initialise();
        initialized = 1;
    }

    memset(&keyCtx, 0, sizeof(Twofish_key));
    Twofish_prepare_key(key, keyLength, &keyCtx);

    Twofish_cfb128_decrypt(&keyCtx, (Twofish_Byte*)data, (Twofish_Byte*)data, 
			   (size_t)dataLength, (Twofish_Byte*)IV, &usedBytes);
}
