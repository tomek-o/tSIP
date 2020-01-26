/**
 * @file aubuf.c  Audio Buffer
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h> 
#include <string.h>
#include <rem_aubuf.h>

static bool debug_enabled = false;


/** Locked audio-buffer with almost zero-copy */
struct aubuf {
	struct list afl;
	struct lock *lock;
	size_t wish_sz;
	size_t cur_sz;
	size_t max_sz;
	bool filling;
	uint64_t ts;

	struct {
		size_t or;
		size_t ur;
	} stats;
};


struct auframe {
	struct le le;
	struct mbuf *mb;
};


static void auframe_destructor(void *arg)
{
	struct auframe *af = arg;

	list_unlink(&af->le);
	mem_deref(af->mb);
}


static void aubuf_destructor(void *arg)
{
	struct aubuf *ab = arg;

	list_flush(&ab->afl);
	mem_deref(ab->lock);
}


/**
 * Allocate a new audio buffer
 *
 * @param abp    Pointer to allocated audio buffer
 * @param min_sz Minimum buffer size
 * @param max_sz Maximum buffer size (0 for no max size)
 *
 * @return 0 for success, otherwise error code
 */
int aubuf_alloc(struct aubuf **abp, size_t min_sz, size_t max_sz)
{
	struct aubuf *ab;
	int err;

	if (!abp || !min_sz)
		return EINVAL;

	ab = mem_zalloc(sizeof(*ab), aubuf_destructor);
	if (!ab)
		return ENOMEM;

	err = lock_alloc(&ab->lock);
	if (err)
		goto out;

	ab->wish_sz = min_sz;
	ab->max_sz = max_sz;
	ab->filling = true;

 out:
	if (err)
		mem_deref(ab);
	else
		*abp = ab;

	return err;
}


/**
 * Append a PCM-buffer to the end of the audio buffer
 *
 * @param ab Audio buffer
 * @param mb Mbuffer with PCM samples
 *
 * @return 0 for success, otherwise error code
 */
int aubuf_append(struct aubuf *ab, struct mbuf *mb)
{
	struct auframe *af;

	if (!ab || !mb)
		return EINVAL;

	af = mem_zalloc(sizeof(*af), auframe_destructor);
	if (!af)
		return ENOMEM;

	af->mb = mem_ref(mb);

	lock_write_get(ab->lock);

	list_append(&ab->afl, &af->le, af);
	ab->cur_sz += mbuf_get_left(mb);

	if (ab->max_sz && ab->cur_sz > ab->max_sz) {
		if (debug_enabled) {
			++ab->stats.or;
			(void)re_printf("aubuf: %p overrun (cur=%zu)\n",
				ab, ab->cur_sz);
		}
		af = list_ledata(ab->afl.head);
		if (af) {
			ab->cur_sz -= mbuf_get_left(af->mb);
			mem_deref(af);
		}
	}

	lock_rel(ab->lock);

	return 0;
}


/**
 * Write PCM samples to the audio buffer
 *
 * @param ab Audio buffer
 * @param p  Pointer to PCM data
 * @param sz Number of bytes to write
 *
 * @return 0 for success, otherwise error code
 */
int aubuf_write(struct aubuf *ab, const uint8_t *p, size_t sz)
{
	struct mbuf *mb = mbuf_alloc(sz);
	int err;

	if (!mb)
		return ENOMEM;

	(void)mbuf_write_mem(mb, p, sz);
	mb->pos = 0;

	err = aubuf_append(ab, mb);

	mem_deref(mb);

	return err;
}


/**
 * Read PCM samples from the audio buffer. If there is not enough data
 * in the audio buffer, silence will be read.
 *
 * @param ab Audio buffer
 * @param p  Buffer where PCM samples are read into
 * @param sz Number of bytes to read
 */
