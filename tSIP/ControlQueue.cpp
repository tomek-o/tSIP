//---------------------------------------------------------------------------


#pragma hdrstop

#include "ControlQueue.h"
#include "common/Mutex.h"
#include "common/ScopedLock.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#include "common/fifo.h"
#include "Command.h"

namespace
{
	Fifo<Command, 512> fifo;
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

void ControlQueue::Call(int accountId, AnsiString target, AnsiString extraHeaderLines)
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
	fifo.push();
}

void ControlQueue::Answer(int callId, AnsiString audioRxMod, AnsiString audioRxDev)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::ANSWER;
	cmd->audioMod = audioRxMod;
	cmd->audioDev = audioRxDev;
	cmd->callId = callId;
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

void ControlQueue::SetMsgLogging(bool enabled)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::SET_MSG_LOGGING;
	cmd->bEnabled = enabled;
	fifo.push();
}

void ControlQueue::Hangup(int callId, int code)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::HANGUP;
	cmd->callId = callId;
	cmd->code = code;
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

void ControlQueue::StopRing(void)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::STOP_RING;
	fifo.push();
}

void ControlQueue::Record(AnsiString wavFile, unsigned int channels)
{
	ScopedLock<Mutex> lock(mutex);
	Command *cmd = fifo.getWriteable();
	if (!cmd)
		return;
	cmd->type = Command::RECORD;
	cmd->channels = channels;
	cmd->target = wavFile;
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

