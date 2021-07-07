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

#ifndef _ZRTPPACKETGOCLEAR_H_
#define _ZRTPPACKETGOCLEAR_H_

/**
 * @file ZrtpPacketGoClear.h
 * @brief The ZRTP GoClear message
 *
 * GNU ZRTP does not implement GoClear feature
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the GoClear packet.
 *
 * The ZRTP message GoClear. The implementation sends this
 * to order the peer to switch to clear mode (non-SRTP mode).
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

class __EXPORT ZrtpPacketGoClear : public ZrtpPacketBase {

 protected:
    GoClear_t* clearHeader;

 public:
    /// Creates a GoCLear packet with default data
    ZrtpPacketGoClear();

    /// Creates a GoClear packet from received data
    ZrtpPacketGoClear(uint8_t* data);

    virtual ~ZrtpPacketGoClear();

    /// Not used
    const uint8_t* getClearHmac() { return clearHeader->clearHmac; };

    /// Not used
    void setClearHmac(uint8_t *text) { memcpy(clearHeader->clearHmac, text, 32); };

    /// Not used
    void clrClearHmac()              { memset(clearHeader->clearHmac, 0, 32); };

 private:
     GoClearPacket_t data;
};

/**
 * @}
 */
#endif // ZRTPPACKETGOCLEAR

