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

#include <libzrtpcpp/ZrtpPacketRelayAck.h>

ZrtpPacketRelayAck::ZrtpPacketRelayAck() {
    DEBUGOUT((fprintf(stdout, "Creating RelayAck packet without data\n")));

    zrtpHeader = &data.hdr;	// the standard header

    setZrtpId();
    setLength((sizeof (RelayAckPacket_t) / ZRTP_WORD_SIZE) - 1);
    setMessageType((uint8_t*)RelayAckMsg);
}

ZrtpPacketRelayAck::ZrtpPacketRelayAck(uint8_t *data) {
    DEBUGOUT((fprintf(stdout, "Creating RelayAck packet from data\n")));

    zrtpHeader = (zrtpPacketHeader_t *)&((RelayAckPacket_t*)data)->hdr;	// the standard header
}

ZrtpPacketRelayAck::~ZrtpPacketRelayAck() {
    DEBUGOUT((fprintf(stdout, "Deleting RelayAck packet: alloc: %x\n", allocated)));
}
