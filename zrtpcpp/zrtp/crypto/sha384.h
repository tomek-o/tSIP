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
 * Functions to compute SHA384 digest.
 *
 * @author: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#ifndef _SHA384_H
#define _SHA384_H

/**
 * @file sha384.h
 * @brief Function that provide SHA384 support
 * 
 * @ingroup GNU_ZRTP
 * @{
 */

#include <stdint.h>
#include <vector>

#ifndef SHA384_DIGEST_LENGTH
#define SHA384_DIGEST_LENGTH 48
#endif

/**
 * Compute SHA384 digest.
 *
 * This functions takes one data chunk and computes its SHA384 digest. This 
 * function creates and deletes an own SHA384 context to perform the SHA384
 * operations.
 *
 * @param data
 *    Points to the data chunk.
 * @param dataLength
 *    Length of the data in bytes
 * @param digest
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 48 bytes (SHA384_DIGEST_LENGTH).
 */
void sha384(const uint8_t *data,
            uint64_t dataLength,
            uint8_t *digest);

/**
 * Compute SHA384 digest over several data cunks.
 *
 * This functions takes several data chunks and computes the SHA384 digest.
 * This function creates and deletes an own SHA384 context to perform the
 * SHA384 operations.
 *
 * @param data
 *    Vector of pointers that point to the data chunks. A NULL
 *    pointer in an array element terminates the data chunks.
 * @param dataLength
 *    Vector of integers that hold the length of each data chunk.
 * @param digest
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 48 bytes (SHA384_DIGEST_LENGTH).
 */
void sha384(const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength, uint8_t* digest);

/**
 * Create and initialize a SHA384 context.
 *
 * An application uses this context to hash several data into one SHA384
 * digest. See also sha384Ctx(...) and closeSha384Context(...).
 *
 * @return Returns a pointer to the initialized SHA384 context or @c NULL in case of an error.
 */
void* createSha384Context();

/**
 * Compute a digest and close the SHA384 digest.
 *
 * An application uses this function to compute the SHA384 digest and to
 * close the SHA384 context. This function calls @c free to free the context.
 *
 * @param ctx
 *    Points to the SHA384 context.
 * @param digest
 *    If this pointer is not NULL then it must point to a byte array that
 *    is big enough to hold the SHA384 digest (384 bit = 48 Bytes). If this
 *    pointer is NULL then the functions does not compute the digest but
 *    closes the context only. The context cannot be used anymore.
 */
void closeSha384Context(void* ctx, uint8_t* digest);

/**
 * Initialize a SHA384 context.
 *
 * An application uses this context to hash several data into one SHA384
 * digest. See also sha384Ctx(...) and closeSha384Context(...).
 *
 * @param ctx
 *    Points to the SHA384 context.
 * @return Returns the pointer to the initialized SHA384 context
 */
void* initializeSha384Context(void* ctx);

/**
 * Compute a digest.
 *
 * An application uses this function to compute the SHA384 digest.
 *
 * @param ctx
 *    Points to the SHA384 context.
 * @param digest
 *    If this pointer is not NULL then it must point to a byte array that
 *    is big enough to hold the SHA384 digest (384 bit = 48 Bytes). If this
 *    pointer is NULL then the functions does not compute the digest.
 */
void finalizeSha384Context(void* ctx, uint8_t* digest);

/**
 * Update the SHA384 context with data.
 *
 * This functions updates the SHA384 context with some data.
 * See also CloseSha384Context(...) how to get the digest.
 *
 * @param ctx
 *    Points to the SHA384 context.
 * @param data
 *    Points to the data to update the context.
 * @param dataLength
 *    The length of the data in bytes.
 */
void sha384Ctx(void* ctx, const uint8_t* data, uint64_t dataLength);

/**
 * Update the SHA384 context with several data chunks.
 *
 * This functions updates the SHA384 context with some data.
 * See also CloseSha384Context(...) how to get the digest.
 *
 * @param ctx
 *    Points to the SHA384 context.
 * @param data
 *    Vector of pointers that point to the data chunks. A NULL
 *    pointer in an array element terminates the data chunks.
 * @param dataLength
 *    Vector of integers that hold the length of each data chunk.
 *
 */
void sha384Ctx(void* ctx, const std::vector<const uint8_t*>& data, const std::vector<uint64_t>& dataLength);

/**
 * @}
 */
#endif

