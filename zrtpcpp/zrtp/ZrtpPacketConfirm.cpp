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

#include <libzrtpcpp/ZrtpPacketConfirm.h>

ZrtpPacketConfirm::ZrtpPacketConfirm() {
    DEBUGOUT((fprintf(stdout, "Creating Confirm packet without data, no sl data\n")));
    initialize();
    setSignatureLength(0);
}

ZrtpPacketConfirm::ZrtpPacketConfirm(uint32_t sl) {
    DEBUGOUT((fprintf(stdout, "Creating Confirm packet without data\n")));
    initialize();
    setSignatureLength(sl);
}

void ZrtpPacketConfirm::initialize() {
    void* allocated = &data;
    memset(allocated, 0, sizeof(data));

    zrtpHeader = (zrtpPacketHeader_t *)&((ConfirmPacket_t *)allocated)->hdr;	// the standard header
    confirmHeader = (Confirm_t *)&((ConfirmPacket_t *)allocated)->confirm;

    setZrtpId();
}

bool ZrtpPacketConfirm::setSignatureLength(uint32_t sl) {
    if (sl > 512)
        return false;

    int32_t length = sizeof(ConfirmPacket_t) + (sl * ZRTP_WORD_SIZE);
    confirmHeader->sigLength = sl;                                     // sigLength is a uint byte
    if (sl & 0x100) {                                                  // check the 9th bit
        confirmHeader->filler[1] = 1;                                  // and set it if necessary
    }
    setLength(length / 4);
    return true;
}

bool ZrtpPacketConfirm::setSignatureData(uint8_t* data, int32_t length) {
    int32_t l = getSignatureLength() * 4;
    if (length > l || (length % 4) != 0)
        return false;

    uint8_t* p = ((uint8_t*)&confirmHeader->expTime) + 4;              // point to signature block
    memcpy(p, data, length);
    return true;
}

bool ZrtpPacketConfirm::isSignatureLengthOk() {
    int32_t actualLen = getLength();
    int32_t expectedLen = 19;                  // Confirm packet fixed part is 19 ZRTP words
    int32_t sigLen = getSignatureLength();

    expectedLen += sigLen;
    return (expectedLen == actualLen);
}

int32_t ZrtpPacketConfirm::getSignatureLength() {
    int32_t sl = confirmHeader->sigLength & 0xff;
    if (confirmHeader->filler[1] == 1) {                              // do we have a 9th bit
        sl |= 0x100;
    }
    return sl;
}

ZrtpPacketConfirm::ZrtpPacketConfirm(uint8_t* data) {
    DEBUGOUT((fprintf(stdout, "Creating Confirm packet from data\n")));

    allocated = NULL;
    zrtpHeader = (zrtpPacketHeader_t *)&((ConfirmPacket_t *)data)->hdr;	// the standard header
    confirmHeader = (Confirm_t *)&((ConfirmPacket_t *)data)->confirm;
}

ZrtpPacketConfirm::~ZrtpPacketConfirm() {
    DEBUGOUT((fprintf(stdout, "Deleting Confirm packet: alloc: %x\n", allocated)));
}
