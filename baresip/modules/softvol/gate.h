#ifndef GATE_H
#define GATE_H

#include <stdint.h>
#include <stdlib.h>

struct gate_st {
	int opened;
	float gain;					// current gain
	unsigned int hold_samples;
	unsigned int srate;
};

void gate_reset(struct gate_st *st, unsigned int srate);

void gate_process(struct gate_st *st, int16_t *sampv, size_t sampc, uint16_t close_threshold, unsigned int hold_ms, float attack_ms, float release_ms);

#endif
