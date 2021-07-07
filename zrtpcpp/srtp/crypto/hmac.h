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
 * Functions to compute SHA1 HAMAC.
 *
 * @author Werner Dittmann
 */

#ifndef HMAC_H
#define HMAC_H

/**
 * @file hmac.h
 * @brief Functions that provide SHA1 HMAC support
 * 
 * @ingroup GNU_ZRTP
 * @{
 */

#include <vector>
#include <stdint.h>
#include "srtp/crypto/sha1.h"

#ifndef SHA1_DIGEST_LENGTH
#define SHA1_DIGEST_LENGTH 20
#endif

typedef struct _hmacSha1Context {
    sha1_ctx ctx;
    sha1_ctx innerCtx;
    sha1_ctx outerCtx;
} hmacSha1Context;


/**
 * Compute SHA1 HMAC.
 *
 * This functions takes one data chunk and computes its SHA1 HMAC.
 *
 * @param key
 *    The MAC key.
 * @param keyLength
 *    Lneght of the MAC key in bytes
 * @param data
 *    Points to the data chunk.
 * @param dataLength
 *    Length of the data in bytes
 * @param mac
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 20 bytes (SHA1_DIGEST_LENGTH).
 * @param macLength
 *    Point to an integer that receives the length of the computed HMAC.
 */

void hmac_sha1(const uint8_t* key, int64_t keyLength,
               const uint8_t* data, uint64_t dataLength,
               uint8_t* mac, int32_t* macLength);

/**
 * Compute SHA1 HMAC over several data cunks.
 *
 * This functions takes several data chunk and computes the SHA1 HAMAC.
 *
 * @param key
 *    The MAC key.
 * @param keyLength
 *    Length of the MAC key in bytes
 * @param data
 *    Vector of pointers that point to the data chunks. A NULL
 *    pointer in an array element terminates the data chunks.
 * @param dataLength
 *    Vector of integers that hold the length of each data chunk.
 * @param mac
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 20 bytes (SHA1_DIGEST_LENGTH).
 * @param macLength
 *    Point to an integer that receives the length of the computed HMAC.
 */
void hmac_sha1(const uint8_t* key, uint64_t keyLength,
               const std::vector<const uint8_t*>& data,
               const std::vector<uint64_t>& dataLength,
               uint8_t* mac, int32_t* macLength);

/**
 * Create and initialize a SHA1 HMAC context.
 *
 * An application uses this context to create several HMAC with the same key.
 *
 * @param key
 *    The MAC key.
 * @param key_length
 *    Lenght of the MAC key in bytes
 * @return Returns a pointer to the initialized context or @c NULL in case of an error.
 */
void* createSha1HmacContext(const uint8_t* key, uint64_t key_length);

/**
 * Initialize a SHA1 HMAC context.
 *
 * An application uses this context to create several HMAC with the same key.
 *
 * @param ctx
 *     Pointer to initialized SHA1 HMAC context
 * @param key
 *    The MAC key.
 * @param key_length
 *    Lenght of the MAC key in bytes
 * @return Returns a pointer to the initialized context.
 */
void* initializeSha1HmacContext(void* ctx, uint8_t* key, uint64_t key_length);

/**
 * Compute SHA1 HMAC.
 *
 * This functions takes one data chunk and computes its SHA1 HMAC. On return
 * the SHA1 MAC context is ready to compute a HMAC for another data chunk.
 *
 * @param ctx
 *     Pointer to initialized SHA1 HMAC context
 * @param data
 *    Points to the data chunk.
 * @param data_length
 *    Length of the data in bytes
 * @param mac
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 20 bytes (SHA1_DIGEST_LENGTH).
 * @param mac_length
 *    Point to an integer that receives the length of the computed HMAC.
 */
void hmacSha1Ctx(void* ctx, const uint8_t* data, uint64_t data_length,
                 uint8_t* mac, int32_t* mac_length);

/**
 * Compute SHA1 HMAC over several data cunks.
 *
 * This functions takes several data chunks and computes the SHA1 HAMAC. On return
 * the SHA1 MAC context is ready to compute a HMAC for another data chunk.
 *
 * @param ctx 
 *     Pointer to initialized SHA1 HMAC context
 * @param data
 *    Vector of pointers that point to the data chunks. A NULL
 *    pointer in an array element terminates the data chunks.
 * @param data_length
 *    Vector of integers that hold the length of each data chunk.
 * @param mac
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 20 bytes (SHA1_DIGEST_LENGTH).
 * @param macLength
 *    Point to an integer that receives the length of the computed HMAC.
 */
void hmacSha1Ctx(void* ctx,
                 const std::vector<const uint8_t*>& data,
                 const std::vector<uint64_t>& dataLength,
                 uint8_t* mac, uint32_t* macLength);

/**
 * Free SHA1 HMAC context.
 *
 * @param ctx a pointer to SHA1 HMAC context
 */
void freeSha1HmacContext(void* ctx);


/**
 * @}
 */
#endif
