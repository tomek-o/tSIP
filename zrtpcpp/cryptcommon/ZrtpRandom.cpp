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

#include <fcntl.h>

#include <cryptcommon/ZrtpRandom.h>
#include <cryptcommon/aescpp.h>
#include <zrtp/crypto/sha2.h>

#include <windows.h>
#include <Mutex.h>

static sha512_ctx mainCtx;

struct Mutex lockRandom;

static bool initialized = false;

/*
 * memset_volatile is a volatile pointer to the memset function.
 * You can call (*memset_volatile)(buf, val, len) or even
 * memset_volatile(buf, val, len) just as you would call
 * memset(buf, val, len), but the use of a volatile pointer
 * guarantees that the compiler will not optimise the call away.
 */
static void * (*volatile memset_volatile)(void *, int, size_t) = memset;

/*
 * Random bits are produced as follows.
 * First stir new entropy into the random state (zrtp->rand_ctx).
 * Then make a copy of the random context and finalize it.
 * Use the digest to seed an AES-256 context and, if space remains, to
 * initialize a counter.
 * Then encrypt the counter with the AES-256 context, incrementing it
 * per block, until we have produced the desired quantity of data.
 */
/*----------------------------------------------------------------------------*/
int ZrtpRandom::getRandomData(uint8_t* buffer, uint32_t length) {

    AESencrypt aesCtx;
    sha512_ctx randCtx2;
    uint8_t    md[SHA512_DIGEST_SIZE];
    uint8_t    ctr[AES_BLOCK_SIZE];
    uint8_t    rdata[AES_BLOCK_SIZE];
    uint32_t   generated = length;

    /*
     * Add entropy from system state
     * We will include whatever happens to be in the buffer, it can't hurt
     */
    lockRandom.lock();
    ZrtpRandom::addEntropy(buffer, length, true);

    /* Copy the mainCtx and finalize it into the md buffer */
    memcpy(&randCtx2, &mainCtx, sizeof(sha512_ctx));
    sha512_end(md, &randCtx2);

    lockRandom.unlock();

    /* Key an AES context from this buffer */
    aesCtx.key256(md);

    /* Initialize counter, using excess from md if available */
    memset (ctr, 0, sizeof(ctr));
    if (SHA512_DIGEST_SIZE > (256/8)) {
        uint32_t ctrbytes = SHA512_DIGEST_SIZE - (256/8);
        if (ctrbytes > AES_BLOCK_SIZE)
            ctrbytes = AES_BLOCK_SIZE;
        memcpy(ctr + sizeof(ctr) - ctrbytes, md + (256/8), ctrbytes);
    }

    /* Encrypt counter, copy to destination buffer, increment counter */
    while (length) {
        uint8_t *ctrptr;
        uint32_t copied;
        aesCtx.encrypt(ctr, rdata);
        copied = (sizeof(rdata) < length) ? sizeof(rdata) : length;
        memcpy (buffer, rdata, copied);
        buffer += copied;
        length -= copied;

        /* Increment counter */
        ctrptr = ctr + sizeof(ctr) - 1;
        while (ctrptr >= ctr) {
            if ((*ctrptr-- += 1) != 0) {
                break;
            }
        }
    }
    memset_volatile(&randCtx2, 0, sizeof(randCtx2));
    memset_volatile(md, 0, sizeof(md));
    memset_volatile(&aesCtx, 0, sizeof(aesCtx));
    memset_volatile(ctr, 0, sizeof(ctr));
    memset_volatile(rdata, 0, sizeof(rdata));

    return generated;
}


int ZrtpRandom::addEntropy(const uint8_t *buffer, uint32_t length, bool isLocked)
{

    uint8_t newSeed[64];
    size_t len = getSystemSeed(newSeed, sizeof(newSeed));

    if (!isLocked) lockRandom.lock();

    initialize();

    if (buffer && length) {
        sha512_hash(buffer, length, &mainCtx);
    }
    if (len > 0) {
        sha512_hash(newSeed, len, &mainCtx);
        length += len;
    }
    if (!isLocked) lockRandom.unlock();

    return length;
}


void ZrtpRandom::initialize() {
    if (initialized)
        return;

    sha512_begin(&mainCtx);
    initialized = true;
}

#if (defined(_WIN32) || defined(_WIN64))
#include <windows.h>
#include <wincrypt.h> /* CryptAcquireContext, CryptGenRandom */
static int RandBytes(void* const byte_buf, const size_t byte_len) {
  HCRYPTPROV p;
  ULONG     i;

  if (CryptAcquireContext(&p, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE) {
	return -1;
  }

  if (CryptGenRandom(p, byte_len, (BYTE*)byte_buf) == FALSE) {
	CryptReleaseContext(p, 0);
	return -2;
  }

  CryptReleaseContext(p, 0);
  return 0;
}
#endif

/*
 * This works for Linux and similar systems. For other systems add
 * other functions (using #ifdef conditional compile) to get some
 * random data that we can use as seed for the internal PRNG below.
 */

size_t ZrtpRandom::getSystemSeed(uint8_t *seed, size_t length)
{
    size_t num = 0;

#if !(defined(_WIN32) || defined(_WIN64))
	int rnd = open("/dev/urandom", O_RDONLY);
	if (rnd >= 0) {
		num = read(rnd, seed, length);
        close(rnd);
    }
    else
		return num;
#else
	if (RandBytes(seed, length) == 0)
		num = length;
#endif
    return num;
}

int zrtp_AddEntropy(const uint8_t *buffer, uint32_t length, int32_t isLocked) {
    return ZrtpRandom::addEntropy(buffer, length, isLocked);
}

int zrtp_getRandomData(uint8_t *buffer, uint32_t length) {
    return ZrtpRandom::getRandomData(buffer, length);
}
