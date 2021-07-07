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

#ifndef _ZRTPPACKETPING_H_
#define _ZRTPPACKETPING_H_

/**
 * @file ZrtpPacketPing.h
 * @brief The ZRTP Ping message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the PingAck packet.
 *
 * The ZRTP simple message PingAck.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */
class __EXPORT ZrtpPacketPing : public ZrtpPacketBase {

 protected:
    Ping_t* pingHeader;     ///< Point the the Ping message

 public:
    /// Creates a Ping message with default data
    ZrtpPacketPing();

    /// Creates a Ping message from received data
    ZrtpPacketPing(uint8_t* data);

    virtual ~ZrtpPacketPing();

    /// Set ZRTP protocol version field, fixed ASCII character array
    void setVersion(uint8_t *text)     { memcpy(pingHeader->version, text,ZRTP_WORD_SIZE ); }

    /// Get the endpoit hash, fixed byte array
    uint8_t* getEpHash()               { return pingHeader->epHash; }

 private:
     PingPacket_t data;
};

/**
 * @}
 */

#endif // ZRTPPACKETCLEARACK

