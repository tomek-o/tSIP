// I copied/extracted the following code from the ICU C/C++ source code
// to support compilation/linking with older Android NDK and other systems
// that do not (yet) support the full Unicode functions.
//
// The license for ICU code is available at:
// https://www.unicode.org/copyright.html#License

//
// Werner Dittmann <Werner.Dittmann@t-online.de>
// Copyright (c) 2015 of the modifications by Werner Dittmann
//

#ifndef UTF_ICUUTF_H
#define UTF_ICUUTF_H

/**
 * @file
 * @brief UTF support functions
 * @defgroup GNU_ZRTP The GNU ZRTP C++ implementation
 * @{
 */

#include <stdint.h>

/**
 * \var UChar
 * Define UChar to be UCHAR_TYPE, if that is #defined (for example, to char16_t),
 * or wchar_t if that is 16 bits wide; always assumed to be unsigned.
 * If neither is available, then define UChar to be uint16_t.
 *
 * This makes the definition of UChar platform-dependent
 * but allows direct string type compatibility with platforms with
 * 16-bit wchar_t types.
 *
 * @stable ICU 4.4
 */
#if defined(UCHAR_TYPE)
typedef UCHAR_TYPE UChar;
/* Not #elif U_HAVE_CHAR16_T -- because that is type-incompatible with pre-C++11 callers
    typedef char16_t UChar;  */
#elif U_SIZEOF_WCHAR_T==2
typedef wchar_t UChar;
#elif defined(__CHAR16_TYPE__)
typedef __CHAR16_TYPE__ UChar;
#else
typedef uint16_t UChar;
#endif

/**
 * Define UChar32 as a type for single Unicode code points.
 * UChar32 is a signed 32-bit integer (same as int32_t).
 *
 * The Unicode code point range is 0..0x10ffff.
 * All other values (negative or >=0x110000) are illegal as Unicode code points.
 * They may be used as sentinel values to indicate "done", "error"
 * or similar non-code point conditions.
 *
 * Before ICU 2.4 (Jitterbug 2146), UChar32 was defined
 * to be wchar_t if that is 32 bits wide (wchar_t may be signed or unsigned)
 * or else to be uint32_t.
 * That is, the definition of UChar32 was platform-dependent.
 *
 * @see U_SENTINEL
 * @stable ICU 2.4
 */
typedef int32_t UChar32;

/**
 * This value is intended for sentinel values for APIs that
 * (take or) return single code points (UChar32).
 * It is outside of the Unicode code point range 0..0x10ffff.
 *
 * For example, a "done" or "error" value in a new API
 * could be indicated with U_SENTINEL.
 *
 * ICU APIs designed before ICU 2.4 usually define service-specific "done"
 * values, mostly 0xffff.
 * Those may need to be distinguished from
 * actual U+ffff text contents by calling functions like
 * CharacterIterator::hasNext() or UnicodeString::length().
 *
 * @return -1
 * @see UChar32
 * @stable ICU 2.4
 */
#define U_SENTINEL (-1)

typedef enum UErrorCode {
    U_STRING_NOT_TERMINATED_WARNING = -124,/**< An output string could not be NUL-terminated because output length==destCapacity. */
    U_ZERO_ERROR              =  0,     /**< No error, no warning. */
    U_ILLEGAL_ARGUMENT_ERROR  =  1,     /**< Start of codes indicating failure */
    U_INVALID_CHAR_FOUND      = 10,     /**< Character conversion: Unmappable input sequence. In other APIs: Invalid character. */
    U_BUFFER_OVERFLOW_ERROR   = 15,     /**< A result would not fit in the supplied buffer */
} UErrorCode;

/**
 * Does the error code indicate success?
 * @stable ICU 2.0
 */
#   define U_SUCCESS(x) ((x)<=U_ZERO_ERROR)

/**
 * Does the error code indicate a failure?
 * @stable ICU 2.0
 */
#   define U_FAILURE(x) ((x)>U_ZERO_ERROR)

/**
 * Is this code point a surrogate (U+d800..U+dfff)?
 * @param c 32-bit code point
 * @return TRUE or FALSE
 * @stable ICU 2.4
 */
#define U_IS_SURROGATE(c) (((c)&0xfffff800)==0xd800)

/**
 * Get the lead surrogate (0xd800..0xdbff) for a
 * supplementary code point (0x10000..0x10ffff).
 * @param supplementary 32-bit code point (U+10000..U+10ffff)
 * @return lead surrogate (U+d800..U+dbff) for supplementary
 * @stable ICU 2.4
 */
#define U16_LEAD(supplementary) (UChar)(((supplementary)>>10)+0xd7c0)

/**
 * Get the trail surrogate (0xdc00..0xdfff) for a
 * supplementary code point (0x10000..0x10ffff).
 * @param supplementary 32-bit code point (U+10000..U+10ffff)
 * @return trail surrogate (U+dc00..U+dfff) for supplementary
 * @stable ICU 2.4
 */
#define U16_TRAIL(supplementary) (UChar)(((supplementary)&0x3ff)|0xdc00)

/**
 * How many 16-bit code units are used to encode this Unicode code point? (1 or 2)
 * The result is not defined if c is not a Unicode code point (U+0000..U+10ffff).
 * @param c 32-bit code point
 * @return 1 or 2
 * @stable ICU 2.4
 */
#define U16_LENGTH(c) ((uint32_t)(c)<=0xffff ? 1 : 2)

