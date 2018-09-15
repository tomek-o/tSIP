/*
 * Audio recorder
 */
#ifndef RecorderH
#define RecorderH

/** \note This simplified interface cannot handle multiple simultaneous calls
*/
int recorder_start(const char* const filename, unsigned int rec_channels);

#endif
