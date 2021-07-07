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
// #define UNIT_TEST

#include <string>
#include <stdlib.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <io.h>
#else
#include <unistd.h>
#endif

#include <crypto/zrtpDH.h>

#include <libzrtpcpp/ZIDCacheEmpty.h>


static ZIDCacheEmpty* instance;

/**
 * A poor man's factory.
 *
 * The build process must not allow two cache file implementation classes linked
 * into the same library.
 */

ZIDCache* getZidCacheInstance() {

    if (instance == NULL) {
        instance = new ZIDCacheEmpty();
    }
    return instance;
}

int ZIDCacheEmpty::open(char* name) {
    (void) name;
    return 1;
}

void ZIDCacheEmpty::close() {}

ZIDRecord *ZIDCacheEmpty::getRecord(unsigned char *zid) {
    return new ZIDRecordEmpty();
}

unsigned int ZIDCacheEmpty::saveRecord(ZIDRecord *zidRec) {
    (void) zidRec;
    return 1;
}

int32_t ZIDCacheEmpty::getPeerName(const uint8_t *peerZid, std::string *name) {
    (void) peerZid;
    (void) name;
    return 0;
}

void ZIDCacheEmpty::putPeerName(const uint8_t *peerZid, const std::string name) {
    (void) peerZid;
    (void) name;
}
