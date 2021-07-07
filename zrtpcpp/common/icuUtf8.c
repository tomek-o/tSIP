// I copied/extracted the following code from the ICU C/C++ source code
// to support compilation/linking with older Android NDK and other systems
// that do not (yet) support the full Unicode functions.
//
// The license for ICU code is available at:
// https://www.unicode.org/copyright.html#License

//
// @author Werner Dittmann <Werner.Dittmann@t-online.de>
// Copyright (c) 2015 of the modifications by Werner Dittmann
//
#include <stdio.h>
#include <common/icuUtf.h>

/* for utf8_nextCharSafeBodyTerminated() */
static const UChar32 utf8_minLegal[4]={ 0, 0x80, 0x800, 0x10000 };

/**
 * NUL-terminate a string no matter what its type.
 * Set warning and error codes accordingly.
 */
#define __TERMINATE_STRING(dest, destCapacity, length, pErrorCode)      \
    if(pErrorCode!=NULL && U_SUCCESS(*pErrorCode)) {                    \
        /* not a public function, so no complete argument checking */   \
                                                                        \
        if(length<0) {                                                  \
            /* assume that the caller handles this */                   \
        } else if(length<destCapacity) {                                \
            /* NUL-terminate the string, the NUL fits */                \
            dest[length]=0;                                             \
            /* unset the not-terminated warning but leave all others */ \
            if(*pErrorCode==U_STRING_NOT_TERMINATED_WARNING) {          \
                *pErrorCode=U_ZERO_ERROR;                               \
            }                                                           \
        } else if(length==destCapacity) {                               \
            /* unable to NUL-terminate, but the string itself fit - set a warning code */ \
            *pErrorCode=U_STRING_NOT_TERMINATED_WARNING;                \
        } else /* length>destCapacity */ {                              \
            /* even the string itself did not fit - set an error code */ \
            *pErrorCode=U_BUFFER_OVERFLOW_ERROR;                        \
        }                                                               \
    }

int32_t u_terminateUChars(UChar *dest, int32_t destCapacity, int32_t length, UErrorCode *pErrorCode) {
    __TERMINATE_STRING(dest, destCapacity, length, pErrorCode);
    return length;
}

/*
 * Version of utf8_nextCharSafeBody() with the following differences:
 * - checks for NUL termination instead of length
 * - works with pointers instead of indexes
 * - always strict (strict==-1)
 *
 * *ps points to after the lead byte and will be moved to after the last trail byte.
 * c is the lead byte.
 * @return the code point, or U_SENTINEL
 */
static UChar32
utf8_nextCharSafeBodyTerminated(const uint8_t **ps, UChar32 c) {
    const uint8_t *s=*ps;
    uint8_t trail, illegal = 0;
    int32_t count = U8_COUNT_TRAIL_BYTES(c);
//    U_ASSERT(count<6);
    U8_MASK_LEAD_BYTE((c), count);
    /* count==0 for illegally leading trail bytes and the illegal bytes 0xfe and 0xff */
    switch(count) {
        /* each branch falls through to the next one */
        case 5:
        case 4:
            /* count>=4 is always illegal: no more than 3 trail bytes in Unicode's UTF-8 */
            illegal=1;
            break;
        case 3:
            trail=(uint8_t)(*s++ - 0x80);
            c=(c<<6)|trail;
            if(trail>0x3f || c>=0x110) {
                /* not a trail byte, or code point>0x10ffff (outside Unicode) */
                illegal=1;
                break;
            }
        case 2: /*fall through*/
            trail=(uint8_t)(*s++ - 0x80);
            if(trail>0x3f) {
                /* not a trail byte */
                illegal=1;
                break;
            }
            c=(c<<6)|trail;
        case 1: /*fall through*/
            trail=(uint8_t)(*s++ - 0x80);
            if(trail>0x3f) {
                /* not a trail byte */
                illegal=1;
            }
            c=(c<<6)|trail;
            break;
        case 0:
        default:
            return U_SENTINEL;
            /* no default branch to optimize switch()  - all values are covered */
    }

    /* correct sequence - all trail bytes have (b7..b6)==(10)? */
    /* illegal is also set if count>=4 */
    if(illegal || c<utf8_minLegal[count] || U_IS_SURROGATE(c)) {
        /* error handling */
        /* don't go beyond this sequence */
        s=*ps;
        while(count>0 && U8_IS_TRAIL(*s)) {
            ++s;
            --count;
        }
        c=U_SENTINEL;
    }
    *ps=s;
    return c;
}

/*
 * Version of utf8_nextCharSafeBody() with the following differences:
 * - works with pointers instead of indexes
 * - always strict (strict==-1)
 *
 * *ps points to after the lead byte and will be moved to after the last trail byte.
 * c is the lead byte.
 * @return the code point, or U_SENTINEL
 */
