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

	void Call(int accountId, unsigned int callUid, AnsiString target, AnsiString extraHeaderLines, bool video, void *vidispParentHandle);
	void Answer(unsigned int callUid, AnsiString audioRxMod="", AnsiString audioRxDev="", bool video = true, void *vidispParentHandle = NULL);
	void Transfer(unsigned int callUid, AnsiString target);
	void TransferReplace(unsigned int callUid, unsigned int callReplaceUid);
	void CallStartAudioExtraSource(unsigned int callUid);
	void SendDigit(unsigned int callUid, char key);
	void GenerateTone(unsigned int callUid,
		float amplitude1 = 0.0f, float frequency1 = 0.0f,
		float amplitude2 = 0.0f, float frequency2 = 0.0f,
		float amplitude3 = 0.0f, float frequency3 = 0.0f,
		float amplitude4 = 0.0f, float frequency4 = 0.0f);
	void Hold(unsigned int callUid, bool state);
	void Mute(unsigned int callUid, bool state);
	void Hangup(unsigned int callUid, int code=0, AnsiString reason = "Busy Here");
	void SetMsgLogging(bool enabled, bool onlyFirstLines);
	void SetAubufLogging(bool enabled);	
	void ReRegister(int accountId);
	void UnRegister(int accountId);
	void StartRing(AnsiString wavFile);
	void PlayStop(void);
	void StartRing2(AnsiString wavFile);
	void Record(unsigned int callUid, AnsiString wavFile, unsigned int channels, unsigned int side, unsigned int fileFormat, unsigned int bitrate);
	void RecordPause(unsigned int callUid);
	/** \brief Start transmitting RTP to specified targee
		\param target address IP + port
		\param pagingTxWaveFile audio file to be transmitted; if not specified default audio source is used
	*/
	void PagingTx(AnsiString target, AnsiString pagingTxWaveFile, AnsiString codec, unsigned int ptime);
	/** \brief Change audio source device for call
	*/
	void SwitchAudioSource(unsigned int callUid, AnsiString audioMod, AnsiString audioDev);
	/** \brief Change audio player device for call
	*/
	void SwitchAudioPlayer(unsigned int callUid, AnsiString audioMod, AnsiString audioDev);
	void SwitchVideoSource(unsigned int callUid, AnsiString videoMod, AnsiString videoDev);
	void UpdateVolume(void);
	int SendCustomRequest(int requestId, AnsiString method, AnsiString target, AnsiString extraHeaderLines);
	int SendMessage(int requestId, int accountId, AnsiString target, AnsiString text);
	int ZrtpVerifySas(bool state);
	int ConferenceStart(void);
};

#define UA ControlQueue::Instance()

#endif
