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


#include <stdint.h>
#include <common/osSpecifics.h>


#if defined(_WIN32) || defined(_WIN64)

#else

#endif

#if defined(_WIN32) || defined(_WIN64)
# include <WinSock2.h>
# include <time.h>

uint64_t  zrtpGetTickCount()
{
   // return GetTickCount64();  //works only on 64bit OS
   unsigned long long ret;
   FILETIME ft;
   GetSystemTimeAsFileTime(&ft);
   ret = ft.dwHighDateTime;
   ret <<= 32;
   ret |= ft.dwLowDateTime;

   return ret / 10;             //return msec
}
#else
# include <netinet/in.h>
# include <sys/time.h>

uint64_t zrtpGetTickCount()
{
   struct timeval tv;
   gettimeofday(&tv, 0);

   return ((uint64_t)tv.tv_sec) * (uint64_t)1000 + ((uint64_t)tv.tv_usec) / (uint64_t)1000;
}

#endif

uint32_t zrtpNtohl (uint32_t net)
{
    return ntohl(net);
}

uint16_t zrtpNtohs (uint16_t net)
{
    return ntohs(net);
}

uint32_t zrtpHtonl (uint32_t host)
{
    return htonl(host);
}
uint16_t zrtpHtons (uint16_t host)
{
    return htons(host);
}

