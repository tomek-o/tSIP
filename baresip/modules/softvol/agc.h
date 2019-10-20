#ifndef AGC_H
#define AGC_H

#include <stdint.h>
#include <stdlib.h>

struct agc_st {
	float gain;					// current gain
	uint16_t history[300];		// peak values from each audio frame
	unsigned int history_cnt;	// number of valid entries in history
	unsigned int history_pos;	// position to write to
};

void agc_reset(struct agc_st *st);

void agc_process(struct agc_st *st, int16_t *sampv, size_t sampc, uint16_t target, float max_gain, float attack_rate, float release_rate);

#endif
