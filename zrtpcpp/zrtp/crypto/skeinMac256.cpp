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

#include <cryptcommon/macSkein.h>
#include <zrtp/crypto/skeinMac256.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

void macSkein256(const uint8_t *key, uint64_t keyLength, const uint8_t* data, uint64_t dataLength, uint8_t* mac, uint32_t* macLength)
{
    macSkein(key, keyLength, data, dataLength, mac, SKEIN256_DIGEST_LENGTH*8, SKEIN_SIZE);
    *macLength = SKEIN256_DIGEST_LENGTH;
}


void macSkein256(const uint8_t* key, uint64_t keyLength, const std::vector<const uint8_t*>& data,
                  const std::vector<uint64_t>& dataLength, uint8_t* mac, uint32_t* macLength )
{
    macSkein(key, keyLength, data, dataLength, mac, SKEIN256_DIGEST_LENGTH*8, SKEIN_SIZE);
    *macLength = SKEIN256_DIGEST_LENGTH;
}

void* createMacSkein256Context(uint8_t* key, uint64_t keyLength)
{
    return createSkeinMacContext(key, keyLength, SKEIN256_DIGEST_LENGTH*8, SKEIN_SIZE);
}

void macSkein256Ctx(void* ctx, const uint8_t* data, uint32_t dataLength, uint8_t* mac, int32_t* macLength)
{

    macSkeinCtx(ctx, data, dataLength, mac);
    *macLength = SKEIN256_DIGEST_LENGTH;
}

void macSkein256Ctx(void* ctx, const std::vector<const uint8_t*>& data,
                    const std::vector<uint64_t>& dataLength, uint8_t* mac, int32_t* macLength )
{
    macSkeinCtx(ctx, data, dataLength, mac);
    *macLength = SKEIN256_DIGEST_LENGTH;
}

void freeMacSkein256Context(void* ctx)
{
    freeSkeinMacContext(ctx);
}
#pragma clang diagnostic pop