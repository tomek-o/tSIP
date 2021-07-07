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

#ifndef _ZRTPPACKETERRORACK_H_
#define _ZRTPPACKETERRORACK_H_

/**
 * @file ZrtpPacketErrorAck.h
 * @brief The ZRTP ErrorAck message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the ErrorAck packet.
 *
 * The ZRTP simple message ErrorAck. The implementation sends this
 * after receiving and checking the Error message.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */
class __EXPORT ZrtpPacketErrorAck : public ZrtpPacketBase {

 public:
    /// Creates a ErrorAck packet with default data
    ZrtpPacketErrorAck();

    /// Creates a ErrorAck packet from received data
    ZrtpPacketErrorAck(uint8_t* data);
    virtual ~ZrtpPacketErrorAck();

 private:
     ErrorAckPacket_t data;
};

/**
 * @}
 */
#endif  // _ZRTPPACKETERRORACK_H_
