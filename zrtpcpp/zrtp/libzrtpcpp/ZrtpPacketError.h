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

#ifndef _ZRTPPACKETERROR_H_
#define _ZRTPPACKETERROR_H_

/**
 * @file ZrtpPacketError.h
 * @brief The ZRTP Error message
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpPacketBase.h>

/**
 * Implement the Error packet.
 *
 * The ZRTP simple message Error. The implementation sends this
 * after detecting an error.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

class __EXPORT ZrtpPacketError : public ZrtpPacketBase {

 protected:
    Error_t* errorHeader;   ///< Points to Error message

 public:
    /// Creates a Error packet with default data
    ZrtpPacketError();

    /// Creates a Error packet from received data
    ZrtpPacketError(uint8_t* data);

    virtual ~ZrtpPacketError();

    /// Get the error code from Error message
    uint32_t getErrorCode() { return zrtpNtohl(errorHeader->errorCode); };

    /// Set error code in Error message
    void setErrorCode(uint32_t code) {errorHeader->errorCode = zrtpHtonl(code); };

 private:
     ErrorPacket_t data;
};

/**
 * @}
 */
#endif // ZRTPPACKETERROR

