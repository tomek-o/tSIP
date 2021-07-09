/**
 * @file session.cpp  GNU ZRTP: Session class implementation
 *
 * Copyright (C) 2010 - 2017 Alfred E. Heggestad
 */
#include <stdint.h>

#include <re.h>
#include <baresip.h>
#include <baresip_zrtp.h>

#include "session.h"

#define DEBUG_MODULE "session"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

static baresip_zrtp_state_h *baresip_state_handler = NULL;

static void report_state(int session_id, Stream *stream) {
	if (baresip_state_handler) {
		struct zrtp_st st;
		memset(&st, 0, sizeof(st));
		if (stream) {
			st.active = true;
			str_ncpy(st.sas, stream->get_sas(), sizeof(st.sas));
			str_ncpy(st.cipher, stream->get_ciphers(), sizeof(st.cipher));
			st.verified = stream->sas_verified();
		}
		baresip_state_handler(session_id, &st);		
	}
}


std::vector<Session *> Session::s_sessl;


Session::Session(const ZRTPConfig& config)
	: m_start_parallel(config.start_parallel)
	, m_master(NULL)
	, m_encrypted(0)
{
	int newid = 1;
	for (std::vector<Session *>::iterator it = s_sessl.begin();
	     it != s_sessl.end(); ++it) {

		if ((*it)->id() >= newid)
			newid = (*it)->id() + 1;
	}

	m_id = newid;

	s_sessl.push_back(this);

	DEBUG_INFO("zrtp: New session <%d>\n", id());
}


Session::~Session()
{
	for (std::vector<Session *>::iterator it = s_sessl.begin();
	     it != s_sessl.end(); ++it) {

		if (*it == this) {
			s_sessl.erase(it);
			break;
		}
	}

	DEBUG_INFO("zrtp: Session <%d> is destroyed\n", id());
	report_state(-1, NULL);
}


Stream *Session::create_stream(const ZRTPConfig& config,
                               udp_sock *rtpsock,
                               udp_sock *rtcpsock,
                               uint32_t local_ssrc,
                               StreamMediaType media_type)
{
	int err = 0;

	Stream *st = new Stream (err, config, this, rtpsock, rtcpsock,
	                         local_ssrc, media_type);
	if (!st || err) {
		delete st;
		return NULL;
	}

	return st;
}


int Session::start_stream(Stream *stream)
{
	if (stream->started())
		return 0;

	m_streams.push_back(stream);

	// Start all streams in parallel using DH mode. This is a kind of
	// probing. The first stream to receive HelloACK will be the master
	// stream.  If disabled, only the first stream starts in DH (master)
	// mode.
	if (m_start_parallel) {
		if (m_master && m_encrypted)
			// If we already have a master in secure state,
			// start in multistream mode
			return stream->start(m_master);
		else
			// Start a new stream in DH mode
			return stream->start(NULL);
	}
	else {
		if (!m_master) {
			// Start the first stream in DH mode
			m_master = stream;
			return stream->start(NULL);
		}
		else if (m_encrypted) {
			// Master is in secure state; multistream
			return stream->start(m_master);
		}
	}

	return 0;
}


bool Session::request_master(Stream *stream)
{
	if (!m_start_parallel)
		return true;

	if (m_master)
		return false;

	// This is the first stream to receive HelloACK. It will be
	// used as the master for the other streams in the session.
	m_master = stream;
	// Stop other DH-mode streams. They will be started in the
	// multistream mode after the master enters secure state.
	for (std::vector<Stream *>::iterator it = m_streams.begin();
	     it != m_streams.end(); ++it) {

		if (*it != m_master) {
			(*it)->stop();
		}
	}

	return true;
}


void Session::on_secure(Stream *stream)
{
	++m_encrypted;

	if (m_encrypted == m_streams.size() && m_master) {
		DEBUG_WARNING("zrtp: All streams are encrypted (%s), "
		     "SAS is [%s] (%s)\n",
		     m_master->get_ciphers(),
		     m_master->get_sas(),
			 (m_master->sas_verified())? "verified" : "NOT VERIFIED");
		report_state(this->id(), m_master);
		return;
	}

	if (stream != m_master)
		return;

	// Master stream has just entered secure state. Start other
	// streams in the multistream mode.

	DEBUG_INFO("zrtp: Starting other streams (%d)\n", m_streams.size() - 1);

	for (std::vector<Stream *>::iterator it = m_streams.begin();
	     it != m_streams.end(); ++it) {

		if (*it != m_master) {
			(*it)->start(m_master);
		}
	}
}

/** \param verify true => verify, false => unverify */
int Session::verify_sas(int session_id, bool verify)
{
	Session *sess = NULL;


	for (std::vector<Session *>::iterator it = s_sessl.begin();
	     it != s_sessl.end(); ++it) {

		if ((*it)->id() == session_id) {
			sess = *it;
			break;
		}
	}

	if (!sess) {
		DEBUG_WARNING("zrtp: No session with id %d\n", session_id);
		return EINVAL;
	}

	if (!sess->m_master) {
		DEBUG_WARNING("zrtp: No master stream for the session with id %d\n",
		        sess->id());
		return EFAULT;
	}

	sess->m_master->verify_sas(verify);

	DEBUG_WARNING("zrtp: Session <%d>: SAS [%s] is %s\n", sess->id(),
	     sess->m_master->get_sas(),
	     (sess->m_master->sas_verified())? "verified" : "NOT VERIFIED");

	report_state(sess->id(), sess->m_master);

	return 0;
}

int baresip_zrtp_verify_sas(int session_id, bool verify)
{
	return Session::verify_sas(session_id, verify);
}

void baresip_zrtp_init(baresip_zrtp_state_h *state_h)
{
	baresip_state_handler = state_h;
}


