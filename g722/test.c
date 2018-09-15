/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2016 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "g722_decoder.h"

#define BUFFER_SIZE 10

int
main()
{
    FILE *fi, *fo;
    uint8_t ibuf[BUFFER_SIZE];
    int16_t obuf[BUFFER_SIZE];
    G722_DEC_CTX *g722_dctx;
    int i;

    fi = fopen("test.g722", "r");
    if (fi == NULL) {
        fprintf(stderr, "cannot open test.g722\n");
        exit (1);
    }
    fo = fopen("test.raw", "w");
    if (fo == NULL) {
        fprintf(stderr, "cannot open test.raw\n");
        exit (1);
    }

    g722_dctx = g722_decoder_new(64000, G722_SAMPLE_RATE_8000);
    if (g722_dctx == NULL) {
        fprintf(stderr, "g722_decoder_new() failed\n");
        exit (1);
    }

    while (fread(ibuf, sizeof(ibuf), 1, fi) == 1) {
        g722_decode(g722_dctx, ibuf, sizeof(ibuf), obuf);
        for (i = 0; i < BUFFER_SIZE; i++)
            obuf[i] = htole16(obuf[i]);
        fwrite(obuf, sizeof(obuf), 1, fo);
        fflush(fo);
    }

    fclose(fi);
    fclose(fo);

    exit(0);
}
