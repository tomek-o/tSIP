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
#include <openssl/sha.h>

#include <crypto/sha384.h>

void sha384(const uint8_t *data, uint64_t dataLength, uint8_t *digest)
{
	SHA384(data, dataLength, digest);
}

void sha384(const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength, uint8_t* digest)
{
	SHA512_CTX ctx = {};
	SHA384_Init( &ctx);
    for (size_t i = 0, size = data.size(); i < size; i++) {
        SHA384_Update(&ctx, data[i], dataLength[i]);
    }
	SHA384_Final(digest, &ctx);
}

void* createSha384Context()
{
    auto* ctx = (SHA512_CTX*)malloc(sizeof (SHA512_CTX));
    if (ctx == nullptr)
        return nullptr;
    SHA384_Init(ctx);
    return (void*)ctx;
}

void closeSha384Context(void* ctx, uint8_t * digest)
{
    auto* hd = (SHA512_CTX*)ctx;

    if (digest != nullptr) {
        SHA384_Final(digest, hd);
    }
    free(hd);
}

void* initializeSha384Context(void* ctx)
{
    auto* hd = (SHA512_CTX*)ctx;
    SHA384_Init(hd);
    return (void*)hd;
}

void finalizeSha384Context(void* ctx, uint8_t * digest)
{
    auto* hd = (SHA512_CTX*)ctx;

    if (digest != nullptr) {
        SHA384_Final(digest, hd);
    }
}

void sha384Ctx(void* ctx, const uint8_t* data, uint64_t dataLength)
{
    auto* hd = (SHA512_CTX*)ctx;
    SHA384_Update(hd, data, dataLength);
}

void sha384Ctx(void* ctx, const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength)
{
    auto* hd = (SHA512_CTX*)ctx;

    for (size_t i = 0, size = data.size(); i < size; i++) {
        SHA384_Update(hd, data[i], dataLength[i]);
    }
}
