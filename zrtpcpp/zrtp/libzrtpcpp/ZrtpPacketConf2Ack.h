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

#ifndef _ZRTPPACKETCON2FACK_H_
#define _ZRTPPACKETCON2FACK_H_

/**
 * @file ZrtpPacketConf2Ack.h
 * @brief The ZRTP Conf2Ack message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the Conf2Ack packet.
 *
 * The ZRTP simple message Conf2Ack. The implementation sends this
 * after receiving and checking the Confirm2 message.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

class __EXPORT ZrtpPacketConf2Ack : public ZrtpPacketBase {

 public:
    /// Creates a Conf2Ack packet with default data
    ZrtpPacketConf2Ack();

    ///Creates a Conf2Ack packet from received data
    ZrtpPacketConf2Ack(char* data);

    /// Normal destructor
    virtual ~ZrtpPacketConf2Ack();

 private:
     Conf2AckPacket_t data;
};

/**
 * @}
 */
#endif // ZRTPPACKETCONF2ACK

