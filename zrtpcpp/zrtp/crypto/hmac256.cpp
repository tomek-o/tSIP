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

#include <stdint.h>
#include <cstring>
#include <cstdio>
#include "zrtp/crypto/sha2.h"
#include "zrtp/crypto/hmac256.h"

#include <mem.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

typedef struct _hmacSha256Context {
    sha256_ctx ctx;
    sha256_ctx innerCtx;
    sha256_ctx outerCtx;
} hmacSha256Context;

static int32_t hmacSha256Init(hmacSha256Context *ctx, const uint8_t *key, uint64_t kLength)
{
    int32_t i;
    uint8_t localPad[SHA256_BLOCK_SIZE] = {0};
    uint8_t localKey[SHA256_BLOCK_SIZE] = {0};

    if (key == NULL)
        return 0;

    memset(ctx, 0, sizeof(hmacSha256Context));

    /* check key length and reduce it if necessary */
    if (kLength > SHA256_BLOCK_SIZE) {
        sha256_begin(&ctx->ctx);
        sha256_hash(key, kLength, &ctx->ctx);
        sha256_end(localKey, &ctx->ctx);
    }
    else {
        memcpy(localKey, key, kLength);
    }
    /* prepare inner hash and hold the context */
    for (i = 0; i < SHA256_BLOCK_SIZE; i++)
        localPad[i] = static_cast<uint_8t >(localKey[i] ^ 0x36);

    sha256_begin(&ctx->innerCtx);
    sha256_hash(localPad, SHA256_BLOCK_SIZE, &ctx->innerCtx);

    /* prepare outer hash and hold the context */
    for (i = 0; i < SHA256_BLOCK_SIZE; i++)
        localPad[i] = static_cast<uint_8t >(localKey[i] ^ 0x5c);

    sha256_begin(&ctx->outerCtx);
    sha256_hash(localPad, SHA256_BLOCK_SIZE, &ctx->outerCtx);

    /* copy prepared inner hash to work hash - ready to process data */
    memcpy(&ctx->ctx, &ctx->innerCtx, sizeof(sha256_ctx));

    memset(localKey, 0, sizeof(localKey));

    return 1;
}

static void hmacSha256Reset(hmacSha256Context *ctx)
{
    /* copy prepared inner hash to work hash context */
    memcpy(&ctx->ctx, &ctx->innerCtx, sizeof(sha256_ctx));
}

static void hmacSha256Update(hmacSha256Context *ctx, const uint8_t *data, uint64_t dLength)
{
    /* hash new data to work hash context */
    sha256_hash(data, dLength, &ctx->ctx);
}

static void hmacSha256Final(hmacSha256Context *ctx, uint8_t *mac)
{
    uint8_t tmpDigest[SHA256_DIGEST_SIZE];

    /* finalize work hash context */
    sha256_end(tmpDigest, &ctx->ctx);

    /* copy prepared outer hash to work hash */
    memcpy(&ctx->ctx, &ctx->outerCtx, sizeof(sha256_ctx));

    /* hash inner digest to work (outer) hash context */
    sha256_hash(tmpDigest, SHA256_DIGEST_SIZE, &ctx->ctx);

    /* finalize work hash context to get the hmac*/
    sha256_end(mac, &ctx->ctx);
}


void hmac_sha256(const uint8_t *key, uint64_t keyLength, const uint8_t* data, uint64_t dataLength, uint8_t* mac, uint32_t* macLength)
{
    hmacSha256Context ctx = {};

    hmacSha256Init(&ctx, key, keyLength);
    hmacSha256Update(&ctx, data, dataLength);
    hmacSha256Final(&ctx, mac);
    *macLength = SHA256_DIGEST_SIZE;
}

void hmacSha256(const uint8_t* key, uint64_t keyLength,
                const std::vector<const uint8_t*>& dataChunks,
                const std::vector<uint64_t>& dataChunkLength,
                uint8_t* mac, uint32_t* macLength )
{
    hmacSha256Context ctx= {};

    hmacSha256Init(&ctx, key, keyLength);

    for (size_t i = 0, size = dataChunks.size(); i < size; i++) {
        hmacSha256Update(&ctx, dataChunks[i], dataChunkLength[i]);
    }
    hmacSha256Final(&ctx, mac);
    *macLength = SHA256_DIGEST_SIZE;
}

void* createSha256HmacContext(uint8_t* key, uint64_t keyLength)
{
    hmacSha256Context* ctx = reinterpret_cast<hmacSha256Context*>(malloc(sizeof(hmacSha256Context)));

    if (ctx != NULL) {
        hmacSha256Init(ctx, key, keyLength);
    }
    return ctx;
}

void hmacSha256Ctx(void* ctx, const uint8_t* data, uint64_t dataLength,
                uint8_t* mac, uint32_t* macLength)
{
    hmacSha256Context *pctx = (hmacSha256Context*)ctx;

    hmacSha256Reset(pctx);
    hmacSha256Update(pctx, data, dataLength);
    hmacSha256Final(pctx, mac);
    *macLength = SHA256_DIGEST_SIZE;
}

void hmacSha256Ctx(void* ctx,
                   const std::vector<const uint8_t*>& data,
                   const std::vector<uint64_t>& dataLength,
                   uint8_t* mac, uint32_t* macLength )
{
    hmacSha256Context *pctx = (hmacSha256Context*)ctx;

    hmacSha256Reset(pctx);
    for (size_t i = 0, size = data.size(); i < size; i++) {
        hmacSha256Update(pctx, data[i], dataLength[i]);
    }
    hmacSha256Final(pctx, mac);
    *macLength = SHA256_DIGEST_SIZE;
}

void freeSha256HmacContext(void* ctx)
{
    if (ctx) {
        memset(ctx, 0, sizeof(hmacSha256Context));
        free(ctx);
    }
}
#pragma clang diagnostic pop