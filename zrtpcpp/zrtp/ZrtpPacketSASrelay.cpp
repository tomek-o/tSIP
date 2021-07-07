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

#include <libzrtpcpp/ZrtpPacketSASrelay.h>

ZrtpPacketSASrelay::ZrtpPacketSASrelay() {
    DEBUGOUT((fprintf(stdout, "Creating SASrelay packet without data, no sl data\n")));
    initialize();
    setSignatureLength(0);
}

ZrtpPacketSASrelay::ZrtpPacketSASrelay(uint32_t sl) {
    DEBUGOUT((fprintf(stdout, "Creating SASrelay packet without data\n")));
    initialize();
    setSignatureLength(sl);
}

void ZrtpPacketSASrelay::initialize() {
    void* allocated = &data;
    memset(allocated, 0, sizeof(data));

    zrtpHeader = (zrtpPacketHeader_t *)&((SASrelayPacket_t *)allocated)->hdr;	// the standard header
    sasRelayHeader = (SASrelay_t *)&((SASrelayPacket_t *)allocated)->sasrelay;

    setZrtpId();
    setMessageType((uint8_t*)SasRelayMsg);
}

void ZrtpPacketSASrelay::setSignatureLength(uint32_t sl) {
    sl &= 0x1ff;                                                       // make sure it is max 9 bits
    int32_t length = sizeof(ConfirmPacket_t) + (sl * ZRTP_WORD_SIZE);
    sasRelayHeader->sigLength = sl;                                     // sigLength is a uint byte
    if (sl & 0x100) {                                                  // check the 9th bit
        sasRelayHeader->filler[1] = 1;                                  // and set it if necessary
    }
    setLength(length / 4);
}

uint32_t ZrtpPacketSASrelay::getSignatureLength() {
    uint32_t sl = sasRelayHeader->sigLength;
    if (sasRelayHeader->filler[1] == 1) {                              // do we have a 9th bit
        sl |= 0x100;
    }
    return sl;
}

ZrtpPacketSASrelay::ZrtpPacketSASrelay(uint8_t* data) {
    DEBUGOUT((fprintf(stdout, "Creating SASrelay packet from data\n")));

    allocated = NULL;
    zrtpHeader = (zrtpPacketHeader_t *)&((SASrelayPacket_t *)data)->hdr;	// the standard header
    sasRelayHeader = (SASrelay_t *)&((SASrelayPacket_t *)data)->sasrelay;
}

ZrtpPacketSASrelay::~ZrtpPacketSASrelay() {
    DEBUGOUT((fprintf(stdout, "Deleting SASrelay packet: alloc: %x\n", allocated)));
}
