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

#include "crypto/hmac.h"
#include <cstring>
#include <cstdio>
#include <mem.h>

static int32_t hmacSha1Init(hmacSha1Context *ctx, const uint8_t *key, uint64_t kLength)
{
    int32_t i;
    uint8_t localPad[SHA1_BLOCK_SIZE] = {0};
    uint8_t localKey[SHA1_BLOCK_SIZE] = {0};

	if (key == NULL)
        return 0;

    memset(ctx, 0, sizeof(hmacSha1Context));

    /* check key length and reduce it if necessary */
    if (kLength > SHA1_BLOCK_SIZE) {
        sha1_begin(&ctx->ctx);
        sha1_hash(key, kLength, &ctx->ctx);
        sha1_end(localKey, &ctx->ctx);
    }
    else {
        memcpy(localKey, key, kLength);
    }
    /* prepare inner hash and hold the context */
    for (i = 0; i < SHA1_BLOCK_SIZE; i++)
        localPad[i] = static_cast<uint_8t >(localKey[i] ^ 0x36);

    sha1_begin(&ctx->innerCtx);
    sha1_hash(localPad, SHA1_BLOCK_SIZE, &ctx->innerCtx);

    /* prepare outer hash and hold the context */
    for (i = 0; i < SHA1_BLOCK_SIZE; i++)
        localPad[i] = static_cast<uint_8t >(localKey[i] ^ 0x5c);

    sha1_begin(&ctx->outerCtx);
    sha1_hash(localPad, SHA1_BLOCK_SIZE, &ctx->outerCtx);

    /* copy prepared inner hash to work hash - ready to process data */
    memcpy(&ctx->ctx, &ctx->innerCtx, sizeof(sha1_ctx));

    memset(localKey, 0, sizeof(localKey));

    return 1;
}

static void hmacSha1Reset(hmacSha1Context *ctx)
{
    /* copy prepared inner hash to work hash context */
    memcpy(&ctx->ctx, &ctx->innerCtx, sizeof(sha1_ctx));
}

static void hmacSha1Update(hmacSha1Context *ctx, const uint8_t *data, uint64_t dLength)
{
    /* hash new data to work hash context */
    sha1_hash(data, dLength, &ctx->ctx);
}

static void hmacSha1Final(hmacSha1Context *ctx, uint8_t *mac)
{
    uint8_t tmpDigest[SHA1_DIGEST_SIZE];

    /* finalize work hash context */
    sha1_end(tmpDigest, &ctx->ctx);

    /* copy prepared outer hash to work hash */
    memcpy(&ctx->ctx, &ctx->outerCtx, sizeof(sha1_ctx));

    /* hash inner digest to work (outer) hash context */
    sha1_hash(tmpDigest, SHA1_DIGEST_SIZE, &ctx->ctx);

    /* finalize work hash context to get the hmac*/
    sha1_end(mac, &ctx->ctx);
}


void hmac_sha1(const uint8_t *key, uint64_t keyLength, const uint8_t* data, uint32_t dataLength, uint8_t* mac, int32_t* macLength)
{
    hmacSha1Context ctx = {};

    hmacSha1Init(&ctx, key, keyLength);
    hmacSha1Update(&ctx, data, dataLength);
    hmacSha1Final(&ctx, mac);
    *macLength = SHA1_BLOCK_SIZE;
}

void hmac_sha1(const uint8_t* key, uint64_t keyLength,
               const std::vector<const uint8_t*>& data,
               const std::vector<uint64_t>& dataLength,
               uint8_t* mac, uint32_t* macLength )
{
    hmacSha1Context ctx = {};

    hmacSha1Init(&ctx, key, keyLength);

    for (size_t i = 0, size = data.size(); i < size; i++) {
        hmacSha1Update(&ctx, data[i], dataLength[i]);
    }
    hmacSha1Final(&ctx, mac);
    *macLength = SHA1_BLOCK_SIZE;
}

void* createSha1HmacContext(const uint8_t* key, uint64_t keyLength)
{
	hmacSha1Context *ctx = reinterpret_cast<hmacSha1Context*>(malloc(sizeof(hmacSha1Context)));
	if (ctx == NULL)
        return NULL;

    hmacSha1Init(ctx, key, keyLength);
    return ctx;
}

void* initializeSha1HmacContext(void* ctx, uint8_t* key, uint64_t keyLength)
{
    hmacSha1Context *pctx = (hmacSha1Context*)ctx;

    hmacSha1Init(pctx, key, keyLength);
    return pctx;
}

void hmacSha1Ctx(void* ctx, const uint8_t* data, uint64_t dataLength,
                uint8_t* mac, uint32_t* macLength)
{
    hmacSha1Context *pctx = (hmacSha1Context*)ctx;

    hmacSha1Reset(pctx);
    hmacSha1Update(pctx, data, dataLength);
    hmacSha1Final(pctx, mac);
    *macLength = SHA1_BLOCK_SIZE;
}

void hmacSha1Ctx(void* ctx,
                 const std::vector<const uint8_t*>& data,
                 const std::vector<uint64_t>& dataLength,
                 uint8_t* mac, uint32_t* macLength )
{
    hmacSha1Context *pctx = (hmacSha1Context*)ctx;

    hmacSha1Reset(pctx);
    for (size_t i = 0, size = data.size(); i < size; i++) {
        hmacSha1Update(pctx, data[i], dataLength[i]);
    }
    hmacSha1Final(pctx, mac);
    *macLength = SHA1_BLOCK_SIZE;
}

void freeSha1HmacContext(void* ctx)
{
    if (ctx) {
        memset(ctx, 0, sizeof(hmacSha1Context));
        free(ctx);
    }
}