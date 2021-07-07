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

#ifndef _ZRTPCRC32_H_
#define _ZRTPCRC32_H_

/**
 *
 * @file ZrtpCrc32.h
 * @brief Methods to compute the CRC32 checksum for ZRTP packets
 * 
 * @ingroup GNU_ZRTP
 * @{
 * 
 * @see ZrtpCallback
 */

/**
 * Check if a buffer matches a given CRC32 checksum.
 * 
 * @param buffer
 *     Pointer to the data buffer.
 * @param length
 *     Length in bytes of the data buffer.
 * @param crc32
 *     The CRC32 checksum.
 * 
 * @return
 *    @c true if the CRC32 checksum matches the computed checksum of the
 *    buffer, @c false otherwise.
 */
bool zrtpCheckCksum(const uint8_t *buffer, uint16_t length, uint32_t crc32);

/**
 * Generate a CRC32 checksum of a data buffer
 * 
 * @param buffer
 *    Pointer to the buffer.
 * @param length
 *     Lenght of the buffer in bytes.
 * 
 * @return
 *    A preliminary CRC32 checksum
 */
uint32_t zrtpGenerateCksum(const uint8_t *buffer, uint16_t length);

/**
 * Close CRC32 computation.
 * 
 * @param crc32
 *    A preliminary CRC32 checksum.
 * 
 * @return
 *    The ready to use CRC32 checksum in host order.
 */
uint32_t zrtpEndCksum(uint32_t crc32);

/**
 * @}
 */
#endif
