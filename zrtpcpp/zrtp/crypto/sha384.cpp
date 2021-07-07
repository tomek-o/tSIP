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
#include <zrtp/crypto/sha384.h>

void sha384(const uint_8t *data, uint64_t dataLength, uint8_t *digest )
{
    sha384_ctx ctx = {};

    sha384_begin(&ctx);
    sha384_hash(data, dataLength, &ctx);
    sha384_end(digest, &ctx);
}

void sha384(const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength, uint8_t *digest)
{
    sha384_ctx ctx = {};

    sha384_begin(&ctx);
    for (size_t i = 0, size = data.size(); i < size; i++) {
        sha384_hash(data[i], dataLength[i], &ctx);
    }
    sha384_end(digest, &ctx);
}

void* createSha384Context()
{
	sha384_ctx* ctx = reinterpret_cast<sha384_ctx*>(malloc(sizeof(sha384_ctx)));
	if (ctx != NULL) {
        sha384_begin(ctx);
    }
    return (void*)ctx;
}

void closeSha384Context(void* ctx, uint_8t* digest)
{
	sha384_ctx* hd = reinterpret_cast<sha384_ctx*>(ctx);

    if (digest != NULL && hd != NULL) {
        sha384_end(digest, hd);
    }
    free(hd);
}

void* initializeSha384Context(void* ctx)
{
	sha384_ctx* hd = reinterpret_cast<sha384_ctx*>(ctx);

    if (hd != NULL) {
        sha384_begin(hd);
    }
    return (void*)hd;
}

void finalizeSha384Context(void* ctx, uint_8t* digest)
{
	sha384_ctx* hd = reinterpret_cast<sha384_ctx*>(ctx);

	if (digest != NULL && hd != NULL) {
        sha384_end(digest, hd);
    }
}

void sha384Ctx(void* ctx, const uint_8t* data, uint64_t dataLength)
{
	sha384_ctx* hd = reinterpret_cast<sha384_ctx*>(ctx);

    sha384_hash(data, dataLength, hd);
}

void sha384Ctx(void* ctx, const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength)
{
    sha384_ctx* hd = reinterpret_cast<sha384_ctx*>(ctx);

    for (size_t i = 0, size = data.size(); i < size; i++) {
        sha384_hash(data[i], dataLength[i], hd);
    }
}
