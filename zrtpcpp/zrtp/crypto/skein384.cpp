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

#include <cryptcommon/skeinApi.h>
#include <zrtp/crypto/skein384.h>

#include <stdlib.h>

#define SKEIN_SIZE Skein512
#define SKEIN384_DIGEST_LENGTH  48

void skein384(const uint8_t *data, uint64_t dataLength, uint8_t *digest )
{
    SkeinCtx_t ctx = {};

    skeinCtxPrepare(&ctx, SKEIN_SIZE);
    skeinInit(&ctx, SKEIN384_DIGEST_LENGTH*8);
    skeinUpdate(&ctx, data, dataLength);

    skeinFinal(&ctx, digest);
}

void skein384(const std::vector<const uint_8t *>& dataChunks, const std::vector<uint64_t>& dataChunkLength, uint8_t *digest)
{
    SkeinCtx_t ctx = {};

    skeinCtxPrepare(&ctx, SKEIN_SIZE);
    skeinInit(&ctx, SKEIN384_DIGEST_LENGTH*8);
    for (size_t i = 0, size = dataChunks.size(); i < size; i++) {
        skeinUpdate(&ctx, dataChunks[i], dataChunkLength[i]);
    }
    skeinFinal(&ctx, digest);
}

void* createSkein384Context()
{
    SkeinCtx_t *ctx = reinterpret_cast<SkeinCtx_t *>(malloc(sizeof(SkeinCtx_t )));
    if (ctx != NULL) {
        skeinCtxPrepare(ctx, SKEIN_SIZE);
        skeinInit(ctx, SKEIN384_DIGEST_LENGTH*8);
    }
    return (void*)ctx;
}

void closeSkein384Context(void* ctx, uint8_t* digest)
{
    SkeinCtx_t* hd = reinterpret_cast<SkeinCtx_t*>(ctx);

    if (digest != NULL && hd != NULL) {
        skeinFinal(hd, digest);
    }
    free(hd);
}

void* initializeSkein384Context(void* ctx)
{
	SkeinCtx_t* hd = reinterpret_cast<SkeinCtx_t *>(ctx);
    if (hd != NULL) {
        skeinCtxPrepare(hd, SKEIN_SIZE);
        skeinInit(hd, SKEIN384_DIGEST_LENGTH*8);
    }
    return (void*)hd;
}

void finalizeSkein384Context(void* ctx, uint8_t* digest)
{
	SkeinCtx_t* hd = reinterpret_cast<SkeinCtx_t*>(ctx);

    if (digest != NULL && hd != NULL) {
        skeinFinal(hd, digest);
    }
}

void skein384Ctx(void* ctx, const uint8_t* data, uint64_t dataLength)
{
	SkeinCtx_t* hd = reinterpret_cast<SkeinCtx_t*>(ctx);

    skeinUpdate(hd, data, dataLength);
}

void skein384Ctx(void* ctx,
                 const std::vector<const uint8_t*>& data,
                 const std::vector<uint64_t>& dataLength)
{
    SkeinCtx_t* hd = reinterpret_cast<SkeinCtx_t*>(ctx);

    for (size_t i = 0, size = data.size(); i < size; i++) {
        skeinUpdate(hd, data[i], dataLength[i]);
    }
}
