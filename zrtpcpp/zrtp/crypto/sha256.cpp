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
 * @author: Werner Dittmann
 */

#include <zrtp/crypto/sha2.h>
#include <zrtp/crypto/sha256.h>

void sha256(const uint8_t *data, uint64_t dataLength, uint8_t *digest )
{
    sha256_ctx ctx = {};

    sha256_begin(&ctx);
    sha256_hash(data, dataLength, &ctx);
    sha256_end(digest, &ctx);
}

void sha256(const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength, uint8_t *digest)
{
    sha256_ctx ctx  = {};

    sha256_begin(&ctx);
    for (size_t i = 0, size = data.size(); i < size; i++) {
        sha256_hash(data[i], dataLength[i], &ctx);
    }
    sha256_end(digest, &ctx);
}

void* createSha256Context()
{
    sha256_ctx *ctx = reinterpret_cast<sha256_ctx*>(malloc(sizeof(sha256_ctx)));
    sha256_begin(ctx);
    return (void*)ctx;
}

void closeSha256Context(void* ctx, uint8_t * digest)
{
    sha256_ctx* hd = reinterpret_cast<sha256_ctx*>(ctx);

    if (digest != NULL && hd != NULL) {
        sha256_end(digest, hd);
    }
    free(hd);
}

void* initializeSha256Context(void* ctx)
{
	sha256_ctx* hd = reinterpret_cast<sha256_ctx*>(ctx);

    if (hd != NULL) {
        sha256_begin(hd);
    }
    return (void*)hd;
}

void finalizeSha256Context(void* ctx, uint8_t * digest)
{
	sha256_ctx* hd = reinterpret_cast<sha256_ctx*>(ctx);

    if (digest != NULL && hd != NULL) {
        sha256_end(digest, hd);
    }
}

void sha256Ctx(void* ctx, const uint8_t* data, uint64_t dataLength)
{
    sha256_ctx* hd = reinterpret_cast<sha256_ctx*>(ctx);

    sha256_hash(data, dataLength, hd);
}

void sha256Ctx(void* ctx, const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength)
{
    sha256_ctx* hd = reinterpret_cast<sha256_ctx*>(ctx);

    for (size_t i = 0, size = data.size(); i < size; i++) {
        sha256_hash(data[i], dataLength[i], hd);
    }
}
