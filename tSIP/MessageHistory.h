//---------------------------------------------------------------------------

#ifndef MessageHistoryH
#define MessageHistoryH
//---------------------------------------------------------------------------

#include <System.hpp>
#include <time.h>
#include <vector>

/** \brief Persistent chat history for the MESSAGE (SIMPLE) feature.
 *
 * Each contact's messages are stored in a separate newline-delimited JSON
 * (NDJSON) file, one JSON object per line, inside a subfolder grouping all
 * contacts' files together. File names are derived from the contact
 * identifier (typically a SIP URI or number) with characters that are not
 * safe in Windows file names replaced.
 */
namespace MessageHistory
{
	struct Entry
	{
		time_t time;
		bool incoming;
		AnsiString contentType;	///< e.g. "text/plain"
		AnsiString body;		///< message body, UTF-8 encoded

		Entry(void):
			time(0),
			incoming(false)
		{
		}
	};

	/** \brief Set the directory the per-contact history files are stored in; created if missing. */
	void SetDir(AnsiString dir);

	/** \brief Append one message to the history file for the given contact. */
	void Append(AnsiString target, bool incoming, AnsiString contentType, AnsiString utf8Body);

	/** \brief Load up to \p count most recent messages for the given contact, oldest first. */
	std::vector<Entry> LoadLast(AnsiString target, unsigned int count);

	/** \brief Load up to \p count messages for the given contact older than the
	 *  \p alreadyLoaded most recent ones already shown (used for "load older
	 *  messages" pagination). Returns oldest-first; empty if there is nothing
	 *  older left.
	 */
	std::vector<Entry> LoadOlder(AnsiString target, unsigned int alreadyLoaded, unsigned int count);
}

#endif
