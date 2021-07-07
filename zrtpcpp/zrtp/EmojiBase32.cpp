//
// Created by werner on 26.12.15.
//
#include <iostream>
// stdlib.h required for Android NDK builds, not included within the above includes
#include <cstdlib>

#include <libzrtpcpp/EmojiBase32.h>

// I used the information on this Unicode page http://unicode.org/emoji/charts/full-emoji-list.html
/*
Some technical details:
- add a new SAS algorithm to the ZRTP library that uses 32 Unicode code points
  instead of 32 ASCII characters.
- select 32 emojis that are easily distinguishable, known to everyone, not offending etc,
  using standard Unicode code points
- select colored emojis that look good on white and on black backgrounds (most
  emojis look good on white only)
- select emojis that are available on iOS, Android, Mac OS X (Windows?)
- add some code to return the resulting SAS string in UTF-8 encoding, suitable
  for most platforms except Java.

To use the codes for Java we need to translate the UTF-8 encoding into UTF-16
encoding. Because most of the emojis below are Unicode supplementary characters
the UTF-8 to UTF-16 conversion must generate the necessary UTF-16 surrogate pairs.

To support the UTF-8 / UTF-16 conversion the common directory contains conversion
functions that I extracted from ICU C++/C library source.
*/
// The comments are:                  Seq. Nr.  Name
static const char32_t emojis[] = {
		0x0001f601,        // 0002     GRINNING FACE WITH SMILING EYES
		0x0001f63a,        // 0080     SMILING CAT FACE WITH OPEN MOUTH
		0x0001f465,        // 0270     BUSTS IN SILHOUETTE
        0x0001f332,        // 0611     EVERGREEN TREE
		0x0001f45f,        // 0516     ATHLETIC SHOE
		0x0000270b,        // 0394     RAISED HAND
		0x0001f44d,        // 0412     THUMBS UP SIGN
		0x0001f435,        // 0528     MONKEY FACE
		0x0001f434,        // 0540     HORSE FACE
		0x0001f40d,        // 0580     SNAKE
		0x0001f41f,        // 0586     FISH
		0x0001f338,        // 0601     CHERRY BLOSSOM
		0x0001f310,        // 0694     GLOBE WITH MERIDIANS
		0x0001f3e0,        // 0711     HOUSE BUILDING
		0x0001f31e,        // 0876     SUN WITH FACE
		0x0001f698,        // 0779     ONCOMING AUTOMOBILE
		0x0001f535,        // 1367     LARGE BLUE CIRCLE
		0x0001f6a2,        // 0804     SHIP
		0x0001f53a,        // 1358     UP-POINTING RED TRIANGLE
		0x0001f42a,        // 0554     DROMEDARY CAMEL
		0x0001f525,        // 0903     FIRE
		0x0001f388,        // 0911     BALLOON
		0x0001f426,        // 0575     BIRD
		0x0001f50d,        // 1052     LEFT-POINTING MAGNIFYING GLASS
		0x0001f4d7,        // 1064     GREEN BOOK
		0x0001f4a1,        // 1058     ELECTRIC LIGHT BULB
		0x0001f536,        // 1354     LARGE ORANGE DIAMOND
		0x0001f528,        // 1134     HAMMER
		0x0001f55B,        // 0837     CLOCK FACE TWELVE OCLOCK
		0x0001f31f,        // 0878     GLOWING STAR
		0x0000274e,        // 1232     NEGATIVE SQUARED CROSS MARK
		0x0001f6a9         // 1157     TRIANGULAR FLAG ON POST
};

EmojiBase32::EmojiBase32(const unsigned char* data, size_t noOfBits)
{
    b2a_l(data, (noOfBits+7)/8, noOfBits);
}

using namespace std;

