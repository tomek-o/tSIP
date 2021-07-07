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

/*
 * Authors: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#ifndef _ZRTPPACKETBASE_H_
#define _ZRTPPACKETBASE_H_

/**
 * @file ZrtpPacketBase.h
 * @brief The ZRTP message header class
 *
 * This class defines the ZRTP message header and provides access and
 * check methods.
 *
 * @ingroup GNU_ZRTP
 * @{
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <common/osSpecifics.h>

#include <libzrtpcpp/zrtpPacket.h>
#include <libzrtpcpp/ZrtpTextData.h>
#include <libzrtpcpp/ZrtpConfigure.h>
#include <libzrtpcpp/ZrtpCrc32.h>

#include <mem.h>

// #define DEBUGOUT(deb)   deb
#define DEBUGOUT(deb)

/*
 * This is the unique ZRTP ID in network order (PZ)
 */
const uint16_t zrtpId = 0x505a;

/**
 * This is the base class for all ZRTP packets
 *
 * All other ZRTP packet classes inherit from this class. It does not have
 * an implementation of its own.
 *
 * The standard constructors of the subclasses usually initialize the @c allocate
 * field with their fixed data array which is large enough to hold all message
 * data. If an implementation needs to change this to use dynamic memory
 * allocation only that line in the subclasses must be changed and the destructors
 * should take care of memory management.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

class __EXPORT ZrtpPacketBase {

  private:

  protected:
      void* allocated;                  ///< Pointer to ZRTP message data
      zrtpPacketHeader_t* zrtpHeader;   ///< points to the fixed ZRTP header structure

  public:
    /**
     * Destructor is empty
     */
    virtual ~ZrtpPacketBase() {};

    /**
     * Get pointer to ZRTP header
     *
     * @return
     *     Pointer to ZRTP header structure.
     */
    const uint8_t* getHeaderBase() { return (const uint8_t*)zrtpHeader; };

    /**
     * Check is this is a ZRTP message
     *
     * @return
     *     @c true if check was ok
     */
    bool isZrtpPacket()            { return (zrtpNtohs(zrtpHeader->zrtpId) == zrtpId); };

    /**
     * Get the length in words of the ZRTP message
     *
     * @return
     *     The length in words
     */
    uint16_t getLength()           { return zrtpNtohs(zrtpHeader->length); };

    /**
     * Return pointer to fixed length message type ASCII data
     *
     * @return
     *     Pointer to ASCII character array
     */
    uint8_t* getMessageType()      { return zrtpHeader->messageType; };

    /**
     * Set the lenght field in the ZRTP header
     *
     * @param len
     *     The length of the ZRTP message in words, host order
     */
    void setLength(uint16_t len)  { zrtpHeader->length = zrtpHtons(len); };

    /**
     * Copy the message type ASCII data to ZRTP message type field
     *
     * @param msg
     *     Pointer to message type ASCII character array
     */
    void setMessageType(uint8_t *msg)
        { memcpy(zrtpHeader->messageType, msg, sizeof(zrtpHeader->messageType)); };

    /**
     * Initializes the ZRTP Id field
     */
    void setZrtpId()              { zrtpHeader->zrtpId = zrtpHtons(zrtpId); }
};

/**
 * @}
 */
#endif // ZRTPPACKETBASE
