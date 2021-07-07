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

/*
 * Authors: Werner Dittmann
 */

#define SKEIN_SIZE Skein512
#define SKEIN384_DIGEST_LENGTH  48

#include <cryptcommon/macSkein.h>
#include <zrtp/crypto/skeinMac384.h>

void macSkein384(const uint8_t *key, uint64_t keyLength, const uint8_t* data, uint64_t dataLength, uint8_t* mac, uint32_t* macLength)
{
    macSkein(key, keyLength, data, dataLength, mac, SKEIN384_DIGEST_LENGTH*8, SKEIN_SIZE);
    *macLength = SKEIN384_DIGEST_LENGTH;
}


void macSkein384(const uint8_t* key, uint64_t keyLength, const std::vector<const uint8_t*>& data,
                 const std::vector<uint64_t>& dataLength, uint8_t* mac, uint32_t* macLength )
{
    macSkein(key, keyLength, data, dataLength, mac, SKEIN384_DIGEST_LENGTH*8, SKEIN_SIZE);
    *macLength = SKEIN384_DIGEST_LENGTH;
}

void* createMacSkein384Context(const uint8_t* key, uint64_t keyLength)
{
    return createSkeinMacContext(key, keyLength, SKEIN384_DIGEST_LENGTH*8, SKEIN_SIZE);
}

void macSkein384Ctx(void* ctx, const uint8_t* data, uint64_t dataLength, uint8_t* mac, uint32_t* macLength)
{

    macSkeinCtx(ctx, data, dataLength, mac);
    *macLength = SKEIN384_DIGEST_LENGTH;
}

void macSkein384Ctx(void* ctx,
                    const std::vector<const uint8_t*>& data,
                    const std::vector<uint64_t>& dataLength,
                    uint8_t* mac, uint32_t* macLength )
{
    macSkeinCtx(ctx, data, dataLength, mac);
    *macLength = SKEIN384_DIGEST_LENGTH;
}

void freeMacSkein384Context(void* ctx)
{
    freeSkeinMacContext(ctx);
}