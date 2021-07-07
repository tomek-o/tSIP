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
 * Authors: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#ifndef _TWOCFB_H__
#define _TWOCFB_H__

#include <stdint.h>

/**
 * @file aesCFB.h
 * @brief Function that provide AES CFB mode support
 * 
 * @ingroup GNU_ZRTP
 * @{
 */

#ifndef TWO_BLOCK_SIZE
#define TWO_BLOCK_SIZE 16
#endif

/**
 * Encrypt data with Twofish CFB mode, full block feedback size.
 *
 * This functions takes one data chunk and encrypts it with
 * Twofish CFB mode. The lenght of the data may be arbitrary and
 * it is not needed to be a multiple of Twofish blocksize.
 *
 * @param key
 *    Points to the key bytes.
 * @param keyLength
 *    Length of the key in bytes
 * @param IV
 *    The initialization vector which must be TWO_BLOCKSIZE (16) bytes.
 * @param data
 *    Points to a buffer that contains and receives the computed
 *    the data (in-place encryption).
 * @param dataLength
 *    Length of the data in bytes
 */

void twoCfbEncrypt(uint8_t* key, int32_t keyLength, uint8_t* IV, uint8_t *data, int32_t dataLength);

/**
 * Decrypt data with Twofish CFB mode, full block feedback size.
 *
 * This functions takes one data chunk and decrypts it with
 * Twofish CFB mode. The lenght of the data may be arbitrary and
 * it is not needed to be a multiple of Twofish blocksize.
 *
 * @param key
 *    Points to the key bytes.
 * @param keyLength
 *    Length of the key in bytes
 * @param IV
 *    The initialization vector which must be TWO_BLOCKSIZE (16) bytes.
 * @param data
 *    Points to a buffer that contains and receives the computed
 *    the data (in-place decryption).
 * @param dataLength
 *    Length of the data in bytes
 */

void twoCfbDecrypt(uint8_t* key, int32_t keyLength, uint8_t* IV, uint8_t *data, int32_t dataLength);
/**
 * @}
 */
#endif
