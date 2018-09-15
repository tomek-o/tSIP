/**
 * @file g729.c  G.729 audio codec module
 */
#include <stdlib.h>
#include <string.h>
#include <re.h>
#include <baresip.h>
#include <module.h>
#include <assert.h>

#include "g729a_v11/typedef.h"
#include "g729a_v11/basic_op.h"
#include "g729a_v11/ld8a.h"
#include "g729a_v11/tab_ld8a.h"
#include "g729a_v11/util.h"
#include "g729a_v11/pre_proc.h"

extern mem_alloc_cb mem_alloc_fn;
extern mem_deref_cb mem_deref_fn;
extern aucodec_register_cb aucodec_register_fn;
extern aucodec_unregister_cb aucodec_unregister_fn;

struct auenc_state {
    CodState *enc;
};

static void auenc_state_destructor(void *arg) {
    struct auenc_state *s = arg;

    if (s->enc)
        free(s->enc);
}

struct audec_state {
    DecState *dec;
};

static void audec_state_destructor(void *arg) {
    struct audec_state *s = arg;

    if (s->dec)
        free(s->dec);
}


static int encode_update(struct auenc_state **aesp,
                         const struct aucodec *ac,
                         struct auenc_param *prm, const char *fmtp) {
    struct auenc_state *st;
    int err = 0;
    (void)prm;
    (void)fmtp;

    if (!aesp || !ac)
        return EINVAL;

    if (*aesp)
        return 0;

    st = mem_alloc_fn(sizeof(*st), auenc_state_destructor);
    if (!st)
        return ENOMEM;

    st->enc = Init_Coder_ld8a();
    if (!st->enc)
        return ENOMEM;
    Init_Pre_Process(st->enc);

//out:
    if (err)
        mem_deref_fn(st);
    else
        *aesp = st;

    return err;
}


static int decode_update(struct audec_state **adsp,
                         const struct aucodec *ac, const char *fmtp) {
    struct audec_state *st;
    int err = 0;
    (void)fmtp;

    if (!adsp || !ac)
        return EINVAL;

    if (*adsp)
        return 0;

    st = mem_alloc_fn(sizeof(*st), audec_state_destructor);
    if (!st)
        return ENOMEM;

    st->dec = Init_Decod_ld8a();
    if (!st->dec)
        return ENOMEM;
    Init_Post_Filter(st->dec);
    Init_Post_Process(st->dec);

//out:
    if (err)
        mem_deref_fn(st);
    else
        *adsp = st;

    return err;
}

static void g729_encode(CodState *state, const short *decoded, unsigned char *encoded) {
    int16_t parm[PRM_SIZE];
    Copy((int16_t *) decoded, state->new_speech, 80);
    Pre_Process(state, state->new_speech, 80);
    Coder_ld8a(state, parm);
    Store_Params(parm, encoded);
}

static int encode(struct auenc_state *st, uint8_t *buf, size_t *len,
                  const int16_t *sampv, size_t sampc) {
    assert(sampc % 80 == 0);
    int total_len = 0;
    while (sampc >= 80) {
        g729_encode(st->enc, sampv, buf);
        sampv += 80;
        sampc -= 80;
        buf += 10;
        total_len += 10;
    }
    *len = total_len;
    return 0;
}

void g729_decode(DecState *state, short *decoded, const unsigned char *encoded, int len) {
    int16_t *synth;
    int16_t parm[PRM_SIZE + 1];

    Restore_Params(encoded, &parm[1]);
    synth = state->synth_buf + M;

    parm[0] = 1;
    for (int i = 0; i < PRM_SIZE; i++) {
        if (parm[i + 1] != 0) {
            parm[0] = 0;
            break;
        }
    }

    parm[4] = Check_Parity_Pitch(parm[3], parm[4]);

    Decod_ld8a(state, parm, synth, state->Az_dec, state->T2, &state->bad_lsf);
    Post_Filter(state, synth, state->Az_dec, state->T2);
    Post_Process(state, synth, L_FRAME);
    memmove(decoded, synth, 2 * L_FRAME);
};

static int decode(struct audec_state *st, int16_t *sampv, size_t *sampc,
                  const uint8_t *buf, size_t len) {
    if (!st || !sampv || !buf)
        return EINVAL;
    assert(len % 10 == 0);
    int total_sampc = 0;
    while (len >= 10) {
        g729_decode(st->dec, sampv, buf, 10);
        buf += 10;
        len -= 10;
        sampv += 80;
        total_sampc += 80;
    }
    *sampc = total_sampc;

    return 0;
}


static struct aucodec g729 = {
    LE_INIT, "18", "G729", 8000, 1, NULL,
    encode_update, encode,
    decode_update, decode, NULL,
    NULL, NULL
};


static int module_init(void) {
    aucodec_register_fn(&g729);
    return 0;
}


static int module_close(void) {
    aucodec_unregister_fn(&g729);
    return 0;
}


/** Module exports */
EXPORT_SYM const struct mod_export DECL_EXPORTS(g729) = {
    "g729",
    "codec",
    module_init,
    module_close
};
