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

#ifndef _ZRTPTEXTDATA_H_
#define _ZRTPTEXTDATA_H_

/**
 * @file ZrtpTextData.h
 * @brief The ZRTP ASCII texts - extern references
 *  
 * @ingroup GNU_ZRTP
 * @{
 */

#include <libzrtpcpp/ZrtpConfigure.h>

/**
 * The extern references to the global data.
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */

extern char zrtpBuildInfo[];

extern char clientId[];
extern char zrtpVersion_11[];
extern char zrtpVersion_12[];

/**
 *
 */
extern char HelloMsg[];
extern char HelloAckMsg[];
extern char CommitMsg[];
extern char DHPart1Msg[];
extern char DHPart2Msg[];
extern char Confirm1Msg[];
extern char Confirm2Msg[];
extern char Conf2AckMsg[];
extern char ErrorMsg[];
extern char ErrorAckMsg[];
extern char GoClearMsg[];
extern char ClearAckMsg[];
extern char PingMsg[];
extern char PingAckMsg[];
extern char SasRelayMsg[];
extern char RelayAckMsg[];

/**
 *
 */
extern char responder[];
extern char initiator[];
extern char iniMasterKey[];
extern char iniMasterSalt[];
extern char respMasterKey[];
extern char respMasterSalt[];

extern char iniHmacKey[];
extern char respHmacKey[];
extern char retainedSec[];

extern char iniZrtpKey[];
extern char respZrtpKey[];

extern char sasString[];

extern char KDFString[];
extern char zrtpSessionKey[];
extern char zrtpExportedKey[];
extern char zrtpMsk[];
extern char zrtpTrustedMitm[];


extern char s256[];
extern char s384[];
extern char skn2[];
extern char skn3[];
extern const char* mandatoryHash;

extern char aes3[];
extern char aes2[];
extern char aes1[];
extern char two3[];
extern char two2[];
extern char two1[];

extern const char* mandatoryCipher;

extern char dh2k[];
extern char dh3k[];
extern char ec25[];
extern char ec38[];
extern char e255[];
extern char e414[];

extern char mult[];

extern const char* mandatoryPubKey;

extern char b32[];
extern char b256[];
extern char b32e[];
extern char b10d[];
extern const char* mandatorySasType;

extern char hs32[];
extern char hs80[];
extern char sk32[];
extern char sk64[];
extern const char* mandatoryAuthLen_1;
extern const char* mandatoryAuthLen_2;

extern const char* sas256WordsOdd[];
extern const char* sas256WordsEven[];

/**
 * @}
 */
#endif     // _ZRTPTEXTDATA_H_

