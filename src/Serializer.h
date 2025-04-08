#pragma once

#include "Settings.h"
#include "Statistics.h"

class Serializer
{
  public:
	static bool loadSettings(Settings &settings);
	static bool saveSettings(const Settings &settings);

	static bool loadStatistics(Statistics &statistics);
	static bool saveStatistics(const Statistics &statistics);

  private:
	static void validateSettings(Settings &settings);
};

// Meyers' Singleton
extern Serializer &serializer();
