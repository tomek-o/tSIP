#ifndef BARESIP_TONE_GENERATOR_H
#define BARESIP_TONE_GENERATOR_H

#include <re_types.h>

struct tone_generator_channel
{
	float amplitude;
	float frequency;
};

struct tone_generator
{
	unsigned int sample_id;
	struct tone_generator_channel channels[4];
};

int tone_generator_init(struct tone_generator *generator);

int tone_generator_start(struct tone_generator *generator, unsigned int tone_id, float amplitude, float frequency);

int tone_generator_stop(struct tone_generator *generator, unsigned int tone_id);

void tone_generator_process(struct tone_generator *generator, uint32_t sample_rate, int16_t *sampv, uint32_t sampc);

#endif
