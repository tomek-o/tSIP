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

#include <libzrtpcpp/ZrtpPacketDHPart.h>

ZrtpPacketDHPart::ZrtpPacketDHPart() {
    DEBUGOUT((fprintf(stdout, "Creating DHPart packet without data and pkt type\n")));
    initialize();
}

ZrtpPacketDHPart::ZrtpPacketDHPart(const char* pkt) {
    DEBUGOUT((fprintf(stdout, "Creating DHPart packet without data\n")));
    initialize();
    setPubKeyType(pkt);
}

void ZrtpPacketDHPart::initialize() {

    void* allocated = &data;
    memset(allocated, 0, sizeof(data));

    zrtpHeader = &((DHPartPacket_t *)allocated)->hdr; // the standard header
    DHPartHeader = &((DHPartPacket_t *)allocated)->dhPart;
    pv = ((uint8_t*)allocated) + sizeof(DHPartPacket_t);    // point to the public key value

    setZrtpId();
}

// The fixed numbers below are taken from ZRTP specification, chap 5.1.5
void ZrtpPacketDHPart::setPubKeyType(const char* pkt) {
    // Well - the algorithm type is only 4 char thus cast to int32 and compare
    if (*(int32_t*)pkt == *(int32_t*)dh2k) {
        dhLength = 256;
    }
    else if (*(int32_t*)pkt == *(int32_t*)dh3k) {
        dhLength = 384;
    }
    else if (*(int32_t*)pkt == *(int32_t*)ec25) {
        dhLength = 64;
    }
    else if (*(int32_t*)pkt == *(int32_t*)ec38) {
        dhLength = 96;
    }
    else if (*(int32_t*)pkt == *(int32_t*)e255) {
        dhLength = 32;
    }
    else if (*(int32_t*)pkt == *(int32_t*)e414) {
        dhLength = 104;
    }
    else
        return;

    uint16_t length = static_cast<uint16_t>(sizeof(DHPartPacket_t) + dhLength + (2 * ZRTP_WORD_SIZE)); // HMAC field is 2*ZRTP_WORD_SIZE
    setLength(static_cast<uint16_t>(length / ZRTP_WORD_SIZE));
}

ZrtpPacketDHPart::ZrtpPacketDHPart(uint8_t *data) {
    DEBUGOUT((fprintf(stdout, "Creating DHPart packet from data\n")));

    zrtpHeader = &((DHPartPacket_t *)data)->hdr;  // the standard header
    DHPartHeader = &((DHPartPacket_t *)data)->dhPart;

    int16_t len = getLength();
    DEBUGOUT((fprintf(stdout, "DHPart length: %d\n", len)));
    if (len == 85) {         // Dh2k
        dhLength = 256;
    }
    else if (len == 117) {   // Dh3k
        dhLength = 384;
    }
    else if (len == 37) {    // EC256
        dhLength = 64;
    }
    else if (len == 45) {    // EC384
        dhLength = 96;
    }
    else if (len == 29) {    // E255
        dhLength = 32;
    }
    else if (len == 47) {    // E414
        dhLength = 104;
    }
    else {
        pv = NULL;
        return;
    }
    pv = data + sizeof(DHPartPacket_t);    // point to the public key value
}

ZrtpPacketDHPart::~ZrtpPacketDHPart() {
    DEBUGOUT((fprintf(stdout, "Deleting DHPart packet: alloc: %x\n", allocated)));
}
