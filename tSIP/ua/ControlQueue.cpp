//---------------------------------------------------------------------------


#pragma hdrstop

#include "ControlQueue.h"
#include "common/Mutex.h"
#include "common/ScopedLock.h"
#include "common/StaticCheck.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#include "common/fifo.h"
#include "Command.h"

#define ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))

namespace
{
	Fifo<Command, 1024> fifo;
	Mutex mutex;
}

ControlQueue::ControlQueue()
{

}

int ControlQueue::GetCommand(Command& cmd)
{
	ScopedLock<Mutex> lock(mutex);
	Command *tmpcmd = fifo.getReadable();
	if (tmpcmd == NULL)
	{
		return 1;
	}
	cmd = *tmpcmd;
	fifo.pop();
	return 0;
}

void ControlQueue::ReRegister(int accountId)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::REREGISTER;
	cmd->accountId = accountId;
	fifo.push();
}

void ControlQueue::UnRegister(int accountId)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::UNREGISTER;
	cmd->accountId = accountId;
	fifo.push();
}

void ControlQueue::Call(int accountId, AnsiString target, AnsiString extraHeaderLines, bool video, void *vidispParentHandle)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::CALL;
	cmd->accountId = accountId;
	cmd->target = target;
	if (extraHeaderLines != "")
	{
		if (extraHeaderLines.Length() < 2)
		{
			extraHeaderLines += "\r\n";
		}
		else
		{
			AnsiString end = extraHeaderLines.SubString(extraHeaderLines.Length() - 1, 2);
			if (end != "\r\n")
			{
        		extraHeaderLines += "\r\n";
			}
        }
    }
	cmd->extraHeaderLines = extraHeaderLines;
	cmd->video = video;
	cmd->vidispParentHandle = vidispParentHandle;
	fifo.push();
}

void ControlQueue::Answer(int callId, AnsiString audioRxMod, AnsiString audioRxDev, bool video, void *vidispParentHandle)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::ANSWER;
	cmd->audioMod = audioRxMod;
	cmd->audioDev = audioRxDev;
	cmd->callId = callId;
	cmd->video = video;
	cmd->vidispParentHandle = vidispParentHandle;
	fifo.push();
}

void ControlQueue::Transfer(int callId, AnsiString target)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::TRANSFER;
	cmd->callId = callId;
	cmd->target = target;
	fifo.push();
}

void ControlQueue::SendDigit(int callId, char key)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::SEND_DIGIT;
	cmd->callId = callId;
	cmd->key = key;
	fifo.push();
}

void ControlQueue::GenerateTone(int callId,
	float amplitude1, float frequency1,
	float amplitude2, float frequency2,
	float amplitude3, float frequency3,
	float amplitude4, float frequency4)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::GENERATE_TONES;
	cmd->callId = callId;
	STATIC_CHECK(ARRAY_SIZE(cmd->tones) == 4, TonesSizeMismatch);
	cmd->tones[0].amplitude = amplitude1;
	cmd->tones[0].frequency = frequency1;
	cmd->tones[1].amplitude = amplitude2;
	cmd->tones[1].frequency = frequency2;
	cmd->tones[2].amplitude = amplitude3;
	cmd->tones[2].frequency = frequency3;
	cmd->tones[3].amplitude = amplitude4;
	cmd->tones[3].frequency = frequency4;
	fifo.push();
}

void ControlQueue::Hold(int callId, bool state)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::HOLD;
	cmd->callId = callId;
	cmd->bEnabled = state;
	fifo.push();
}

void ControlQueue::Mute(int callId, bool state)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::MUTE;
	cmd->callId = callId;
	cmd->bEnabled = state;
	fifo.push();
}

void ControlQueue::SetMsgLogging(bool enabled, bool onlyFirstLines)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::SET_MSG_LOGGING;
	cmd->bEnabled = enabled;
	cmd->bParam = onlyFirstLines;
	fifo.push();
}