static UChar32
utf8_nextCharSafeBodyPointer(const uint8_t **ps, const uint8_t *limit, UChar32 c) {
    const uint8_t *s=*ps;
    uint8_t trail, illegal=0;
    int32_t count = U8_COUNT_TRAIL_BYTES(c);
    if((limit-s)>=count) {
        U8_MASK_LEAD_BYTE((c), count);
        /* count==0 for illegally leading trail bytes and the illegal bytes 0xfe and 0xff */
        switch(count) {
            /* each branch falls through to the next one */
            case 5:
            case 4:
                /* count>=4 is always illegal: no more than 3 trail bytes in Unicode's UTF-8 */
                illegal=1;
                break;
            case 3:
                trail=*s++;
                c=(c<<6)|(trail&0x3f);
                if(c<0x110) {
                    illegal|=(trail&0xc0)^0x80;
                } else {
                    /* code point>0x10ffff, outside Unicode */
                    illegal=1;
                    break;
                }
            case 2: /*fall through*/
                trail=*s++;
                c=(c<<6)|(trail&0x3f);
                illegal|=(trail&0xc0)^0x80;
            case 1: /*fall through*/
                trail=*s++;
                c=(c<<6)|(trail&0x3f);
                illegal|=(trail&0xc0)^0x80;
                break;
            case 0:
            default:
                return U_SENTINEL;
                /* no default branch to optimize switch()  - all values are covered */
        }
    } else {
        illegal=1; /* too few bytes left */
    }

    /* correct sequence - all trail bytes have (b7..b6)==(10)? */
    /* illegal is also set if count>=4 */
//    U_ASSERT(illegal || count<UPRV_LENGTHOF(utf8_minLegal));
    if(illegal || c<utf8_minLegal[count] || U_IS_SURROGATE(c)) {
        /* error handling */
        /* don't go beyond this sequence */
        s=*ps;
        while(count>0 && s<limit && U8_IS_TRAIL(*s)) {
            ++s;
            --count;
        }
        c=U_SENTINEL;
    }
    *ps=s;
    return c;
}


UChar *u_strFromUTF8WithSub(UChar *dest,
                            int32_t destCapacity,
                            int32_t *pDestLength,
                            const char *src,
                            int32_t srcLength,
                            UChar32 subchar, int32_t *pNumSubstitutions,
                            UErrorCode *pErrorCode) {
    UChar *pDest = dest;
    UChar *pDestLimit = dest + destCapacity;
    UChar32 ch;
    int32_t reqLength = 0;
    const uint8_t *pSrc = (const uint8_t *) src;
    uint8_t t1, t2; /* trail bytes */
    int32_t numSubstitutions;

/* args check */
    if (pErrorCode == NULL || U_FAILURE(*pErrorCode)) {
        return NULL;
    }

    if ((src == NULL && srcLength != 0) || srcLength < -1 ||
        (destCapacity < 0) || (dest == NULL && destCapacity > 0) ||
        subchar > 0x10ffff || U_IS_SURROGATE(subchar)
            ) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return NULL;
    }

    if (pNumSubstitutions != NULL) {
        *pNumSubstitutions = 0;
    }
    numSubstitutions = 0;

