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
#include "zrtp/crypto/hmac384.h"

#include <mem.h>

typedef struct _hmacSha384Context {
    sha384_ctx ctx;
    sha384_ctx innerCtx;
    sha384_ctx outerCtx;
} hmacSha384Context;

static int32_t hmacSha384Init(hmacSha384Context *ctx, const uint8_t *key, uint64_t kLength)
{
    int32_t i;
    uint8_t localPad[SHA384_BLOCK_SIZE] = {0};
    uint8_t localKey[SHA384_BLOCK_SIZE] = {0};

    if (key == NULL)
        return 0;

    memset(ctx, 0, sizeof(hmacSha384Context));

    /* check key length and reduce it if necessary */
    if (kLength > SHA384_BLOCK_SIZE) {
        sha384_begin(&ctx->ctx);
        sha384_hash(key, kLength, &ctx->ctx);
        sha384_end(localKey, &ctx->ctx);
    }
    else {
        memcpy(localKey, key, kLength);
    }
    /* prepare inner hash and hold the context */
    for (i = 0; i < SHA384_BLOCK_SIZE; i++)
        localPad[i] = static_cast<uint_8t >(localKey[i] ^ 0x36);

    sha384_begin(&ctx->innerCtx);
    sha384_hash(localPad, SHA384_BLOCK_SIZE, &ctx->innerCtx);

    /* prepare outer hash and hold the context */
    for (i = 0; i < SHA384_BLOCK_SIZE; i++)
        localPad[i] = static_cast<uint_8t >(localKey[i] ^ 0x5c);

    sha384_begin(&ctx->outerCtx);
    sha384_hash(localPad, SHA384_BLOCK_SIZE, &ctx->outerCtx);

    /* copy prepared inner hash to work hash - ready to process data */
    memcpy(&ctx->ctx, &ctx->innerCtx, sizeof(sha384_ctx));

    memset(localKey, 0, sizeof(localKey));

    return 1;
}

static void hmacSha384Reset(hmacSha384Context *ctx)
{
    /* copy prepared inner hash to work hash context */
    memcpy(&ctx->ctx, &ctx->innerCtx, sizeof(sha384_ctx));
}

static void hmacSha384Update(hmacSha384Context *ctx, const uint8_t *data, uint64_t dLength)
{
    /* hash new data to work hash context */
    sha384_hash(data, dLength, &ctx->ctx);
}

static void hmacSha384Final(hmacSha384Context *ctx, uint8_t *mac)
{
    uint8_t tmpDigest[SHA384_DIGEST_SIZE];

    /* finalize work hash context */
    sha384_end(tmpDigest, &ctx->ctx);

    /* copy prepared outer hash to work hash */
    memcpy(&ctx->ctx, &ctx->outerCtx, sizeof(sha384_ctx));

    /* hash inner digest to work (outer) hash context */
    sha384_hash(tmpDigest, SHA384_DIGEST_SIZE, &ctx->ctx);

    /* finalize work hash context to get the hmac*/
    sha384_end(mac, &ctx->ctx);
}


void hmac_sha384(const uint8_t *key, uint64_t keyLength, const uint8_t* data, uint64_t dataLength, uint8_t* mac, uint32_t* macLength)
{
    hmacSha384Context ctx = {};

    hmacSha384Init(&ctx, key, keyLength);
    hmacSha384Update(&ctx, data, dataLength);
    hmacSha384Final(&ctx, mac);
    *macLength = SHA384_DIGEST_SIZE;
}

void hmacSha384(const uint8_t* key, uint64_t keyLength,
                const std::vector<const uint8_t*>& data,
                const std::vector<uint64_t>& dataLength,
                uint8_t* mac, uint32_t* macLength )
{
    hmacSha384Context ctx = {};

    hmacSha384Init(&ctx, key, keyLength);

    for (size_t i = 0, size = data.size(); i < size; i++) {
        hmacSha384Update(&ctx, data[i], dataLength[i]);
    }
    hmacSha384Final(&ctx, mac);
    *macLength = SHA384_DIGEST_SIZE;
}

void* createSha384HmacContext(const uint8_t* key, uint64_t keyLength)
{
	hmacSha384Context *ctx = reinterpret_cast<hmacSha384Context*>(malloc(sizeof(hmacSha384Context)));

    if (ctx != NULL) {
        hmacSha384Init(ctx, key, keyLength);
    }
    return ctx;
}

void hmacSha384Ctx(void* ctx, const uint8_t* data, uint64_t dataLength,
                uint8_t* mac, uint32_t* macLength)
{
    hmacSha384Context* pctx = (hmacSha384Context*)ctx;

    hmacSha384Reset(pctx);
    hmacSha384Update(pctx, data, dataLength);
    hmacSha384Final(pctx, mac);
    *macLength = SHA384_DIGEST_SIZE;
}

void hmacSha384Ctx(void* ctx,
                   const std::vector<const uint8_t*>& data,
                   const std::vector<uint64_t>& dataLength,
                   uint8_t* mac, uint32_t* macLength )
{
    hmacSha384Context* pctx = (hmacSha384Context*)ctx;

    hmacSha384Reset(pctx);
    for (size_t i = 0, size = data.size(); i < size; i++) {
        hmacSha384Update(pctx, data[i], dataLength[i]);
    }
    hmacSha384Final(pctx, mac);
    *macLength = SHA384_DIGEST_SIZE;
}

void freeSha384HmacContext(void* ctx)
{
    if (ctx) {
        memset(ctx, 0, sizeof(hmacSha384Context));
        free(ctx);
    }
}