void aubuf_read(struct aubuf *ab, uint8_t *p, size_t sz)
{
	struct le *le;

	if (!ab || !p || !sz)
		return;

	lock_write_get(ab->lock);


	if (ab->cur_sz < (ab->filling ? ab->wish_sz : sz)) {
		if (debug_enabled) {
			if (!ab->filling) {
				++ab->stats.ur;
				(void)re_printf("aubuf: %p underrun (cur=%zu)\n",
						ab, ab->cur_sz);
			}
		}
		ab->filling = true;
		memset(p, 0, sz);
		goto out;
	}

	ab->filling = false;

	le = ab->afl.head;

	while (le) {
		struct auframe *af = le->data;
		size_t n;

		le = le->next;

		n = min(mbuf_get_left(af->mb), sz);

		(void)mbuf_read_mem(af->mb, p, n);
		ab->cur_sz -= n;

		if (!mbuf_get_left(af->mb))
			mem_deref(af);

		if (n == sz)
			break;

		p  += n;
		sz -= n;
	}

 out:
	lock_rel(ab->lock);
}


/**
 * Timed read PCM samples from the audio buffer. If there is not enough data
 * in the audio buffer, silence will be read.
 *
 * @param ab    Audio buffer
 * @param ptime Packet time in [ms]
 * @param p     Buffer where PCM samples are read into
 * @param sz    Number of bytes to read
 *
 * @note This does the same as aubuf_read() except that it also takes
 *       timing into consideration.
 *
 * @return 0 if valid PCM was read, ETIMEDOUT if no PCM is ready yet
 */
int aubuf_get(struct aubuf *ab, uint32_t ptime, uint8_t *p, size_t sz)
{
	uint64_t now;
	int err = 0;

	if (!ab || !ptime)
		return EINVAL;

	lock_write_get(ab->lock);

	now = tmr_jiffies();
	if (!ab->ts)
		ab->ts = now;

	if (now < ab->ts) {
		err = ETIMEDOUT;
		goto out;
	}

	ab->ts += ptime;

 out:
	lock_rel(ab->lock);

	if (!err)
		aubuf_read(ab, p, sz);

	return err;
}


/**
 * Flush the audio buffer
 *
 * @param ab Audio buffer
 */
void aubuf_flush(struct aubuf *ab)
{
	if (!ab)
		return;

	lock_write_get(ab->lock);

	list_flush(&ab->afl);
	ab->filling = true;
	ab->cur_sz  = 0;
	ab->ts      = 0;

	lock_rel(ab->lock);
}


/**
 * Audio buffer debug handler, use with fmt %H
 *
 * @param pf Print function
 * @param ab Audio buffer
 *
 * @return 0 if success, otherwise errorcode
 */
int aubuf_debug(struct re_printf *pf, const struct aubuf *ab)
{
	int err;

	if (!ab)
		return 0;

	lock_read_get(ab->lock);
	err = re_hprintf(pf, "wish_sz=%zu cur_sz=%zu filling=%d",
			 ab->wish_sz, ab->cur_sz, ab->filling);

	if (debug_enabled) {
		err |= re_hprintf(pf, " [overrun=%zu underrun=%zu]",
			  ab->stats.or, ab->stats.ur);
	}

	lock_rel(ab->lock);

	return err;
}


/**
 * Get the current number of bytes in the audio buffer
 *
 * @param ab Audio buffer
 *
 * @return Number of bytes in the audio buffer
 */
size_t aubuf_cur_size(const struct aubuf *ab)
{
	size_t sz;

	if (!ab)
		return 0;

	lock_read_get(ab->lock);
	sz = ab->cur_sz;
	lock_rel(ab->lock);

	return sz;
}

void aubuf_stop_buffering(struct aubuf *ab)
{
	if (!ab)
		return;
	lock_write_get(ab->lock);
	ab->filling = false;
	ab->wish_sz = 0;
	lock_rel(ab->lock);
}

void aubuf_debug_enable(bool state)
{
	debug_enabled = state;
}


int aubuf_write_samp(struct aubuf *ab, const int16_t *sampv,
				   size_t sampc)
{
	return aubuf_write(ab, (const uint8_t *)sampv, sampc * 2);
}


void aubuf_read_samp(struct aubuf *ab, int16_t *sampv,
				   size_t sampc)
{
	aubuf_read(ab, (uint8_t *)sampv, sampc * 2);
}

int aubuf_get_samp(struct aubuf *ab, uint32_t ptime,
				 int16_t *sampv, size_t sampc)
{
	return aubuf_get(ab, ptime, (uint8_t *)sampv, sampc * 2);
}
