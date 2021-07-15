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
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#define MAKE_F8_TEST

#include <stdlib.h>
#include <crypto/SrtpSymCrypto.h>
#include <cryptcommon/twofish.h>
#include <cryptcommon/aesopt.h>
#include <string.h>
#include <stdio.h>
#include <common/osSpecifics.h>

SrtpSymCrypto::SrtpSymCrypto(int algo):key(NULL), algorithm(algo) {
}

SrtpSymCrypto::SrtpSymCrypto( uint8_t* k, int32_t keyLength, int algo):
    key(NULL), algorithm(algo) {

    setNewKey(k, keyLength);
}

SrtpSymCrypto::~SrtpSymCrypto() {
    if (key != NULL) {
        if (algorithm == SrtpEncryptionAESCM || algorithm == SrtpEncryptionAESF8) {
            AESencrypt *saAes = reinterpret_cast<AESencrypt*>(key);
            memset(saAes->cx, 0, sizeof(aes_encrypt_ctx));
            delete saAes;
        }
        else if (algorithm == SrtpEncryptionTWOCM || algorithm == SrtpEncryptionTWOF8) {
            memset(key, 0, sizeof(Twofish_key));
            delete[] (uint8_t*)key;
        }
        key = NULL;
    }
}

static int twoFishInit = 0;

bool SrtpSymCrypto::setNewKey(const uint8_t* k, int32_t keyLength) {
    // release an existing key before setting a new one
    if (key != NULL) {
        if (algorithm == SrtpEncryptionAESCM || algorithm == SrtpEncryptionAESF8) {
            AESencrypt *saAes = reinterpret_cast<AESencrypt*>(key);
            memset(saAes->cx, 0, sizeof(aes_encrypt_ctx));
            delete saAes;
        }
        else if (algorithm == SrtpEncryptionTWOCM || algorithm == SrtpEncryptionTWOF8) {
            memset(key, 0, sizeof(Twofish_key));
            delete[] (uint8_t*)key;
        }
        key = NULL;
    }

    if (!(keyLength == 16 || keyLength == 32)) {
        return false;
    }
    if (algorithm == SrtpEncryptionAESCM || algorithm == SrtpEncryptionAESF8) {
        AESencrypt *saAes = new AESencrypt();
        if (keyLength == 16)
            saAes->key128(k);
        else
            saAes->key256(k);
        key = saAes;
    }
    else if (algorithm == SrtpEncryptionTWOCM || algorithm == SrtpEncryptionTWOF8) {
        if (!twoFishInit) {
            Twofish_initialise();
            twoFishInit = 1;
        }
        key = new uint8_t[sizeof(Twofish_key)];
        memset(key, 0, sizeof(Twofish_key));
        Twofish_prepare_key((Twofish_Byte*)k, keyLength,  (Twofish_key*)key);
    }
    else
        return false;

    return true;
}

void SrtpSymCrypto::encrypt(const uint8_t* input, uint8_t* output) {
    if (algorithm == SrtpEncryptionAESCM || algorithm == SrtpEncryptionAESF8) {
        AESencrypt *saAes = reinterpret_cast<AESencrypt*>(key);
        saAes->encrypt(input, output);
    }
    else if (algorithm == SrtpEncryptionTWOCM || algorithm == SrtpEncryptionTWOF8) {
        Twofish_encrypt((Twofish_key*)key, (Twofish_Byte*)input,
                        (Twofish_Byte*)output); 
    }
}

void SrtpSymCrypto::get_ctr_cipher_stream(uint8_t* output, uint32_t length, uint8_t* iv) {
    uint32_t ctr = 0;
    unsigned char temp[SRTP_BLOCK_SIZE];

    for(ctr = 0; ctr < length/SRTP_BLOCK_SIZE; ctr++) {
        //compute the cipher stream
        iv[14] = (uint8_t)((ctr & 0xFF00) >>  8);
        iv[15] = (uint8_t)((ctr & 0x00FF));

        encrypt(iv, &output[ctr*SRTP_BLOCK_SIZE]);
    }
    if ((length % SRTP_BLOCK_SIZE) > 0) {
        // Treat the last bytes:
        iv[14] = (uint8_t)((ctr & 0xFF00) >>  8);
        iv[15] = (uint8_t)((ctr & 0x00FF));

        encrypt(iv, temp);
        memcpy(&output[ctr*SRTP_BLOCK_SIZE], temp, length % SRTP_BLOCK_SIZE );
    }
}

void SrtpSymCrypto::ctr_encrypt(const uint8_t* input, uint32_t input_length, uint8_t* output, uint8_t* iv) {

    if (key == NULL)
        return;

    uint16_t ctr = 0;
    unsigned char temp[SRTP_BLOCK_SIZE];

    int l = input_length/SRTP_BLOCK_SIZE;
    for (ctr = 0; ctr < l; ctr++ ) {
        iv[14] = (uint8_t)((ctr & 0xFF00) >>  8);
        iv[15] = (uint8_t)((ctr & 0x00FF));

        encrypt(iv, temp);
        for (int i = 0; i < SRTP_BLOCK_SIZE; i++ ) {
            *output++ = temp[i] ^ *input++;
        }

    }
    l = input_length % SRTP_BLOCK_SIZE;
    if (l > 0) {
        // Treat the last bytes:
        iv[14] = (uint8_t)((ctr & 0xFF00) >>  8);
        iv[15] = (uint8_t)((ctr & 0x00FF));

        encrypt(iv, temp);
        for (int i = 0; i < l; i++ ) {
            *output++ = temp[i] ^ *input++;
        }
    }
}

