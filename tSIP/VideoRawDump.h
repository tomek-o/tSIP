//---------------------------------------------------------------------------

#ifndef VideoRawDumpH
#define VideoRawDumpH
//---------------------------------------------------------------------------

#include "baresip_base_config.h"

#ifdef USE_VIDEO

#include <System.hpp>

/** \brief Raw H.264 dump of received video of one call: complete access units (Annex-B)
	written as they are received, starting from the first keyframe received after start.
	Output is an elementary stream (.h264, no timing), playable e.g. with ffplay.
*/
enum VideoRawDumpFormat
{
	VRD_H264_RAW = 0,	///< elementary stream (.h264), no timing
	VRD_MP4 = 1		///< fragmented MP4 (.mp4), timed by arrival
};

namespace VideoRawDump
{
	/** \brief Install the decoder hook. Call once at startup, before video calls. */
	void Init(void);

	/** \brief True if the FFmpeg libraries in use have the MP4 output format */
	bool IsMp4Supported(void);

	/** \brief Start writing to file (replaces a previous file)
		\param dec decoder state of the call's video stream (video_dec_state()), must not be NULL
		\param callUid call of the video stream, for StopForCall()
		\param format output format
		\return 0 on success
	*/
	int Start(AnsiString fileName, const void *dec, unsigned int callUid,
		  VideoRawDumpFormat format);

	/** \brief Stop writing and close file */
	void Stop(void);

	/** \brief Stop writing if the recorded stream belongs to call (call ended) */
	void StopForCall(unsigned int callUid);

	bool IsActive(void);
}

#endif

#endif