/**
 * Counts the trail bytes for a UTF-8 lead byte.
 * Returns 0 for 0..0xbf as well as for 0xfe and 0xff.
 *
 * This is internal since it is not meant to be called directly by external clients;
 * however it is called by public macros in this file and thus must remain stable.
 *
 * Note: Beginning with ICU 50, the implementation uses a multi-condition expression
 * which was shown in 2012 (on x86-64) to compile to fast, branch-free code.
 * leadByte is evaluated multiple times.
 *
 * The pre-ICU 50 implementation used the exported array utf8_countTrailBytes:
 * #define U8_COUNT_TRAIL_BYTES(leadByte) (utf8_countTrailBytes[leadByte])
 * leadByte was evaluated exactly once.
 *
 * @param leadByte The first byte of a UTF-8 sequence. Must be 0..0xff.
 * @internal
 */
#define U8_COUNT_TRAIL_BYTES(leadByte) \
    ((uint8_t)(leadByte)<0xf0 ? \
        ((uint8_t)(leadByte)>=0xc0)+((uint8_t)(leadByte)>=0xe0) : \
        (uint8_t)(leadByte)<0xfe ? 3+((uint8_t)(leadByte)>=0xf8)+((uint8_t)(leadByte)>=0xfc) : 0)

/**
 * Mask a UTF-8 lead byte, leave only the lower bits that form part of the code point value.
 *
 * This is internal since it is not meant to be called directly by external clients;
 * however it is called by public macros in this file and thus must remain stable.
 * @internal
 */
#define U8_MASK_LEAD_BYTE(leadByte, countTrailBytes) ((leadByte)&=(1<<(6-(countTrailBytes)))-1)

/**
 * Is this code unit (byte) a UTF-8 trail byte?
 * @param c 8-bit code unit (byte)
 * @return TRUE or FALSE
 * @stable ICU 2.4
 */
#define U8_IS_TRAIL(c) (((c)&0xc0)==0x80)


#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * Convert a UTF-8 string to UTF-16.
 * If the input string is not well-formed, then the U_INVALID_CHAR_FOUND error code is set.
 *
 * @param dest          A buffer for the result string. The result will be zero-terminated if
 *                      the buffer is large enough.
 * @param destCapacity  The size of the buffer (number of UChars). If it is 0, then
 *                      dest may be NULL and the function will only return the length of the
 *                      result without writing any of the result string (pre-flighting).
 * @param pDestLength   A pointer to receive the number of units written to the destination. If
 *                      pDestLength!=NULL then *pDestLength is always set to the
 *                      number of output units corresponding to the transformation of
 *                      all the input units, even in case of a buffer overflow.
 * @param src           The original source string
 * @param srcLength     The length of the original string. If -1, then src must be zero-terminated.
 * @param pErrorCode    Must be a valid pointer to an error code value,
 *                      which must not indicate a failure before the function call.
 * @return The pointer to destination buffer.
 * @stable ICU 2.0
 * @see u_strFromUTF8WithSub
 * @see u_strFromUTF8Lenient
 */
UChar* u_strFromUTF8(UChar *dest, int32_t destCapacity, int32_t *pDestLength, const char *src, int32_t srcLength,
                     UErrorCode *pErrorCode);

/**
 * Convert a UTF-8 string to UTF-16.
 *
 * Same as u_strFromUTF8() except for the additional subchar which is output for
 * illegal input sequences, instead of stopping with the U_INVALID_CHAR_FOUND error code.
 * With subchar==U_SENTINEL, this function behaves exactly like u_strFromUTF8().
 *
 * @param dest          A buffer for the result string. The result will be zero-terminated if
 *                      the buffer is large enough.
 * @param destCapacity  The size of the buffer (number of UChars). If it is 0, then
 *                      dest may be NULL and the function will only return the length of the
 *                      result without writing any of the result string (pre-flighting).
 * @param pDestLength   A pointer to receive the number of units written to the destination. If
 *                      pDestLength!=NULL then *pDestLength is always set to the
 *                      number of output units corresponding to the transformation of
 *                      all the input units, even in case of a buffer overflow.
 * @param src           The original source string
 * @param srcLength     The length of the original string. If -1, then src must be zero-terminated.
 * @param subchar       The substitution character to use in place of an illegal input sequence,
 *                      or U_SENTINEL if the function is to return with U_INVALID_CHAR_FOUND instead.
 *                      A substitution character can be any valid Unicode code point (up to U+10FFFF)
 *                      except for surrogate code points (U+D800..U+DFFF).
 *                      The recommended value is U+FFFD "REPLACEMENT CHARACTER".
 * @param pNumSubstitutions Output parameter receiving the number of substitutions if subchar>=0.
 *                      Set to 0 if no substitutions occur or subchar<0.
 *                      pNumSubstitutions can be NULL.
 * @param pErrorCode    Pointer to a standard ICU error code. Its input value must
 *                      pass the U_SUCCESS() test, or else the function returns
 *                      immediately. Check for U_FAILURE() on output or use with
 *                      function chaining. (See User Guide for details.)
 * @return The pointer to destination buffer.
 * @see u_strFromUTF8
 * @see u_strFromUTF8Lenient
 * @see u_strToUTF8WithSub
 * @stable ICU 3.6
 */
UChar* u_strFromUTF8WithSub(UChar *dest, int32_t destCapacity, int32_t *pDestLength, const char *src, int32_t srcLength,
                            UChar32 subchar, int32_t *pNumSubstitutions, UErrorCode *pErrorCode);

#if defined(__cplusplus)
}
#endif

/**
 * @}
 */

#endif //UTF_ICUUTF_H
