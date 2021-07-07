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

#ifndef _ZRTPPACKETPINGACK_H_
#define _ZRTPPACKETPINGACK_H_

#include <libzrtpcpp/ZrtpPacketBase.h>
/**
 * @file ZrtpPacketPingAck.h
 * @brief The ZRTP PingAck message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

/**
 * Implement the PingAck packet.
 *
 * The ZRTP simple message PingAck.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */
class __EXPORT ZrtpPacketPingAck : public ZrtpPacketBase {

 protected:
    PingAck_t* pingAckHeader;   ///< Points to PingAck message

 public:
    /// Creates a PingAck message with default data
    ZrtpPacketPingAck();

    /// Creates a PingAck message from received data
    ZrtpPacketPingAck(uint8_t* data);

    virtual ~ZrtpPacketPingAck();

    /// Get SSRC from PingAck message
    uint32_t getSSRC() { return zrtpNtohl(pingAckHeader->ssrc); };

    /// Set ZRTP protocol version field, fixed ASCII character array
    void setVersion(uint8_t *text)      { memcpy(pingAckHeader->version, text, ZRTP_WORD_SIZE ); }

    /// Set SSRC in PingAck message
    void setSSRC(uint32_t data)         {pingAckHeader->ssrc = zrtpHtonl(data); };

    /// Set remote endpoint hash, fixed byte array
    void setRemoteEpHash(uint8_t *hash) { memcpy(pingAckHeader->remoteEpHash, hash, sizeof(pingAckHeader->remoteEpHash)); }

    /// Set local endpoint hash, fixed byte array
    void setLocalEpHash(uint8_t *hash)  { memcpy(pingAckHeader->localEpHash, hash, sizeof(pingAckHeader->localEpHash)); }

 private:
     PingAckPacket_t data;
};

/**
 * @}
 */
#endif // ZRTPPACKETCLEARACK

