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

	void Call(int accountId, int callUid, AnsiString target, AnsiString extraHeaderLines, bool video, void *vidispParentHandle);
	void Answer(int callId, AnsiString audioRxMod="", AnsiString audioRxDev="", bool video = true, void *vidispParentHandle = NULL);
	void Transfer(int callId, AnsiString target);
	void SendDigit(int callId, char key);
	void GenerateTone(int callId,
		float amplitude1 = 0.0f, float frequency1 = 0.0f,
		float amplitude2 = 0.0f, float frequency2 = 0.0f,
		float amplitude3 = 0.0f, float frequency3 = 0.0f,
		float amplitude4 = 0.0f, float frequency4 = 0.0f);
	void Hold(int callId, bool state);
	void Mute(int callId, bool state);
	void Hangup(int callUid, int code=0, AnsiString reason = "Busy Here");
	void SetMsgLogging(bool enabled, bool onlyFirstLines);
	void SetAubufLogging(bool enabled);	
	void ReRegister(int accountId);
	void UnRegister(int accountId);
	void StartRing(AnsiString wavFile);
	void PlayStop(void);
	void StartRing2(AnsiString wavFile);
	void Record(int callUid, AnsiString wavFile, unsigned int channels, unsigned int side, unsigned int fileFormat, unsigned int bitrate);
	void RecordPause(int callUid);
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
	void SwitchVideoSource(int callId, AnsiString videoMod, AnsiString videoDev);
	void UpdateVolume(void);
	int SendCustomRequest(int requestId, AnsiString method, AnsiString target, AnsiString extraHeaderLines);
	int SendMessage(int requestId, int accountId, AnsiString target, AnsiString text);
	int ZrtpVerifySas(bool state);
};

#define UA ControlQueue::Instance()

#endif
