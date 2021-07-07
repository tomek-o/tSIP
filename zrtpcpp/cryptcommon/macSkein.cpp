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


#include <cryptcommon/macSkein.h>
#include <stdlib.h>

void macSkein(const uint8_t* key, uint64_t key_length,
              const uint8_t* data, uint64_t data_length,
              uint8_t* mac, size_t mac_length, SkeinSize_t skeinSize)
{
    SkeinCtx_t ctx = {};

    skeinCtxPrepare(&ctx, skeinSize);

    skeinMacInit(&ctx, key, key_length, mac_length);
    skeinUpdate(&ctx, data, data_length);
    skeinFinal(&ctx, mac);
}

void macSkein(const uint8_t* key, uint64_t key_length,
              std::vector<const uint8_t*> data,
              std::vector<uint64_t> dataLength,
              uint8_t* mac, size_t mac_length, SkeinSize_t skeinSize)
{
    SkeinCtx_t ctx = {};

    skeinCtxPrepare(&ctx, skeinSize);

    skeinMacInit(&ctx, key, key_length, mac_length);
    for (size_t i = 0, size = data.size(); i < size; i++) {
        skeinUpdate(&ctx, data[i], dataLength[i]);
    }
    skeinFinal(&ctx, mac);
}

void* createSkeinMacContext(const uint8_t* key, uint64_t key_length,
                            size_t mac_length, SkeinSize_t skeinSize)
{
	SkeinCtx_t* ctx = (SkeinCtx_t*)malloc(sizeof(SkeinCtx_t));
	if (ctx == NULL)
        return NULL;

    skeinCtxPrepare(ctx, skeinSize);
    skeinMacInit(ctx, key, key_length, mac_length);
    return ctx;
}

void* initializeSkeinMacContext(void* ctx, const uint8_t* key, uint64_t key_length, size_t mac_length, SkeinSize_t skeinSize)
{
    SkeinCtx_t* pctx = (SkeinCtx_t*)ctx;

    skeinCtxPrepare(pctx, skeinSize);
    skeinMacInit(pctx, key, key_length, mac_length);
    return (void*)pctx;
}

void macSkeinCtx(void* ctx, const uint8_t* data, uint64_t data_length, uint8_t* mac)
{
    SkeinCtx_t* pctx = (SkeinCtx_t*)ctx;

    skeinUpdate(pctx, data, data_length);
    skeinFinal(pctx, mac);
    skeinReset(pctx);
}

void macSkeinCtx(void* ctx,
                 const std::vector<const uint8_t*>& data,
                 const std::vector<uint64_t>& dataLength,
                 uint8_t* mac)
{
    SkeinCtx_t* pctx = (SkeinCtx_t*)ctx;

    for (size_t i = 0, size = data.size(); i < size; i++) {
        skeinUpdate(pctx, data[i], dataLength[i]);
    }
    skeinFinal(pctx, mac);
    skeinReset(pctx);
}

void freeSkeinMacContext(void* ctx)
{
    if (ctx)
        free(ctx);
}
