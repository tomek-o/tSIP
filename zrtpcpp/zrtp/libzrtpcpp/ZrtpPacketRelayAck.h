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

#ifndef _ZRTPPACKETRELAYACK_H_
#define _ZRTPPACKETRELAYACK_H_

/**
 * @file ZrtpPacketRelayAck.h
 * @brief The ZRTP RelayAck message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the RelayAck packet.
 *
 * The ZRTP simple message RelayAck. The implementation sends this
 * after receiving and checking the SASrelay message.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */
class __EXPORT ZrtpPacketRelayAck : public ZrtpPacketBase {

 public:
    /// Creates a RelayAck packet with default data
    ZrtpPacketRelayAck();

    /// Creates a RelayAck packet from received data
    ZrtpPacketRelayAck(uint8_t* data);
    virtual ~ZrtpPacketRelayAck();

 private:
     RelayAckPacket_t data;
};

/**
 * @}
 */
#endif  // _ZRTPPACKETRELAYACK_H_
