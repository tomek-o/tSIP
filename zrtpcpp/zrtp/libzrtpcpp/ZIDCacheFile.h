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

#include <stdio.h>

#include <libzrtpcpp/ZIDCache.h>
#include <libzrtpcpp/ZIDRecordFile.h>

#ifndef _ZIDCACHEFILE_H_
#define _ZIDCACHEFILE_H_


/**
 * @file ZIDCacheFile.h
 * @brief ZID cache management
 *
 * A ZID file stores (caches) some data that helps ZRTP to achives its
 * key continuity feature. See @c ZIDRecord for further info which data
 * the ZID file contains.
 *
 * @ingroup GNU_ZRTP
 * @{
 */

/**
 * This class implements a ZID (ZRTP Identifiers) file.
 *
 * The interface defintion @c ZIDCache.h contains the method documentation.
 * The ZID cache file holds information about peers.
 *
 * @author: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

class __EXPORT ZIDCacheFile: public ZIDCache {

private:

    FILE* zidFile;
    unsigned char associatedZid[IDENTIFIER_LEN];

    void createZIDFile(char* name);
    void checkDoMigration(char* name);

public:

    ZIDCacheFile(): zidFile(NULL) {};

    ~ZIDCacheFile();

    int open(char *name);

    bool isOpen() { return (zidFile != NULL); };

    void close();

    ZIDRecord *getRecord(unsigned char *zid);

    unsigned int saveRecord(ZIDRecord *zidRecord);

    const unsigned char* getZid() { return associatedZid; };

    int32_t getPeerName(const uint8_t *peerZid, std::string *name);

    void putPeerName(const uint8_t *peerZid, const std::string name);

    // Not implemented for file based cache
    void cleanup() {};
    void *prepareReadAll() { return NULL; };
    void *readNextRecord(void *stmt, std::string *output) { return NULL; };
    void closeOpenStatment(void *stmt) {}


};

/**
 * @}
 */
#endif
