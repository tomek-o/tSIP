#ifndef BARESIP_DTMF_H
#define BARESIP_DTMF_H

/* DTMF inband transmitter */
#define	DTMF_MAX_DIGITS 128

struct dtmf_state {
	float	kx[2];
	float	ky[2];
	uint32_t duration;
};

struct dtmf_generator {
	struct dtmf_state state[DTMF_MAX_DIGITS];
	volatile uint16_t input_pos;
	volatile uint16_t output_pos;

	float	x[2];
	float	y[2];
	uint32_t duration;
};

int dtmf_is_empty(struct dtmf_generator *pg);

int	dtmf_queue_digit(struct dtmf_generator *, uint8_t digit,
			 uint32_t sample_rate, uint16_t tone_duration,
			 uint16_t gap_duration);
int	dtmf_get_sample(struct dtmf_generator *pg, int16_t *psamp);

#endif
