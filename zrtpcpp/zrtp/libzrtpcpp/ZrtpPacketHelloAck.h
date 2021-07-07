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

#ifndef _ZRTPPACKETHELLOACK_H_
#define _ZRTPPACKETHELLOACK_H_

/**
 * @file ZrtpPacketHelloAck.h
 * @brief The ZRTP HelloAck message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the HelloAck packet.
 *
 * The ZRTP simple message HelloAck. The implementation sends this
 * after receiving a Hello packet.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

class __EXPORT ZrtpPacketHelloAck : public ZrtpPacketBase {

 public:
    /// Creates a HelloAck packet with default data
    ZrtpPacketHelloAck();

    /// Creates a HelloAck packet from received data
    ZrtpPacketHelloAck(uint8_t* data);

    virtual ~ZrtpPacketHelloAck();

 private:
     HelloAckPacket_t data;
};

/**
 * @}
 */
#endif // ZRTPPACKETHELLOACK