void SrtpSymCrypto::ctr_encrypt( uint8_t* data, uint32_t data_length, uint8_t* iv ) {

    if (key == NULL)
        return;

    uint16_t ctr = 0;
    unsigned char temp[SRTP_BLOCK_SIZE];

    int l = data_length/SRTP_BLOCK_SIZE;
    for (ctr = 0; ctr < l; ctr++ ) {
        iv[14] = (uint8_t)((ctr & 0xFF00) >>  8);
        iv[15] = (uint8_t)((ctr & 0x00FF));

        encrypt(iv, temp);
        for (int i = 0; i < SRTP_BLOCK_SIZE; i++ ) {
            *data++ ^= temp[i];
        }

    }
    l = data_length % SRTP_BLOCK_SIZE;
    if (l > 0) {
        // Treat the last bytes:
        iv[14] = (uint8_t)((ctr & 0xFF00) >>  8);
        iv[15] = (uint8_t)((ctr & 0x00FF));

        encrypt(iv, temp);
        for (int i = 0; i < l; i++ ) {
            *data++ ^= temp[i];
        }
    }
}

void SrtpSymCrypto::f8_encrypt(const uint8_t* data, uint32_t data_length,
                         uint8_t* iv, SrtpSymCrypto* f8Cipher ) {

    f8_encrypt(data, data_length, const_cast<uint8_t*>(data), iv, f8Cipher);
}

#define MAX_KEYLEN 32

void SrtpSymCrypto::f8_deriveForIV(SrtpSymCrypto* f8Cipher, uint8_t* key, int32_t keyLen,
             uint8_t* salt, int32_t saltLen) {

    unsigned char *cp_in, *cp_in1, *cp_out;

    unsigned char maskedKey[MAX_KEYLEN];
    unsigned char saltMask[MAX_KEYLEN];

    if (keyLen > MAX_KEYLEN)
        return;

    if (saltLen > keyLen)
        return;
    /*
     * First copy the salt into the mask field, then fill with 0x55 to
     * get a full key.
     */
    memcpy(saltMask, salt, saltLen);
    memset(saltMask+saltLen, 0x55, keyLen-saltLen);

    /*
     * XOR the original key with the above created mask to
     * get the special key.
     */
    cp_out = maskedKey;
    cp_in = key;
    cp_in1 = saltMask;
    for (int i = 0; i < keyLen; i++) {
        *cp_out++ = *cp_in++ ^ *cp_in1++;
    }
    /*
     * Prepare the a new AES cipher with the special key to compute IV'
     */
    f8Cipher->setNewKey(maskedKey, keyLen);
}

void SrtpSymCrypto::f8_encrypt(const uint8_t* in, uint32_t in_length, uint8_t* out,
                         uint8_t* iv, SrtpSymCrypto* f8Cipher ) {


    int offset = 0;

    unsigned char ivAccent[SRTP_BLOCK_SIZE];
    unsigned char S[SRTP_BLOCK_SIZE];

    F8_CIPHER_CTX f8ctx;

    if (key == NULL)
        return;
    /*
     * Get memory for the derived IV (IV')
     */
    f8ctx.ivAccent = ivAccent;
    /*
     * Use the derived IV encryption setup to encrypt the original IV to produce IV'.
     */
    f8Cipher->encrypt(iv, f8ctx.ivAccent);

    f8ctx.J = 0;                       // initialize the counter
    f8ctx.S = S;               // get the key stream buffer

    memset(f8ctx.S, 0, SRTP_BLOCK_SIZE); // initial value for key stream

    while (in_length >= SRTP_BLOCK_SIZE) {
        processBlock(&f8ctx, in+offset, SRTP_BLOCK_SIZE, out+offset);
        in_length -= SRTP_BLOCK_SIZE;
        offset += SRTP_BLOCK_SIZE;
    }
    if (in_length > 0) {
        processBlock(&f8ctx, in+offset, in_length, out+offset);
    }
}

int SrtpSymCrypto::processBlock(F8_CIPHER_CTX *f8ctx, const uint8_t* in, int32_t length, uint8_t* out) {

    int i;
    const uint8_t *cp_in;
    uint8_t* cp_in1, *cp_out;
    uint32_t *ui32p;

    /*
     * XOR the previous key stream with IV'
     * ( S(-1) xor IV' )
     */
    cp_in = f8ctx->ivAccent;
    cp_out = f8ctx->S;
    for (i = 0; i < SRTP_BLOCK_SIZE; i++) {
        *cp_out++ ^= *cp_in++;
    }
    /*
     * Now XOR (S(n-1) xor IV') with the current counter, then increment the counter
     */
    ui32p = (uint32_t *)f8ctx->S;
    ui32p[3] ^= zrtpHtonl(f8ctx->J);
    f8ctx->J++;
    /*
     * Now compute the new key stream using AES encrypt
     */
    encrypt(f8ctx->S, f8ctx->S);
    /*
     * as the last step XOR the plain text with the key stream to produce
     * the ciphertext.
     */
    cp_out = out;
    cp_in = in;
    cp_in1 = f8ctx->S;
    for (i = 0; i < length; i++) {
        *cp_out++ = *cp_in++ ^ *cp_in1++;
    }
    return length;
}