/*
 * Inline processing of UTF-8 byte sequences:
 *
 * Byte sequences for the most common characters are handled inline in
 * the conversion loops. In order to reduce the path lengths for those
 * characters, the tests are arranged in a kind of binary search.
 * ASCII (<=0x7f) is checked first, followed by the dividing point
 * between 2- and 3-byte sequences (0xe0).
 * The 3-byte branch is tested first to speed up CJK text.
 * The compiler should combine the subtractions for the two tests for 0xe0.
 * Each branch then tests for the other end of its range.
 */

    if (srcLength < 0) {
/*
 * Transform a NUL-terminated string.
 * The code explicitly checks for NULs only in the lead byte position.
 * A NUL byte in the trail byte position fails the trail byte range check anyway.
 */
        while (((ch = *pSrc) != 0) && (pDest < pDestLimit)) {
            if (ch <= 0x7f) {
                *pDest++ = (UChar) ch;
                ++pSrc;
            } else {
                if (ch > 0xe0) {
                    if ( /* handle U+1000..U+CFFF inline */
                            ch <= 0xec &&
                            (t1 = (uint8_t)(pSrc[1] - 0x80)) <= 0x3f &&
                            (t2 = (uint8_t)(pSrc[2] - 0x80)) <= 0x3f
                            ) {
/* no need for (ch & 0xf) because the upper bits are truncated after <<12 in the cast to (UChar) */
                        *pDest++ = (UChar)((ch << 12) | (t1 << 6) | t2);
                        pSrc += 3;
                        continue;
                    }
                } else if (ch < 0xe0) {
                    if ( /* handle U+0080..U+07FF inline */
                            ch >= 0xc2 &&
                            (t1 = (uint8_t)(pSrc[1] - 0x80)) <= 0x3f
                            ) {
                        *pDest++ = (UChar)(((ch & 0x1f) << 6) | t1);
                        pSrc += 2;
                        continue;
                    }
                }

/* function call for "complicated" and error cases */
                ++pSrc; /* continue after the lead byte */
                ch = utf8_nextCharSafeBodyTerminated(&pSrc, ch);
                if (ch < 0 && (++numSubstitutions, ch = subchar) < 0) {
                    *pErrorCode = U_INVALID_CHAR_FOUND;
                    return NULL;
                } else if (ch <= 0xFFFF) {
                    *(pDest++) = (UChar) ch;
                } else {
                    *(pDest++) = U16_LEAD(ch);
                    if (pDest < pDestLimit) {
                        *(pDest++) = U16_TRAIL(ch);
                    } else {
                        reqLength++;
                        break;
                    }
                }
            }
        }

/* Pre-flight the rest of the string. */
        while ((ch = *pSrc) != 0) {
            if (ch <= 0x7f) {
                ++reqLength;
                ++pSrc;
            } else {
                if (ch > 0xe0) {
                    if ( /* handle U+1000..U+CFFF inline */
                            ch <= 0xec &&
                            (uint8_t)(pSrc[1] - 0x80) <= 0x3f &&
                            (uint8_t)(pSrc[2] - 0x80) <= 0x3f
                            ) {
                        ++reqLength;
                        pSrc += 3;
                        continue;
                    }
                } else if (ch < 0xe0) {
                    if ( /* handle U+0080..U+07FF inline */
                            ch >= 0xc2 &&
                            (uint8_t)(pSrc[1] - 0x80) <= 0x3f
                            ) {
                        ++reqLength;
                        pSrc += 2;
                        continue;
                    }
                }

/* function call for "complicated" and error cases */
                ++pSrc; /* continue after the lead byte */
                ch = utf8_nextCharSafeBodyTerminated(&pSrc, ch);
                if (ch < 0 && (++numSubstitutions, ch = subchar) < 0) {
                    *pErrorCode = U_INVALID_CHAR_FOUND;
                    return NULL;
                }
                reqLength += U16_LENGTH(ch);
            }
        }
    } else /* srcLength >= 0 */ {
        const uint8_t *pSrcLimit = pSrc + srcLength;
        int32_t count;

/* Faster loop without ongoing checking for pSrcLimit and pDestLimit. */
        for (; ;) {
/*
 * Each iteration of the inner loop progresses by at most 3 UTF-8
 * bytes and one UChar, for most characters.
 * For supplementary code points (4 & 2), which are rare,
 * there is an additional adjustment.
 */
            count = (int32_t)(pDestLimit - pDest);
            srcLength = (int32_t)((pSrcLimit - pSrc) / 3);
            if (count > srcLength) {
                count = srcLength; /* min(remaining dest, remaining src/3) */
            }
            if (count < 3) {
/*
 * Too much overhead if we get near the end of the string,
 * continue with the next loop.
 */
                break;
            }

            do {
                ch = *pSrc;
                if (ch <= 0x7f) {
                    *pDest++ = (UChar) ch;
                    ++pSrc;
                } else {
                    if (ch > 0xe0) {
                        if ( /* handle U+1000..U+CFFF inline */
                                ch <= 0xec &&
                                (t1 = (uint8_t)(pSrc[1] - 0x80)) <= 0x3f &&
                                (t2 = (uint8_t)(pSrc[2] - 0x80)) <= 0x3f
                                ) {
/* no need for (ch & 0xf) because the upper bits are truncated after <<12 in the cast to (UChar) */
                            *pDest++ = (UChar)((ch << 12) | (t1 << 6) | t2);
                            pSrc += 3;
                            continue;
                        }
                    } else if (ch < 0xe0) {
                        if ( /* handle U+0080..U+07FF inline */
                                ch >= 0xc2 &&
                                (t1 = (uint8_t)(pSrc[1] - 0x80)) <= 0x3f
                                ) {
                            *pDest++ = (UChar)(((ch & 0x1f) << 6) | t1);
                            pSrc += 2;
                            continue;
                        }
                    }

                    if (ch >= 0xf0 || subchar > 0xffff) {
/*
 * We may read up to six bytes and write up to two UChars,
 * which we didn't account for with computing count,
 * so we adjust it here.
 */
                        if (--count == 0) {
                            break;
                        }
                    }

/* function call for "complicated" and error cases */
                    ++pSrc; /* continue after the lead byte */
                    ch = utf8_nextCharSafeBodyPointer(&pSrc, pSrcLimit, ch);
                    if (ch < 0 && (++numSubstitutions, ch = subchar) < 0) {
                        *pErrorCode = U_INVALID_CHAR_FOUND;
                        return NULL;
                    } else if (ch <= 0xFFFF) {
                        *(pDest++) = (UChar) ch;
                    } else {
                        *(pDest++) = U16_LEAD(ch);
                        *(pDest++) = U16_TRAIL(ch);
                    }
                }
            } while (--count > 0);
        }

        while ((pSrc < pSrcLimit) && (pDest < pDestLimit)) {
            ch = *pSrc;
            if (ch <= 0x7f) {
                *pDest++ = (UChar) ch;
                ++pSrc;
            } else {
                if (ch > 0xe0) {
                    if ( /* handle U+1000..U+CFFF inline */
                            ch <= 0xec &&
                            ((pSrcLimit - pSrc) >= 3) &&
                            (t1 = (uint8_t)(pSrc[1] - 0x80)) <= 0x3f &&
                            (t2 = (uint8_t)(pSrc[2] - 0x80)) <= 0x3f
                            ) {
/* no need for (ch & 0xf) because the upper bits are truncated after <<12 in the cast to (UChar) */
                        *pDest++ = (UChar)((ch << 12) | (t1 << 6) | t2);
                        pSrc += 3;
                        continue;
                    }
                } else if (ch < 0xe0) {
                    if ( /* handle U+0080..U+07FF inline */
                            ch >= 0xc2 &&
                            ((pSrcLimit - pSrc) >= 2) &&
                            (t1 = (uint8_t)(pSrc[1] - 0x80)) <= 0x3f
                            ) {
                        *pDest++ = (UChar)(((ch & 0x1f) << 6) | t1);
                        pSrc += 2;
                        continue;
                    }
                }

/* function call for "complicated" and error cases */
                ++pSrc; /* continue after the lead byte */
                ch = utf8_nextCharSafeBodyPointer(&pSrc, pSrcLimit, ch);
                if (ch < 0 && (++numSubstitutions, ch = subchar) < 0) {
                    *pErrorCode = U_INVALID_CHAR_FOUND;
                    return NULL;
                } else if (ch <= 0xFFFF) {
                    *(pDest++) = (UChar) ch;
                } else {
                    *(pDest++) = U16_LEAD(ch);
                    if (pDest < pDestLimit) {
                        *(pDest++) = U16_TRAIL(ch);
                    } else {
                        reqLength++;
                        break;
                    }
                }
            }
        }
/* do not fill the dest buffer just count the UChars needed */
        while (pSrc < pSrcLimit) {
            ch = *pSrc;
            if (ch <= 0x7f) {
                reqLength++;
                ++pSrc;
            } else {
                if (ch > 0xe0) {
                    if ( /* handle U+1000..U+CFFF inline */
                            ch <= 0xec &&
                            ((pSrcLimit - pSrc) >= 3) &&
                            (uint8_t)(pSrc[1] - 0x80) <= 0x3f &&
                            (uint8_t)(pSrc[2] - 0x80) <= 0x3f
                            ) {
                        reqLength++;
                        pSrc += 3;
                        continue;
                    }
                } else if (ch < 0xe0) {
                    if ( /* handle U+0080..U+07FF inline */
                            ch >= 0xc2 &&
                            ((pSrcLimit - pSrc) >= 2) &&
                            (uint8_t)(pSrc[1] - 0x80) <= 0x3f
                            ) {
                        reqLength++;
                        pSrc += 2;
                        continue;
                    }
                }

/* function call for "complicated" and error cases */
                ++pSrc; /* continue after the lead byte */
                ch = utf8_nextCharSafeBodyPointer(&pSrc, pSrcLimit, ch);
                if (ch < 0 && (++numSubstitutions, ch = subchar) < 0) {
                    *pErrorCode = U_INVALID_CHAR_FOUND;
                    return NULL;
                }
                reqLength += U16_LENGTH(ch);
            }
        }
    }

    reqLength += (int32_t)(pDest - dest);

    if (pNumSubstitutions != NULL) {
        *pNumSubstitutions = numSubstitutions;
    }

    if (pDestLength) {
        *pDestLength = reqLength;
    }

/* Terminate the buffer */
    u_terminateUChars(dest, destCapacity, reqLength, pErrorCode);

    return dest;
}


UChar* u_strFromUTF8(UChar *dest,
                     int32_t destCapacity,
                     int32_t *pDestLength,
                     const char* src,
                     int32_t srcLength,
                     UErrorCode *pErrorCode){
    return u_strFromUTF8WithSub(
            dest, destCapacity, pDestLength,
            src, srcLength,
            U_SENTINEL, NULL,
            pErrorCode);
}

