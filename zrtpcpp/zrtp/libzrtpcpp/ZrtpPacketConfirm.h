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
#ifndef _ZRTPPACKETCONFIRM_H_
#define _ZRTPPACKETCONFIRM_H_

/**
 * @file ZrtpPacketConfirm.h
 * @brief The ZRTP Confirm message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the Confirm packet.
 *
 * The ZRTP message Confirm. The implementation sends this
 * to confirm the switch to SRTP (encrypted) mode. The contents of
 * the Confirm message are encrypted, thus the implementation
 * can check if the secret keys work.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

class __EXPORT ZrtpPacketConfirm : public ZrtpPacketBase {

    private:
        Confirm_t* confirmHeader;   ///< Point to the Confirm message part

    public:
        /// Creates a Confirm packet with default data
        ZrtpPacketConfirm();

        /// Creates a Confirm packet with default data and a given signature length
        ZrtpPacketConfirm(uint32_t sl);

        /// Creates a Confirm packet from received data
        ZrtpPacketConfirm(uint8_t* d);

        /// Normal destructor
        virtual ~ZrtpPacketConfirm();

        /// Check if SAS verify flag is set
        const bool isSASFlag()            { return (confirmHeader->flags & 0x4) == 0x4; }

        /// Check if Disclosure flag is set
        const bool isDisclosureFlag()     { return (confirmHeader->flags & 0x1) == 0x1; }

        /// Check if PBXEnrollment flag is set
        const bool isPBXEnrollment()      { return (confirmHeader->flags & 0x8) == 0x8; }

        /// Get pointer to filler bytes (contains one bit of signature length)
        const uint8_t* getFiller()        { return confirmHeader->filler; }

        /// Get pointer to IV data, fixed byte array
        const uint8_t* getIv()            { return confirmHeader->iv; }

        /// Get pointer to MAC data, fixed byte array
        const uint8_t* getHmac()          { return confirmHeader->hmac; }

        /// Get Expiration time data
        const uint32_t getExpTime()       { return zrtpNtohl(confirmHeader->expTime); }

        /// Get pointer to initial hash chain (H0) data, fixed byte array
        uint8_t* getHashH0()              { return confirmHeader->hashH0; }

        /// Get pointer to signature data, variable length, refer to getSignatureLength()
        const uint8_t* getSignatureData() { return ((uint8_t*)&confirmHeader->expTime) + 4; }

        /// get the signature length in words
        int32_t getSignatureLength();

        /// Check if packet length makes sense. Confirm packets are 19 words at minumum
        bool isLengthOk()             {return (getLength() >= 19); }

        bool isSignatureLengthOk();

        /// set SAS verified flag
        void setSASFlag()            { confirmHeader->flags |= 0x4; }

        /// set Disclosure flag
        void setDisclosureFlag()     { confirmHeader->flags |= 0x1; }

        /// set setPBXEnrollment flag
        void setPBXEnrollment()      { confirmHeader->flags |= 0x8; }

        /// Set MAC data, fixed length byte array
        void setHmac(uint8_t* text)  { memcpy(confirmHeader->hmac, text, sizeof(confirmHeader->hmac)); }

        /// Set IV data, fixed length byte array
        void setIv(uint8_t* text)    { memcpy(confirmHeader->iv, text, sizeof(confirmHeader->iv)); }

        /// Set expiration time data
        void setExpTime(uint32_t t)  { confirmHeader->expTime = zrtpHtonl(t); }

        /// Set initial hash chain (H0) data, fixed length byte array
        void setHashH0(uint8_t* t)   { memcpy(confirmHeader->hashH0, t, sizeof(confirmHeader->hashH0)); }

        /// Set signature data, length of the signature data in bytes and must be a multiple of 4.
        bool setSignatureData(uint8_t* data, int32_t length);

        /// Set signature length in words
        bool setSignatureLength(uint32_t sl);

    private:
        void initialize();
     // Confirm packet is of variable length. It maximum size is 524 words:
     // - 11 words fixed size
     // - up to 513 words variable part, depending if signature is present and its length.
     // This leads to a maximum of 4*524=2096 bytes.
        uint8_t data[2100];       // large enough to hold a full blown Confirm packet

};

/**
 * @}
 */
#endif // ZRTPPACKETCONFIRM

