//---------------------------------------------------------------------------

#ifndef RecordingFileH
#define RecordingFileH
//---------------------------------------------------------------------------

#include <System.hpp>

/** \brief Naming and location of call recordings (audio and video), so that all
	recordings of one call get the same base name and go to the same folder
*/
namespace RecordingFile
{
	/** \brief Folder for call recordings, with trailing backslash; created if missing
		\return empty string if the folder could not be created
	*/
	AnsiString GetDir(void);

	/** \brief Base name of a call recording without extension:
		<dir>yyyymmdd_hhnnss_<1 incoming | 0 outgoing>_<peer number or URI, base64 or escaped>
		\param incoming call direction
		\param peerUri peer URI (used for incoming calls)
		\param callUri call URI (used for outgoing calls)
		\return empty string if the folder could not be created
	*/
	AnsiString MakeBaseName(bool incoming, AnsiString peerUri, AnsiString callUri);
}

#endif
