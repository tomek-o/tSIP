/**
 * @file session.h  GNU ZRTP: Session class
 *
 * Copyright (C) 2010 - 2017 Alfred E. Heggestad
 */
#ifndef __SESSION_H
#define __SESSION_H


#include "gzrtp_stream.h"


struct call;
class Stream;
class ZRTPConfig;

class Session {
public:
	Session(const ZRTPConfig& config, struct call *call);

	~Session();

	Stream *create_stream(const ZRTPConfig& config,
	                      udp_sock *rtpsock,
	                      udp_sock *rtcpsock,
	                      uint32_t local_ssrc,
	                      StreamMediaType media_type);

	int start_stream(Stream *stream);
	int id() const { return m_id; }

	bool request_master(Stream *stream);
	void on_secure(Stream *stream);

	/** uid of the call this session belongs to (0 if not known); last known uid is kept after the call is gone */
	unsigned int call_uid();

	static int verify_sas(unsigned int call_uid, bool verify);

private:
	static std::vector<Session *> s_sessl;

	const bool m_start_parallel;
	int m_id;
	struct call *m_call;
	unsigned int m_call_uid;
	std::vector<Stream *> m_streams;
	Stream *m_master;
	unsigned int m_encrypted;
};


#endif // __SESSION_H

