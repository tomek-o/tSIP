/*
 * Audio recorder
 */
#ifndef BARESIP_RECORDER_H
#define BARESIP_RECORDER_H

#ifdef __cplusplus
extern "C" {
#endif

enum recorder_state
{
	RECORDER_STATE_IDLE,
	RECORDER_STATE_ACTIVE,
	RECORDER_STATE_PAUSED
};

struct recorder_st;

typedef void (recorder_state_h)(struct recorder_st *recorder, enum recorder_state state);

void recorder_init(recorder_state_h *state_h);


enum recorder_side
{
	RECORDER_SIDE_BOTH = 0,	// record both parties (mixed)
	RECORDER_SIDE_LOCAL,	// record only local party (i.e. what comes from microphone)
	RECORDER_SIDE_REMOTE	// record only remote party
};

enum recorder_file_format
{
	RECORDER_FILE_FORMAT_WAV = 0,
	RECORDER_FILE_FORMAT_OPUS_OGG
};

/** \note This simplified interface cannot handle multiple simultaneous calls
	\param filename output file
	\param rec_channels 1 (recording as mono, both call parties mixed on only one call party) or 2 (recording two call parties, local and remote as two separate channels)
	\param rec_side which call side (local/remote) to record; used only if recording as mono/single channel
	\return 0 on success
*/
int recorder_start(const char* const filename, unsigned int rec_channels, enum recorder_side rec_side, enum recorder_file_format rec_format, unsigned int rec_bitrate);

/** \brief Toggle recorder active/paused
*/
void recorder_pause_resume(void);

void recorder_pause(void);

#ifdef __cplusplus
}
#endif

#endif
