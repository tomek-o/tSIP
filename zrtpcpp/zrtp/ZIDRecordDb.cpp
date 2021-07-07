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

#include <time.h>

#include <libzrtpcpp/ZIDRecordDb.h>

void ZIDRecordDb::setNewRs1(const unsigned char* data, int32_t expire) {

    // shift RS1 data into RS2 position
    memcpy(record.rs2, record.rs1, RS_LENGTH);
    record.rs2Ttl = record.rs1Ttl;

    // set new RS1 data
    memcpy(record.rs1, data, RS_LENGTH);

    time_t validThru;
    if (expire == -1) {
        validThru = -1;
    }
    else if (expire <= 0) {
        validThru = 0;
    }
    else {
        validThru = time(NULL) + expire;
    }
    record.rs1Ttl = validThru;
    resetRs2Valid();
    setRs1Valid();
}


bool ZIDRecordDb::isRs1NotExpired() {
    time_t current = time(NULL);
    time_t validThru;

    validThru = record.rs1Ttl;

    if (validThru == -1)
        return true;
    if (validThru == 0)
        return false;
    return (current <= validThru) ? true : false;
}

bool ZIDRecordDb::isRs2NotExpired() {
    time_t current = time(NULL);
    time_t validThru;

    validThru = record.rs2Ttl;

    if (validThru == -1)
        return true;
    if (validThru == 0)
        return false;
    return (current <= validThru) ? true : false;
}

void ZIDRecordDb::setMiTMData(const unsigned char* data) {
    memcpy(record.mitmKey, data, RS_LENGTH);
    setMITMKeyAvailable();
}
