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


#ifndef MAC_SKEIN_H
#define MAC_SKEIN_H

#include <cryptcommon/skeinApi.h>
#include <vector>
/**
 * @file macSkein.h
 * @brief Function that provide Skein MAC support
 * 
 *
 * Functions to compute Skein MAC.
 *
 * @ingroup GNU_ZRTP
 * @{
 */

/**
 * Compute Skein MAC.
 *
 * This functions takes one data chunk and computes its Skein MAC.
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
 *    Points to a buffer that receives the computed digest.
 * @param macLength
 *    Integer that contains the length of the MAC in bits (not bytes).
 * @param skeinSize
 *    The Skein size to use.
 */
void macSkein(const uint8_t* key, uint64_t keyLength,
              const uint8_t* data, uint64_t dataLength,
              uint8_t* mac, size_t macLength, SkeinSize_t skeinSize );

/**
 * Compute Skein MAC over several data chunks.
 *
 * This functions takes several data chunk and computes the Skein MAC.
 *
 * @param key
 *    The MAC key.
 * @param keyLength
 *    Lneght of the MAC key in bytes
 * @param data
 *    A Vector of pointers that point to the data chunks. A NULL
 *    pointer in an array element terminates the data chunks.
 * @param dataLength
 *    A vector of integers that hold the length of each data chunk.
 * @param mac
 *    Points to a buffer that receives the computed digest.
 * @param macLength
 *    Integer that contains the length of the MAC in bits (not bytes).
 * @param skeinSize
 *    The Skein size to use.
 */
void macSkein(const uint8_t* key, uint64_t keyLength,
              std::vector<const uint8_t*> data,
              std::vector<uint64_t> dataLength,
              uint8_t* mac, size_t macLength, SkeinSize_t skeinSize);

/**
 * Create and initialize a Skein MAC context.
 *
 * An application uses this context to hash several data with on Skein MAC
 * Context with the same key, key length and mac length
 *
 * @param key
 *    The MAC key.
 * @param keyLength
 *    Length of the MAC key in bytes
 * @param macLength
 *    Integer that contains the length of the MAC in bits (not bytes).
 * @param skeinSize
 *    The Skein size to use.
 * @return Returns a pointer to the initialized context or @c NULL in case of an error.
 */
void* createSkeinMacContext(const uint8_t* key, uint64_t keyLength, size_t macLength, SkeinSize_t skeinSize);

/**
 * Initialize a Skein MAC context.
 *
 * An application uses this context to hash several data with on Skein MAC
 * Context with the same key, key length and mac length
 *
 * @param ctx
 *     Pointer to initialized Skein MAC context
 * @param key
 *    The MAC key.
 * @param keyLength
 *    Length of the MAC key in bytes
 * @param macLength
 *    Integer that contains the length of the MAC in bits (not bytes).
 * @param skeinSize
 *    The Skein size to use.
 * @return Returns a pointer to the initialized context
 */
void* initializeSkeinMacContext(void* ctx, const uint8_t* key, uint64_t keyLength, size_t macLength, SkeinSize_t skeinSize);

/**
 * Compute Skein MAC.
 *
 * This functions takes one data chunk and computes its Skein MAC.
 *
 * @param ctx
 *     Pointer to initialized Skein MAC context
 * @param data
 *    Points to the data chunk.
 * @param dataLength
 *    Length of the data in bytes
 * @param mac
 *    Points to a buffer that receives the computed digest.
 */

void macSkeinCtx(void* ctx, const uint8_t* data, uint64_t dataLength, uint8_t* mac);

/**
 * Compute Skein MAC over several data cunks.
 *
 * This functions takes several data chunk and computes the SHA1 HAMAC.
 *
 * @param ctx 
 *     Pointer to initialized Skein MAC context
 * @param data
 *    Points to an array of pointers that point to the data chunks. A NULL
 *    pointer in an array element terminates the data chunks.
 * @param dataLength
 *    Points to an array of integers that hold the length of each data chunk.
 * @param mac
 *    Points to a buffer that receives the computed digest.
 */
void macSkeinCtx(void* ctx, const std::vector<const uint8_t*>& data,
                 const std::vector<uint64_t>& dataLength,
                 uint8_t* mac);

/**
 * Free Skein MAC context.
 *
 * @param ctx a pointer to Skein MAC context
 */
void freeSkeinMacContext(void* ctx);

/**
 * @}
 */
#endif