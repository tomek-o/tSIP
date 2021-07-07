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
 * @author: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#include <libzrtpcpp/ZrtpPacketClearAck.h>

ZrtpPacketClearAck::ZrtpPacketClearAck() {
    DEBUGOUT((fprintf(stdout, "Creating ClearAck packet without data\n")));

    zrtpHeader = &data.hdr;	// the standard header

    setZrtpId();
    setLength((sizeof(ClearAckPacket_t) / ZRTP_WORD_SIZE) - 1);
    setMessageType((uint8_t*)ClearAckMsg);
}

ZrtpPacketClearAck::ZrtpPacketClearAck(uint8_t *data) {
    DEBUGOUT((fprintf(stdout, "Creating Conf2Ack packet from data\n")));

    zrtpHeader = (zrtpPacketHeader_t *)&((ClearAckPacket_t*)data)->hdr;	// the standard header
}

ZrtpPacketClearAck::~ZrtpPacketClearAck() {
    DEBUGOUT((fprintf(stdout, "Deleting ClearAck packet: alloc: %x\n", allocated)));
}
