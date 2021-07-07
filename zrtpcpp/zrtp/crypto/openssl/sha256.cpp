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

#include <crypto/sha256.h>

void sha256(const uint8_t *data, uint64_t data_length, uint8_t *digest)
{
	SHA256(data, data_length, digest);
}

void sha256(const std::vector<const uint8_t*>& data, const std::vector<uint64_t >& dataLength, uint8_t *digest)
{
	SHA256_CTX ctx = {};
	SHA256_Init( &ctx);
    for (size_t i = 0, size = data.size(); i < size; i++) {
        SHA256_Update(&ctx, data[i], dataLength[i]);
    }
	SHA256_Final(digest, &ctx);
}

void* createSha256Context()
{
    auto* ctx = (SHA256_CTX*)malloc(sizeof (SHA256_CTX));
    if (ctx == nullptr)
        return nullptr;
    SHA256_Init(ctx);
    return (void*)ctx;
}

void closeSha256Context(void* ctx, uint8_t * digest)
{
    auto* hd = (SHA256_CTX*)ctx;

    if (digest != nullptr && hd != nullptr) {
        SHA256_Final(digest, hd);
    }
    free(hd);
}

void* initializeSha256Context(void* ctx) 
{
    auto* hd = (SHA256_CTX*)ctx;
    SHA256_Init(hd);
    return (void*)hd;
}

void finalizeSha256Context(void* ctx, uint8_t * digest)
{
    auto* hd = (SHA256_CTX*)ctx;
    if (digest != nullptr && hd != nullptr) {
        SHA256_Final(digest, hd);
    }
}

void sha256Ctx(void* ctx, const uint8_t* data, uint64_t dataLength)
{
    auto* hd = (SHA256_CTX*)ctx;
    SHA256_Update(hd, data, dataLength);
}

void sha256Ctx(void* ctx, const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength)
{
    auto* hd = (SHA256_CTX*)ctx;

    for (size_t i = 0, size = data.size(); i < size; i++) {
        SHA256_Update(hd, data[i], dataLength[i]);
    }
}
