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

#ifndef _ZRTPRANDOM_H_
#define _ZRTPRANDOM_H_

/**
 * @file ZrtpCommon.h
 * @brief ZRTP standalone random number generator
 * @defgroup GNU_ZRTP The GNU ZRTP C++ implementation
 * @{
 */

#include <string.h>
#if !(defined(_WIN32) || defined(_WIN64))
#include <unistd.h>
#endif
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
class ZrtpRandom {
public:
    /**
     * @brief This method adds entropy to the PRNG.
     *
     * An application may seed some entropy data to the PRNG. If the @c buffer is
     * @c NULL or the @c length is zero then the method adds at least some system
     * entropy.
     *
     * @param buffer some entropy data to add
     * @param length length of entropy data in bytes
     * @param isLocked Set to true when randomLock mutex is already set.
     *
     * @return on success: number of entropy bytes added, on failure: -1. Number of
     *         bytes added may be bigger then @c length because of added system
     *         entropy.
     */
    static int addEntropy(const uint8_t *buffer, uint32_t length, bool isLocked = false);

    /**
     * @brief Get some random data.
     *
     * @param buffer that will contain the random data
     * @param length how many bytes of random data to generate
     * @return the number of generated random data bytes
     */
    static int getRandomData(uint8_t *buffer, uint32_t length);

private:
    static void initialize();
    static size_t getSystemSeed(uint8_t *seed, size_t length);

};
#endif

#ifdef __cplusplus
extern "C"
{
#endif

int zrtp_AddEntropy(const uint8_t *buffer, uint32_t length, int32_t isLocked);

int zrtp_getRandomData(uint8_t *buffer, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* ZRTPRANDOM */