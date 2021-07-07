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
 * Methods to compute a Skein384 HMAC.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef HMAC_SKEIN384_H
#define HMAC_SKEIN384_H

/**
 * @file skeinMac384.h
 * @brief Function that provide Skein384 HMAC support
 * 
 * @ingroup GNU_ZRTP
 * @{
 */

#include <stdint.h>
#include <vector>

#ifndef SKEIN384_DIGEST_LENGTH
#define SKEIN384_DIGEST_LENGTH 48
#endif

#define SKEIN_SIZE Skein512

/**
 * Compute Skein384 HMAC.
 *
 * This functions takes one data chunk and computes its Skein384 HMAC.
 *
 * @param key
 *    The MAC key.
 * @param key_length
 *    Lneght of the MAC key in bytes
 * @param data
 *    Points to the data chunk.
 * @param dataLength
 *    Length of the data in bytes
 * @param mac
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 48 bytes (SKEIN384_DIGEST_LENGTH).
 * @param macLength
 *    Point to an uint32_t that receives the length of the computed HMAC.
 */
void macSkein384(const uint8_t* key, uint64_t key_length, const uint8_t* data, uint64_t dataLength, uint8_t* mac, uint32_t* macLength);

/**
 * Compute Skein384 HMAC over several data cunks.
 *
 * This functions takes several data chunk and computes the Skein384 HAMAC.
 *
 * @param key
 *    The MAC key.
 * @param keyLength
 *    Lneght of the MAC key in bytes
 * @param data
 *    Vector of pointers that point to the data chunks. A NULL
 *    pointer in an array element terminates the data chunks.
 * @param dataLength
 *    Vector of integers that hold the length of each data chunk.
 * @param mac
 *    Points to a buffer that receives the computed digest. This
 *    buffer must have a size of at least 48 bytes (SKEIN384_DIGEST_LENGTH).
 * @param macLength
 *    Pointer to an uint32_t that receives the length of the computed HMAC.
 */
void macSkein384(const uint8_t* key, uint64_t keyLength,
                 const std::vector<const uint8_t*>& data,
                 const std::vector<uint64_t>& dataLength,
                 uint8_t* mac, uint32_t* mac_length);
/**
 * @}
 */
#endif

#pragma clang diagnostic pop