void ControlQueue::SetAubufLogging(bool enabled)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::SET_AUBUF_LOGGING;
	cmd->bEnabled = enabled;
	fifo.push();
}

void ControlQueue::Hangup(int callId, int code, AnsiString reason)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::HANGUP;
	cmd->callId = callId;
	cmd->code = code;
	cmd->reason = reason;
	fifo.push();
}

void ControlQueue::StartRing(AnsiString wavFile)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::START_RING;
	cmd->target = wavFile;
	fifo.push();
}

void ControlQueue::PlayStop(void)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::PLAY_STOP;
	fifo.push();
}

void ControlQueue::StartRing2(AnsiString wavFile)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::START_RING2;
	cmd->target = wavFile;
	fifo.push();
}

void ControlQueue::Record(AnsiString wavFile, unsigned int channels, unsigned int side, unsigned int fileFormat, unsigned int bitrate)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::RECORD;
	cmd->channels = channels;
	cmd->recSide = side;
	cmd->recFileFormat = fileFormat;
	cmd->bitrate = bitrate;
	cmd->target = wavFile;
	fifo.push();
}

void ControlQueue::RecordPause(void)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::RECORD_PAUSE;
	fifo.push();
}

void ControlQueue::PagingTx(AnsiString target, AnsiString pagingTxWaveFile, AnsiString codec, unsigned int ptime)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::PAGING_TX;
	cmd->target = target;
	cmd->pagingTxWaveFile = pagingTxWaveFile;
	cmd->pagingTxCodec = codec;
	cmd->pagingTxPtime = ptime; 
	fifo.push();
}

void ControlQueue::SwitchAudioSource(int callId, AnsiString audioMod, AnsiString audioDev)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::SWITCH_AUDIO_SOURCE;
	cmd->audioMod = audioMod;
	cmd->audioDev = audioDev;
	cmd->callId = callId;
	fifo.push();
}

void ControlQueue::SwitchAudioPlayer(int callId, AnsiString audioMod, AnsiString audioDev)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::SWITCH_AUDIO_PLAYER;
	cmd->audioMod = audioMod;
	cmd->audioDev = audioDev;
	cmd->callId = callId;
	fifo.push();
}

void ControlQueue::UpdateSoftvolTx(unsigned int val)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::UPDATE_SOFTVOL_TX;
	cmd->softvol = val;
	fifo.push();
}

void ControlQueue::UpdateSoftvolRx(unsigned int val)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::UPDATE_SOFTVOL_RX;
	cmd->softvol = val;
	fifo.push();
}

int ControlQueue::SendCustomRequest(int requestId, AnsiString method, AnsiString target, AnsiString extraHeaderLines)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return -1;
	cmd->type = Command::SEND_CUSTOM_REQUEST;
	cmd->requestId = requestId;
	cmd->method = method;
	cmd->target = target;
	if (extraHeaderLines != "")
	{
		if (extraHeaderLines.Length() < 2)
		{
			extraHeaderLines += "\r\n";
		}
		else
		{
			AnsiString end = extraHeaderLines.SubString(extraHeaderLines.Length() - 1, 2);
			if (end != "\r\n")
			{
        		extraHeaderLines += "\r\n";
			}
        }
    }
	cmd->extraHeaderLines = extraHeaderLines;
	fifo.push();
	return 0;
}

int ControlQueue::SendMessage(int requestId, int accountId, AnsiString target, AnsiString text)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return -1;
	cmd->type = Command::SEND_MESSAGE;
	cmd->requestId = requestId;
	cmd->accountId = accountId;
	cmd->target = target;
	cmd->text = text;
	fifo.push();
	return 0;
}

int ControlQueue::ZrtpVerifySas(bool state)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return -1;
	cmd->type = Command::ZRTP_VERIFY_SAS;
	cmd->bParam = state;
	fifo.push();
	return 0;
}




