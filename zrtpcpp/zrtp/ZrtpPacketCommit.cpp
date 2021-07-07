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

#include <libzrtpcpp/ZrtpPacketCommit.h>

ZrtpPacketCommit::ZrtpPacketCommit() {
    DEBUGOUT((fprintf(stdout, "Creating commit packet without data\n")));

    zrtpHeader = &data.hdr;	// the standard header
    commitHeader = &data.commit;

    setZrtpId();
    setLength((sizeof (CommitPacket_t) / ZRTP_WORD_SIZE) - 1);
    setMessageType((uint8_t*)CommitMsg);
}

void ZrtpPacketCommit::setNonce(uint8_t* text) {
    memcpy(commitHeader->hvi, text, sizeof(data.commit.hvi)-4*ZRTP_WORD_SIZE);
    uint16_t len = getLength();
    len -= 4;
    setLength(len);
}

ZrtpPacketCommit::ZrtpPacketCommit(uint8_t *data) {
    DEBUGOUT((fprintf(stdout, "Creating commit packet from data\n")));
    zrtpHeader = (zrtpPacketHeader_t *)&((CommitPacket_t *)data)->hdr;	// the standard header
    commitHeader = (Commit_t *)&((CommitPacket_t *)data)->commit;
}

ZrtpPacketCommit::~ZrtpPacketCommit() {
    DEBUGOUT((fprintf(stdout, "Deleting commit packet: alloc: %x\n", allocated)));
}
