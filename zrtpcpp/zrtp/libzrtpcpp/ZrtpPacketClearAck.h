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

#ifndef _ZRTPPACKETCLEARACK_H_
#define _ZRTPPACKETCLEARACK_H_

/**
 * @file ZrtpPacketClearAck.h
 * @brief The ZRTP ClearAck message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the ClearAck packet - Currently not used
 *
 * The ZRTP simple message ClearAck. The implementation sends this
 * after switching to clear mode (non-SRTP mode).
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */
class __EXPORT ZrtpPacketClearAck : public ZrtpPacketBase {

 public:
    ZrtpPacketClearAck();       /// Creates a ClearAck packet with default data
    ZrtpPacketClearAck(uint8_t* data);  /// Creates a ClearAck packet from received data
    virtual ~ZrtpPacketClearAck();

 private:
     ClearAckPacket_t data;
};

/**
 * @}
 */
#endif // ZRTPPACKETCLEARACK

