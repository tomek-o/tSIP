/**
 * @file
 * "NULL" audio device. For input: silence, for output: discarded data.
 */
#include <re.h>
#include <rem.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <baresip.h>
#include "nullaudio.h"


#define DEBUG_MODULE "nullaudio"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


static struct ausrc *ausrc;
static struct auplay *auplay;


static int nullaudio_init(void)
{
	int err;

	err  = ausrc_register(&ausrc, "nullaudio", nullaudio_src_alloc);
	err |= auplay_register(&auplay, "nullaudio", nullaudio_play_alloc);

	return err;
}


static int nullaudio_close(void)
{
	ausrc = mem_deref(ausrc);
	auplay = mem_deref(auplay);

	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(nullaudio) = {
	"nullaudio",
	"sound",
	nullaudio_init,
	nullaudio_close
};
