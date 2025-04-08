#pragma once

#include <ncine/Rect.h>

struct Settings
{
	float volume = 1.0f;
	unsigned int numPlayers = 1;
	unsigned int matchTime = 60;

	ncine::Recti windowState;
};