void EmojiBase32::b2a_l(const unsigned char* os, size_t len, const size_t lengthinbits) {

    /* if lengthinbits is not a multiple of 8 then this is allocating
     * space for 0, 1, or 2 extra quintets that will be truncated at the
     * end of this function if they are not needed
     */
    u32string result(static_cast<size_t>(divceil(static_cast<int>(len * 8), 5)), ' ');

    /* index into the result buffer, initially pointing to the
     * "one-past-the-end" quintet
     */
    size_t resp = result.size();

    /* pointer into the os buffer, initially pointing to the
     * "one-past-the-end" octet
     */
    const unsigned char* osp = os + len;

    /* Now this is a real live Duff's device.  You gotta love it. */

    unsigned long x = 0;	// to hold up to 32 bits worth of the input
    switch ((osp - os) % 5) {

        case 0:
            do {
                x = *--osp;
                result[--resp] = emojis[x % 32]; /* The least sig 5 bits go into the final quintet. */
                x /= 32;	/* ... now we have 3 bits worth in x... */
                case 4:
                    x |= ((unsigned long)(*--osp)) << 3; /* ... now we have 11 bits worth in x... */
                    result[--resp] = emojis[x % 32];
                    x /= 32; /* ... now we have 6 bits worth in x... */
                    result[--resp] = emojis[x % 32];
                    x /= 32; /* ... now we have 1 bits worth in x... */
                case 3:
                    x |= ((unsigned long)(*--osp)) << 1; /* The 8 bits from the 2-indexed octet.
							    So now we have 9 bits worth in x... */
                    result[--resp] = emojis[x % 32];
                    x /= 32; /* ... now we have 4 bits worth in x... */
                case 2:
                    x |= ((unsigned long)(*--osp)) << 4; /* The 8 bits from the 1-indexed octet.
							    So now we have 12 bits worth in x... */
                    result[--resp] = emojis[x%32];
                    x /= 32; /* ... now we have 7 bits worth in x... */
                    result[--resp] = emojis[x%32];
                    x /= 32; /* ... now we have 2 bits worth in x... */
                case 1:
                    x |= ((unsigned long)(*--osp)) << 2; /* The 8 bits from the 0-indexed octet.
							    So now we have 10 bits worth in x... */
                    result[--resp] = emojis[x%32];
                    x /= 32; /* ... now we have 5 bits worth in x... */
                    result[--resp] = emojis[x];
            } while (osp > os);
    } /* switch ((osp - os.buf) % 5) */

    /* truncate any unused trailing zero quintets */
    encoded = result.substr(0, static_cast<size_t>(divceil(static_cast<int>(lengthinbits), 5)));
    return;
}


// I copied/extracted the following macro from the ICU C/C++ source code
// to support compilation/linking with older Android NDK and other systems
// that do not (yet) support the full Unicode functions.
//
// The license for ICU code is available at:
// http://source.icu-project.org/repos/icu/icu/trunk/license.html
/**
 * Append a code point to a string, overwriting 1 to 4 bytes.
 * The offset points to the current end of the string contents
 * and is advanced (post-increment).
 * "Unsafe" macro, assumes a valid code point and sufficient space in the string.
 * Otherwise, the result is undefined.
 *
 * @param s const uint8_t * string buffer
 * @param i string offset
 * @param c code point to append
 * @see U8_APPEND
 * @stable ICU 2.4
 */
#define U8_APPEND_UNSAFE(s, i, c) { \
    if((uint32_t)(c)<=0x7f) { \
        (s)[(i)++]=(uint8_t)(c); \
    } else { \
        if((uint32_t)(c)<=0x7ff) { \
            (s)[(i)++]=(uint8_t)(((c)>>6)|0xc0); \
        } else { \
            if((uint32_t)(c)<=0xffff) { \
                (s)[(i)++]=(uint8_t)(((c)>>12)|0xe0); \
            } else { \
                (s)[(i)++]=(uint8_t)(((c)>>18)|0xf0); \
                (s)[(i)++]=(uint8_t)((((c)>>12)&0x3f)|0x80); \
            } \
            (s)[(i)++]=(uint8_t)((((c)>>6)&0x3f)|0x80); \
        } \
        (s)[(i)++]=(uint8_t)(((c)&0x3f)|0x80); \
    } \
}

string EmojiBase32::u32StringToUtf8(const u32string& in)
{
    string result;

    string out(6, '\0');
    for (size_t n = 0; n < in.size(); ++n) {
        size_t offset = 0;
        U8_APPEND_UNSAFE(&out[0], offset, in[n]);
        result.append(out, 0, offset);
    }
    return result;
}
