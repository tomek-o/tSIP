#include "tone_generator.h"
#include <string.h>
#include <math.h>

#define MIN_AMPLITUDE 0.000001f

static int16_t saturate_s16(float val) {
	if (val < -32767) {
		return -32767;
	} else if (val > 32767) {
		return 32767;
	} else {
		return val + 0.5f;
	}
}

int tone_generator_init(struct tone_generator *generator) {
	unsigned int i;
	if (generator == NULL)
		return EINVAL;
	memset(generator, 0, sizeof(*generator));
	for (i=0; i<ARRAY_SIZE(generator->channels); i++) {
		struct tone_generator_channel *ch = &generator->channels[i];
		ch->amplitude = 0.0f;
	}
	return 0;
}

int tone_generator_start(struct tone_generator *generator, unsigned int tone_id, float amplitude, float frequency) {
    struct tone_generator_channel *ch;
	if (generator == NULL)
		return EINVAL;
	if (tone_id >= ARRAY_SIZE(generator->channels))
		return EINVAL;
	if (amplitude < MIN_AMPLITUDE || frequency < 0.000001f)
		return EINVAL;
	ch = &generator->channels[tone_id];
	ch->frequency = frequency;
	ch->amplitude = 32767.0f * amplitude;
	return 0;
}

int tone_generator_stop(struct tone_generator *generator, unsigned int tone_id) {
	if (generator == NULL)
		return EINVAL;
	if (tone_id >= ARRAY_SIZE(generator->channels))
		return EINVAL;
	generator->channels[tone_id].amplitude = 0.0f;
	return 0;
}

void tone_generator_process(struct tone_generator *generator, uint32_t sample_rate, int16_t *sampv, uint32_t sampc) {
	int active = 0;
	unsigned int i;
	for (i=0; i<ARRAY_SIZE(generator->channels); i++) {
		struct tone_generator_channel *ch = &generator->channels[i];
		if (ch->amplitude > MIN_AMPLITUDE)
		{
			active = 1;
			break;
		}
	}
	if (!active)
		return;
	for (i=0; i<sampc; i++) {
		float val = 0.0f;
		int j;
		for (j=0; j<ARRAY_SIZE(generator->channels); j++) {
			struct tone_generator_channel *ch = &generator->channels[j];
			if (ch->amplitude > MIN_AMPLITUDE) {
				val += ch->amplitude * sin(2.0f * M_PI * ch->frequency / sample_rate * generator->sample_id);
			}
		}
		sampv[i] = saturate_s16(val);
		generator->sample_id++;
	}
}
