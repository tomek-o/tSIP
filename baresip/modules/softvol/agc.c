#include "agc.h"
#include <stdlib.h>
#include <string.h>
#include <re.h>
#include <baresip.h>


#define DEBUG_MODULE "agc"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


void agc_reset(struct agc_st *st) {
    memset(st, 0, sizeof(*st));
	st->gain = 1.0f;
	st->history_cnt = 0;
}

void agc_process(struct agc_st *st, int16_t *sampv, size_t sampc, uint16_t target, float max_gain, float attack_rate, float release_rate) {
	unsigned int i;
	uint16_t peak = 0;

	// get peak value from frame
	for (i = 0; i < sampc; i++) {
		uint16_t abs_val;
		if (sampv[i] >= 0) {
			abs_val = sampv[i];
		} else {
        	abs_val = - sampv[i];
		}
		if (abs_val > peak) {
			peak = abs_val;
		}
	}
	st->history[st->history_pos] = peak;
	st->history_pos++;
	if (st->history_pos >= sizeof(st->history)/sizeof(st->history[0])) {
		st->history_pos = 0;
	}
	if (st->history_cnt < sizeof(st->history)/sizeof(st->history[0])) {
		st->history_cnt++;
	}

	// start actual processing when 1/4 of history buffer is filled
	if (st->history_cnt >= (sizeof(st->history)/sizeof(st->history[0]) / 4))
	{
		uint16_t history_peak = 0;
		float target_gain;
		for (i = 0; i < st->history_cnt; i++) {
			if (st->history[i] > history_peak) {
				history_peak = st->history[i];
			}
		}
		target_gain = (float)target / history_peak;
		if (target_gain >= st->gain) {
        	st->gain = (st->gain * (1.0f - attack_rate)) + (target_gain * attack_rate);
		} else {
			st->gain = (st->gain * (1.0f - release_rate)) + (target_gain * release_rate);
		}

		// saturate gain
		if (st->gain * peak > 16535) {
			st->gain = 16535.0f / peak;
		}

		if (st->gain > max_gain) {
        	st->gain = max_gain;
		}

		if (st->gain < 1.0f) {
        	st->gain = 1.0f;
		}

		// apply gain
		for (i = 0; i < sampc; i++) {
			sampv[i] = (float)sampv[i] * st->gain;
		}
	}
}


