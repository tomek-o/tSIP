//---------------------------------------------------------------------------
#pragma hdrstop

#include "UaConf.h"
#include "common/MathUtils.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

bool UaConf::AudioAgc::operator==(const UaConf::AudioAgc& right) const {
	return (
		enabled == right.enabled &&
		target == right.target &&
		MathUtils::AlmostEqual(maxGain, right.maxGain) &&
		MathUtils::AlmostEqual(attackRate, right.attackRate) &&
		MathUtils::AlmostEqual(releaseRate, right.releaseRate)
	);
}

bool UaConf::AudioPortaudio::operator==(const UaConf::AudioPortaudio& right) const {
	return (
		MathUtils::AlmostEqual(inSuggestedLatency, right.inSuggestedLatency) &&
		MathUtils::AlmostEqual(outSuggestedLatency, right.outSuggestedLatency) 
	);
}
