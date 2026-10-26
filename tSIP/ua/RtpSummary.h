//---------------------------------------------------------------------------

#ifndef RtpSummaryH
#define RtpSummaryH
//---------------------------------------------------------------------------

#include <System.hpp>

/** \brief RTP statistics of call audio stream, collected when the call ends
	(see baresip struct rtp_summary / call_rtp_summary())
*/
struct RtpSummary
{
	bool valid;					///< summary was collected (not set for entries from older versions)
	unsigned int txPackets;
	unsigned int rxPackets;
	unsigned int txBytes;		///< RTP payload bytes
	unsigned int rxBytes;
	bool rtcpValid;				///< RTCP based values are valid (peer sent reports)
	int rtcpRxLost;				///< lost on receive path
	int rtcpTxLost;				///< lost on transmit path, as reported by peer
	unsigned int rtcpRxJitterUs;
	unsigned int rtcpTxJitterUs;
	bool jbufValid;
	unsigned int jbufLate;
	unsigned int jbufLost;
	unsigned int jbufOos;		///< out of sequence
	unsigned int jbufDups;
	unsigned int jbufOverflow;
	unsigned int jbufUnderflow;
	unsigned int jbufFlush;
	unsigned int rxBufUnderrun;	///< player buffer
	unsigned int rxBufOverrun;
	unsigned int txBufUnderrun;	///< audio source buffer
	unsigned int txBufOverrun;
	unsigned int txBurst;		///< RTP packets sent much sooner than ptime after previous one
	unsigned int txLate;		///< RTP packets sent much later than ptime after previous one
	unsigned int txMaxGapMs;	///< longest interval between two sent RTP packets

	RtpSummary(void):
		valid(false),
		txPackets(0), rxPackets(0), txBytes(0), rxBytes(0),
		rtcpValid(false), rtcpRxLost(0), rtcpTxLost(0), rtcpRxJitterUs(0), rtcpTxJitterUs(0),
		jbufValid(false), jbufLate(0), jbufLost(0), jbufOos(0), jbufDups(0), jbufOverflow(0), jbufUnderflow(0), jbufFlush(0),
		rxBufUnderrun(0), rxBufOverrun(0), txBufUnderrun(0), txBufOverrun(0),
		txBurst(0), txLate(0), txMaxGapMs(0)
	{}

	/** \brief Anything suspicious in the buffers or in TX pacing? */
	bool HasBufferEvents(void) const {
		return (jbufLate + jbufLost + jbufOos + jbufDups + jbufOverflow + jbufUnderflow + jbufFlush +
			rxBufUnderrun + rxBufOverrun + txBufUnderrun + txBufOverrun + txBurst + txLate) > 0;
	}

	/** \brief One line text for log, similar to rtcpsummary module of baresip (parsable key=value; list) */
	AnsiString ToLogLine(void) const {
		AnsiString s;
		s.sprintf("PS=%u;PR=%u;BS=%u;BR=%u;", txPackets, rxPackets, txBytes, rxBytes);
		if (rtcpValid)
			s.cat_sprintf("PL=%d,%d;JI=%.1f,%.1f;", rtcpRxLost, rtcpTxLost, rtcpRxJitterUs / 1000.0, rtcpTxJitterUs / 1000.0);
		if (jbufValid)
			s.cat_sprintf("JB=late %u,lost %u,oos %u,dup %u,over %u,under %u,flush %u;", jbufLate, jbufLost, jbufOos, jbufDups, jbufOverflow, jbufUnderflow, jbufFlush);
		s.cat_sprintf("AB=rx under %u,rx over %u,tx under %u,tx over %u;TXP=burst %u,late %u,maxgap %u ms;",
			rxBufUnderrun, rxBufOverrun, txBufUnderrun, txBufOverrun, txBurst, txLate, txMaxGapMs);
		return s;
	}
};

#endif
