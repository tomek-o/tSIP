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

#include <libzrtpcpp/ZIDRecordEmpty.h>

void ZIDRecordEmpty::setNewRs1(const unsigned char* data, int32_t expire) {
    (void) data;
    (void) expire;
}


bool ZIDRecordEmpty::isRs1NotExpired() {
    return false;
}

bool ZIDRecordEmpty::isRs2NotExpired() {
    return false;
}

void ZIDRecordEmpty::setMiTMData(const unsigned char* data) {
    (void) data;
}
