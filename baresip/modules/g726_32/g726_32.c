#include <g726.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>

#define DEBUG_MODULE "g726-32"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

struct auenc_state {
	g726_state_t enc;
};

struct audec_state {
	g726_state_t dec;
};

static void encode_destructor(void *arg)
{
	//struct auenc_state *st = arg;

}


static void decode_destructor(void *arg)
{
	//struct audec_state *st = arg;

}

static int encode_update(struct auenc_state **aesp, const struct aucodec *ac,
			 struct auenc_param *prm, const char *fmtp)
{
	struct auenc_state *st;
	int err = 0;
	(void)ac;
	(void)prm;
	(void)fmtp;

	if (!aesp)
		return EINVAL;
	if (*aesp)
		return 0;

	st = mem_zalloc(sizeof(*st), encode_destructor);
	if (!st)
		return ENOMEM;


	if (!g726_init(&st->enc, 32000, G726_PACKING_LEFT)) {
		DEBUG_WARNING("g726_init() encoder failed\n");
		err = EPROTO;
		goto out;
	}

 out:
	if (err)
		mem_deref(st);
	else
		*aesp = st;

	return err;
}


static int decode_update(struct audec_state **adsp,
			 const struct aucodec *ac, const char *fmtp)
{
	struct audec_state *st;
	int err = 0;
	(void)ac;
	(void)fmtp;

	if (!adsp)
		return EINVAL;
	if (*adsp)
		return 0;

	st = mem_zalloc(sizeof(*st), decode_destructor);
	if (!st)
		return ENOMEM;

	if (!g726_init(&st->dec, 32000, G726_PACKING_LEFT)) {
		DEBUG_WARNING("g726_init() decoder failed\n");
		err = EPROTO;
		goto out;
	}

 out:
	if (err)
		mem_deref(st);
	else
		*adsp = st;

	return err;
}


static int encode(struct auenc_state *st, uint8_t *buf, size_t *len,
		  const int16_t *sampv, size_t sampc)
{
	if (*len < sampc/2)
		return ENOMEM;
	*len = g726_encode(&st->enc, buf, sampv, sampc);
	return 0;
}


static int decode(struct audec_state *st, int16_t *sampv, size_t *sampc,
		  const uint8_t *buf, size_t len)
{
	if (*sampc < len*2)
		return ENOMEM;

	*sampc = g726_decode(&st->dec, sampv, buf, len);

	return 0;
}




static struct aucodec g726_32_mod = {
	LE_INIT, NULL, "G726-32", 8000, 1, NULL,
	encode_update, encode, decode_update, decode, NULL, NULL, NULL
};

static int module_init(void)
{
	aucodec_register(&g726_32_mod);
	return 0;
}

static int module_close(void)
{
	aucodec_unregister(&g726_32_mod);
	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(g726_32) = {
	"g726_32",
	"codec",
	module_init,
	module_close
};
