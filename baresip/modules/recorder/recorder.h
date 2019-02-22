/*
 * Audio recorder
 */
#ifndef RecorderH
#define RecorderH

/** \note This simplified interface cannot handle multiple simultaneous calls
	\param filename output file
	\param rec_channels 1 (recording as mono, both call parties mixed on only one call party) or 2 (recording two call parties, local and remote as two separate channels)
	\param rec_side which call side (local/remote) to record; used only if recording as mono/single channel
	\return 0 on success
*/
int recorder_start(const char* const filename, unsigned int rec_channels, enum recorder_side rec_side);

#endif
