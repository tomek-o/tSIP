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

#ifndef _ZRTPPACKETDHPART_H_
#define _ZRTPPACKETDHPART_H_

/**
 * @file ZrtpPacketDHPart.h
 * @brief The ZRTP DHPart message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the DHPart packet.
 *
 * The ZRTP message DHPart. The implementation sends this
 * to exchange the Diffie-Helman public keys and the shared
 * secrets between the two parties.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

class __EXPORT ZrtpPacketDHPart : public ZrtpPacketBase {

 protected:
    uint8_t *pv;                ///< points to public key value inside DH message
    DHPart_t* DHPartHeader;     ///< points to DH message structure
    int32_t dhLength;           ///< length of DH message, DH message has variable length

 public:
    /// Creates a DHPart packet no data, must use setPubKeyType(...)
    ZrtpPacketDHPart();

    /// Creates a DHPart packet with default data and a give public key type
    ZrtpPacketDHPart(const char* pkt);

    /// Creates a DHPart packet from received data
    ZrtpPacketDHPart(uint8_t* data);

    /// Standard destructor
    virtual ~ZrtpPacketDHPart();

    /// Get pointer to public key value, variable length byte array
    uint8_t* getPv()             { return pv; }

    /// Get pointer to first retained secretd id, fixed length byte array
    uint8_t* getRs1Id()          { return DHPartHeader->rs1Id; };

    /// Get pointer to second retained secretd id, fixed length byte array
    uint8_t* getRs2Id()          { return DHPartHeader->rs2Id; };

    /// Get pointer to additional retained secretd id, fixed length byte array
    uint8_t* getAuxSecretId()    { return DHPartHeader->auxSecretId; };

    /// Get pointer to PBX retained secretd id, fixed length byte array
    uint8_t* getPbxSecretId()    { return DHPartHeader->pbxSecretId; };

    /// Get pointer to first hash (H1) for hash chain, fixed length byte array
    uint8_t* getH1()             { return DHPartHeader->hashH1; };

    /// Get pointer to HMAC, fixed length byte array
    uint8_t* getHMAC()           { return pv+dhLength; };

    /// Check if packet length makes sense. DHPart packets are 29 words at minumum, using E255
    bool isLengthOk()            {return (getLength() >= 29);}

    /// Setpublic key value, variable length byte array
    void setPv(uint8_t* text)         { memcpy(pv, text, dhLength); };

    /// Set first retained secretd id, fixed length byte array
    void setRs1Id(uint8_t* text)      { memcpy(DHPartHeader->rs1Id, text, sizeof(DHPartHeader->rs1Id)); };

    /// Set second retained secretd id, fixed length byte array
    void setRs2Id(uint8_t* text)      { memcpy(DHPartHeader->rs2Id, text, sizeof(DHPartHeader->rs2Id)); };

    /// Set additional retained secretd id, fixed length byte array
    void setAuxSecretId(uint8_t* t)   { memcpy(DHPartHeader->auxSecretId, t, sizeof(DHPartHeader->auxSecretId)); };

    /// Set PBX retained secretd id, fixed length byte array
    void setPbxSecretId(uint8_t* t)   { memcpy(DHPartHeader->pbxSecretId,t, sizeof(DHPartHeader->pbxSecretId)); };

    /// Set first hash (H1) of hash chain, fixed length byte array
    void setH1(uint8_t* t)            { memcpy(DHPartHeader->hashH1, t, sizeof(DHPartHeader->hashH1)); };

    /// Set key agreement type, fixed size character array
    void setPubKeyType(const char* pkt);

    /// Set first MAC, fixed length byte array
    void setHMAC(uint8_t* t)          { memcpy(pv+dhLength, t, 2*ZRTP_WORD_SIZE); };

 private:
    void initialize();
    // SupportedPubKeys pktype;
     // DHPart packet is of variable length. It maximum size is 141 words:
     // - 13 words fixed sizze
     // - up to 128 words variable part, depending on DH algorithm
     //   leads to a maximum of 4*141=564 bytes.
     uint8_t data[768];       // large enough to hold a full blown DHPart packet
};

/**
 * @}
 */
#endif // ZRTPPACKETDHPART

