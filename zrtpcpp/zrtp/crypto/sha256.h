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
 * Functions to compute SHA256 digest.
 *
 * @author: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#ifndef _SHA256_H
#define _SHA256_H

/**
 * @file sha256.h
 * @brief Function that provide SHA256 support
 * 
 * @ingroup GNU_ZRTP
 * @{
 */

#include <stdint.h>
#include <vector>

#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH 32
#endif

/**
 * Compute SHA256 digest.
 *
 * This functions takes one data chunk and computes its SHA256 digest. This 
 * function creates and deletes an own SHA256 context to perform the SHA256
 * operations.
 *
 * @param data
 *    Points to the data chunk.
 * @param data_length
 *    Length of the data in bytes
 * @param digest
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 32 bytes (SHA256_DIGEST_LENGTH).
 */
void sha256(const uint8_t *data,
            uint64_t data_length,
            uint8_t *digest);

/**
 * Compute SHA256 digest over several data chunks.
 *
 * This functions takes several data chunks and computes the SHA256 digest.
 * This function creates and deletes an own SHA256 context to perform the
 * SHA256 operations.
 *
 * @param data
 *    Vector of pointers that point to the data chunks.
 * @param dataLength
 *    Vector of integers that hold the length of each data chunk.
 * @param digest
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 32 bytes (SHA256_DIGEST_LENGTH).
 */
void sha256(const std::vector<const uint8_t*>& data, const std::vector<uint64_t >& dataLength, uint8_t *digest);

/**
 * Create and initialize a SHA256 context.
 *
 * An application uses this context to hash several data into one SHA256
 * digest. See also sha256Ctx(...) and closeSha256Context(...).
 *
 * @return Returns a pointer to the initialized SHA256 context or @c NULL in case of an error.
 */
void* createSha256Context();

/**
 * Compute a digest and close the SHA256 digest.
 *
 * An application uses this function to compute the SHA256 digest and to
 * close the SHA256 context.
 *
 * @param ctx
 *    Points to the SHA256 context.
 * @param digest
 *    If this pointer is not NULL then it must point to a byte array that
 *    is big enough to hold the SHA256 digest (256 bit = 32 Bytes). If this
 *    pointer is NULL then the functions does not compute the digest but
 *    closes the context only. The context cannot be used anymore.
 */
void closeSha256Context(void* ctx, uint8_t * digest);

/**
 * Initialize a SHA256 context.
 *
 * An application uses this context to hash several data into one SHA256
 * digest. See also sha256Ctx(...) and closeSha384Context(...).
 *
 * @param ctx
 *    Points to the SHA256 context.
 * @return Returns the pointer to the initialized SHA256 context
 */
void* initializeSha256Context(void* ctx);

/**
 * Compute a digest.
 *
 * An application uses this function to compute the SHA256 digest.
 *
 * @param ctx
 *    Points to the SHA256 context.
 * @param digest
 *    If this pointer is not NULL then it must point to a byte array that
 *    is big enough to hold the SHA256 digest (256 bit = 32 Bytes). If this
 *    pointer is NULL then the functions does not compute the digest.
 */
void finalizeSha256Context(void* ctx, uint8_t * digest);

/**
 * Update the SHA256 context with data.
 *
 * This functions updates the SHA256 context with some data.
 * See also CloseSha256Context(...) how to get the digest.
 *
 * @param ctx
 *    Points to the SHA256 context.
 * @param data
 *    Points to the data to update the context.
 * @param dataLength
 *    The length of the data in bytes.
 */
void sha256Ctx(void* ctx, const uint8_t* data, uint64_t dataLength);

/**
 * Update the SHA256 context with several data chunks.
 *
 * This functions updates the SHA256 context with some data.
 * See also CloseSha256Context(...) how to get the digest.
 *
 * @param ctx
 *    Points to the SHA256 context.
 * @param data
 *    Vector of pointers that point to the data chunks. A NULL
 *    pointer in an array element terminates the data chunks.
 * @param dataLength
 *    Vector of integers that hold the length of each data chunk.
 *
 */
void sha256Ctx(void* ctx, const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength);

/**
 * @}
 */
#endif

