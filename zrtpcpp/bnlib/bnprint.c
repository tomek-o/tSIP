/*
 * bnprint.c - Print a bignum, for debugging purposes.
 *
 * Copyright (c) 1995  Colin Plumb.  All rights reserved.
 * For licensing and other legal details, see the file legal.c.
 */
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H 0
#endif
#if HAVE_CONFIG_H
#include "bnconfig.h"
#endif

/*
 * Some compilers complain about #if FOO if FOO isn't defined,
 * so do the ANSI-mandated thing explicitly...
 */
#ifndef NO_STRING_H
#define NO_STRING_H 0
#endif
#ifndef HAVE_STRINGS_H
#define HAVE_STRINGS_H 0
#endif

#include <stdio.h>
#include <stdint.h>

#if !NO_STRING_H
#include <string.h>
#elif HAVE_STRINGS_H
#include <strings.h>
#endif

#include "bn.h"
#include "bnprint.h"

#include "kludge.h"

int
bnPrint(FILE *f, char const *prefix, struct BigNum const *bn,
        char const *suffix)
{
    unsigned char temp[32];	/* How much to print on one line */
    unsigned len;
    size_t i;

    if (prefix && fputs(prefix, f) < 0)
        return EOF;

    len = (bnBits(bn) + 7)/ 8;

    if (!len) {
        if (putc('0', f) < 0)
            return EOF;
    } else {
        while (len > sizeof(temp)) {
            len -= sizeof(temp);
            bnExtractBigBytes(bn, temp, len, sizeof(temp));
            for (i = 0; i < sizeof(temp); i++)
                if (fprintf(f, "%02X", temp[i]) < 0)
                    return EOF;
                if (putc('\\', f) < 0 || putc('\n', f) < 0)
                    return EOF;
                if (prefix) {
                    i = strlen(prefix);
                    while (i--)
                        if (putc(' ', f) < 0)
                            return EOF;
                }
        }
        bnExtractBigBytes(bn, temp, 0, len);
        for (i = 0; i < len; i++)
            if (fprintf(f, "%02X", temp[i]) < 0)
                return EOF;
    }
    return suffix ? fputs(suffix, f) : 0;
}

/*
 * Convert an ASCII character to digit value
 */
static int getAsciiDigit( uint32_t *d, int radix, char c )
{
    *d = 255;

    if( c >= 0x30 && c <= 0x39 )
        *d = c - 0x30;
    if( c >= 0x41 && c <= 0x46 )
        *d = c - 0x37;
    if( c >= 0x61 && c <= 0x66 )
        *d = c - 0x57;

    if( *d >= (uint32_t)radix )
        return( -1 );

    return( 0 );
}

int
bnReadAscii(struct BigNum *X, char *s, int radix)
{
    int slen = strlen(s);
    int i, neg = 0;
    uint32_t d;

    bnSetQ(X, 0);
    for( i = 0; i < slen; i++ ) {
        if(i == 0 && s[i] == '-') {
            neg = 1;
            continue;
        }
        getAsciiDigit(&d, radix, s[i]);
        bnMulQ(X, X, radix);

        bnAddQ(X, d);
    }
    return(neg);
}
