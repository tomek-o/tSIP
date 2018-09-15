/*
 * SpanDSP - a series of DSP components for telephony
 *
 * g722.h - The ITU G.722 codec.
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2005 Steve Underwood
 *
 *  Despite my general liking of the GPL, I place my own contributions 
 *  to this code in the public domain for the benefit of all mankind -
 *  even the slimy ones who might try to proprietize my work and use it
 *  to my detriment.
 *
 * Based on a single channel G.722 codec which is:
 *
 *****    Copyright (c) CMU    1993      *****
 * Computer Science, Speech Group
 * Chengxiang Lu and Alex Hauptmann
 *
 * $Id: g722_private.h,v 1.1 2012/08/07 11:33:45 sobomax Exp $
 */


/*! \file */

#if !defined(_G722_PRIVATE_H_)
#define _G722_PRIVATE_H_

/*! \page g722_page G.722 encoding and decoding
\section g722_page_sec_1 What does it do?
The G.722 module is a bit exact implementation of the ITU G.722 specification for all three
specified bit rates - 64000bps, 56000bps and 48000bps. It passes the ITU tests.

To allow fast and flexible interworking with narrow band telephony, the encoder and decoder
support an option for the linear audio to be an 8k samples/second stream. In this mode the
codec is considerably faster, and still fully compatible with wideband terminals using G.722.

\section g722_page_sec_2 How does it work?
???.
*/

typedef struct g722_encode_state G722_ENC_CTX;
#define _G722_ENC_CTX_DEFINED
typedef struct g722_decode_state G722_DEC_CTX;
#define _G722_DEC_CTX_DEFINED

struct g722_encode_state
{
    /*! TRUE if the operating in the special ITU test mode, with the band split filters
             disabled. */
    int itu_test_mode;
    /*! TRUE if the G.722 data is packed */
    int packed;
    /*! TRUE if encode from 8k samples/second */
    int eight_k;
    /*! 6 for 48000kbps, 7 for 56000kbps, or 8 for 64000kbps. */
    int bits_per_sample;

    /*! Signal history for the QMF */
    int x[24];

    struct
    {
        int s;
        int sp;
        int sz;
        int r[3];
        int a[3];
        int ap[3];
        int p[3];
        int d[7];
        int b[7];
        int bp[7];
        int sg[7];
        int nb;
        int det;
    } band[2];

    unsigned int in_buffer;
    int in_bits;
    unsigned int out_buffer;
    int out_bits;
};

struct g722_decode_state
{
    /*! TRUE if the operating in the special ITU test mode, with the band split filters
             disabled. */
    int itu_test_mode;
    /*! TRUE if the G.722 data is packed */
    int packed;
    /*! TRUE if decode to 8k samples/second */
    int eight_k;
    /*! 6 for 48000kbps, 7 for 56000kbps, or 8 for 64000kbps. */
    int bits_per_sample;

    /*! Signal history for the QMF */
    int x[24];

    struct
    {
        int s;
        int sp;
        int sz;
        int r[3];
        int a[3];
        int ap[3];
        int p[3];
        int d[7];
        int b[7];
        int bp[7];
        int sg[7];
        int nb;
        int det;
    } band[2];
    
    unsigned int in_buffer;
    int in_bits;
    unsigned int out_buffer;
    int out_bits;
};

#endif
