#pragma once

#include <ncine/Rect.h>

struct Settings
{
	float volume = 1.0f;
	float sfxVolume = 1.0f;
	float musicVolume = 1.0f;
	unsigned int numPlayers = 1;
	unsigned int matchTime = 60;
	bool withShaders = true;

	ncine::Recti windowState;
};
