#include "gate.h"
#include <stdlib.h>
#include <string.h>
#include <re.h>
#include <baresip.h>
#include <math.h>


#define DEBUG_MODULE "gate"
#define DEBUG_LEVEL 6
#include <re_dbg.h>
#define LOG_PROMPT DEBUG_MODULE": "


void gate_reset(struct gate_st *st, unsigned int srate) {
    memset(st, 0, sizeof(*st));
	st->gain = 1.0f;
	st->srate = srate;
}

void gate_process(struct gate_st *st, int16_t *sampv, size_t sampc, uint16_t close_threshold, unsigned int hold_ms, float attack_ms, float release_ms) {
	unsigned int i;
	unsigned int max_hold_samples;
	float attack_per_sample, release_per_sample;

	if (st->srate == 0)
		return;

	max_hold_samples = hold_ms * st->srate / 1000;
	if (attack_ms == 0) {
		attack_per_sample = 1.0f;
	} else {
		attack_per_sample = 1000.0f / (attack_ms * st->srate);
	}
	if (release_ms == 0) {
		release_per_sample = 1.0f;
	} else {
		release_per_sample = 1000.0f / (release_ms * st->srate);
	}

	for (i = 0; i < sampc; i++) {
		float val = sampv[i];
		if (fabs(val) > close_threshold) {
			if (st->opened == false) {
				DEBUG_INFO(LOG_PROMPT"TX audio gate: opened\n");
				st->opened = true;
			}
			st->hold_samples = 0;
		} else {
			if (st->opened) {
				st->hold_samples++;
				if (st->hold_samples > max_hold_samples) {
					st->opened = false;
					DEBUG_INFO(LOG_PROMPT"TX audio gate: closing\n");					
				}
			}
		}
		if (st->opened) {
			st->gain += attack_per_sample;
			if (st->gain > 1.0f)
				st->gain = 1.0f;
		} else {
			st->gain -= release_per_sample;
			if (st->gain < 0.0f)
				st->gain = 0.0f;
		}
		sampv[i] = val * st->gain;
	}
}


