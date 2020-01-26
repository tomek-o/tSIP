//---------------------------------------------------------------------------

#ifndef ControlQueueH
#define ControlQueueH
//---------------------------------------------------------------------------

#include "common/singleton.h"
#include <System.hpp>

class Command;

class ControlQueue: public CSingleton<ControlQueue>
{
private:
	ControlQueue();
	friend CSingleton<ControlQueue>;
public:
	int GetCommand(Command& cmd);

	void Call(int accountId, AnsiString target, AnsiString extraHeaderLines);
	void Answer(int callId, AnsiString audioRxMod="", AnsiString audioRxDev="");
	void Transfer(int callId, AnsiString target);
	void SendDigit(int callId, char key);
	void Hold(int callId, bool state);
	void Mute(int callId, bool state);
	void Hangup(int callId, int code=0);
	void SetMsgLogging(bool enabled);
	void SetAubufLogging(bool enabled);	
	void ReRegister(int accountId);
	void UnRegister(int accountId);
	void StartRing(AnsiString wavFile);
	void PlayStop(void);
	void Record(AnsiString wavFile, unsigned int channels, unsigned int side);
	/** \brief Start transmitting RTP to specified targee
		\param target address IP + port
		\param pagingTxWaveFile audio file to be transmitted; if not specified default audio source is used
	*/
	void PagingTx(AnsiString target, AnsiString pagingTxWaveFile, AnsiString codec, unsigned int ptime);
	/** \brief Change audio source device for call
	*/
	void SwitchAudioSource(int callId, AnsiString audioMod, AnsiString audioDev);
	/** \brief Change audio player device for call
	*/
	void SwitchAudioPlayer(int callId, AnsiString audioMod, AnsiString audioDev);
	void UpdateSoftvolTx(unsigned int val);
	void UpdateSoftvolRx(unsigned int val);
	int SendCustomRequest(int requestId, AnsiString method, AnsiString target, AnsiString extraHeaderLines);
	int SendMessage(int accountId, AnsiString target, AnsiString text);
};

#define UA ControlQueue::Instance()

#endif
