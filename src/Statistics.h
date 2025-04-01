#pragma once

struct PlayerStatistics
{
	unsigned int numCatchedBubbles = 0;

	unsigned int numJumps = 0;
	unsigned int numDoubleJumps = 0;
	unsigned int numDashes = 0;
};

struct Statistics
{
	unsigned int playTime = 0;
	unsigned int numMatches = 0;

	unsigned int numDroppedBubles = 0;

	PlayerStatistics playerStats[2];
};
