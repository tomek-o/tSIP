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
