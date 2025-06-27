#include "Serializer.h"
#include "Config.h"

#ifndef __EMSCRIPTEN__
#include <toml.hpp>
#include <nctl/String.h>
#include <nctl/UniquePtr.h>
#include <ncine/FileSystem.h>
#include <ncine/IFile.h>
#include <ncine/AppConfiguration.h>

namespace {
	char const *const SettingsVolumeString = "volume";
	char const *const SettingsNumPlayersString = "numPlayers";
	char const *const SettingsMatchTimeString = "matchTime";
	char const *const SettingsWithShadersString = "withShaders";
	char const *const SettingsWindowStateString = "windowState";

	char const *const StatisticsPlayTimeString = "playTime";
	char const *const StatisticsNumMatchesString = "numMatches";
	char const *const StatisticsNumDroppedBubblesString = "numDroppedBubbles";
	char const *const StatisticsNumCatchedBubblesString = "numCatchedBubbles";
	char const *const StatisticsNumJumpsString = "numJumps";
	char const *const StatisticsNumDoubleJumpsString = "numDoubleJumps";
	char const *const StatisticsNumDashesJumpsString = "numDashes";

	char const *const StatisticsPlayerAString = "PlayerA";
	char const *const StatisticsPlayerBString = "PlayerB";

	toml::value serializeRect(const nc::Recti &rect)
	{
		return toml::value(toml::table {
			{ "x", rect.x },
			{ "y", rect.y },
			{ "width", rect.w },
			{ "height", rect.h }
		});
	}

	nc::Recti deserializeRect(const toml::value &v)
	{
		return nc::Recti{
			toml::find_or<int>(v, "x", 0),
			toml::find_or<int>(v, "y", 0),
			toml::find_or<int>(v, "width", Cfg::Game::Resolution.x),
			toml::find_or<int>(v, "height", Cfg::Game::Resolution.y)
		};
	}
}
#endif

Serializer &serializer()
{
	static Serializer instance;
	return instance;
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

#ifndef __EMSCRIPTEN__
bool Serializer::loadSettings(Settings &settings)
{
	// Initialize the window state rectangle with sane values
	settings.windowState.set(nc::AppConfiguration::WindowPositionIgnore,
	                         nc::AppConfiguration::WindowPositionIgnore, Cfg::Game::Resolution);

	const nctl::String settingsFilepath = nc::fs::joinPath(nc::fs::savePath(), Cfg::SettingsFilename);

	nctl::UniquePtr<nc::IFile> file = nc::IFile::createFileHandle(settingsFilepath.data());
	file->open(nc::IFile::OpenMode::READ | nc::IFile::OpenMode::BINARY);
	if (file->isOpened() == false)
	{
		LOGW_X("Cannot open settings file for reading: %s", settingsFilepath.data());
		return false;
	}

	const unsigned long fileSize = file->size();
	nctl::UniquePtr<char[]> fileBuffer = nctl::makeUnique<char[]>(fileSize + 1);
	file->read(fileBuffer.get(), fileSize);
	fileBuffer[fileSize] = '\0';
	file->close();

	std::string fileString(fileBuffer.get(), fileSize);
	std::istringstream stream(fileString);
	const auto result = toml::try_parse(stream, Cfg::SettingsFilename, toml::spec::v(1, 1, 0));

	if (result.is_ok())
	{
		const toml::value &data = result.unwrap();

		const Settings defaultSettings;
		settings.volume = toml::find_or<float>(data, SettingsVolumeString, defaultSettings.volume);
		settings.numPlayers = toml::find_or<unsigned int>(data, SettingsNumPlayersString, defaultSettings.numPlayers);
		settings.matchTime = toml::find_or<unsigned int>(data, SettingsMatchTimeString, defaultSettings.matchTime);
		settings.withShaders = toml::find_or<bool>(data, SettingsWithShadersString, defaultSettings.withShaders);

		if (data.contains(SettingsWindowStateString) && data.at(SettingsWindowStateString).is_table())
			settings.windowState = deserializeRect(toml::find(data, SettingsWindowStateString));

		validateSettings(settings);

		return true;
	}
	else
	{
		const auto &err = result.unwrap_err();
		LOGW_X("TOML parse error when loading settings file: \"%s\"", err.at(0).title().data());
		return false;
	}
}

bool Serializer::saveSettings(const Settings &settings)
{
	toml::value data = toml::value(toml::table{
		{ SettingsVolumeString, settings.volume },
		{ SettingsNumPlayersString, settings.numPlayers },
		{ SettingsMatchTimeString, settings.matchTime },
		{ SettingsWithShadersString, settings.withShaders }
	});

	data[SettingsWindowStateString] = serializeRect(settings.windowState);

	const nctl::String settingsFilepath = nc::fs::joinPath(nc::fs::savePath(), Cfg::SettingsFilename);

	const nctl::String settingsDirpath = nc::fs::dirName(settingsFilepath.data());
	if (nc::fs::isDirectory(settingsDirpath.data()) == false)
		nc::fs::createDir(settingsDirpath.data());

	nctl::UniquePtr<nc::IFile> file = nc::IFile::createFileHandle(settingsFilepath.data());
	file->open(nc::IFile::OpenMode::WRITE | nc::IFile::OpenMode::BINARY);
	if (file->isOpened() == false)
	{
		LOGW_X("Cannot open settings file for writing: %s", settingsFilepath.data());
		return false;
	}

	const std::string serializedString = toml::format(data);
	file->write(serializedString.data(), serializedString.length());
	file->close();
	return true;
}

bool Serializer::loadStatistics(Statistics &statistics)
{
	const nctl::String statisticsFilepath = nc::fs::joinPath(nc::fs::savePath(), Cfg::StatisticsFilename);

	nctl::UniquePtr<nc::IFile> file = nc::IFile::createFileHandle(statisticsFilepath.data());
	file->open(nc::IFile::OpenMode::READ | nc::IFile::OpenMode::BINARY);
	if (file->isOpened() == false)
	{
		LOGW_X("Cannot open statistics file for reading: %s", statisticsFilepath.data());
		return false;
	}

	const unsigned long fileSize = file->size();
	nctl::UniquePtr<char[]> fileBuffer = nctl::makeUnique<char[]>(fileSize + 1);
	file->read(fileBuffer.get(), fileSize);
	fileBuffer[fileSize] = '\0';
	file->close();

	std::string fileString(fileBuffer.get(), fileSize);
	std::istringstream stream(fileString);
	const auto result = toml::try_parse(stream, Cfg::SettingsFilename, toml::spec::v(1, 1, 0));

	if (result.is_ok())
	{
		const toml::value &data = result.unwrap();

		statistics.playTime = toml::find_or<unsigned int>(data, StatisticsPlayTimeString, 0);
		statistics.numMatches = toml::find_or<unsigned int>(data, StatisticsNumMatchesString, 0);
		statistics.numDroppedBubles = toml::find_or<unsigned int>(data, StatisticsNumDroppedBubblesString, 0);

		// Player 1 statistics
		if (data.contains(StatisticsPlayerAString) && data.at(StatisticsPlayerAString).is_table())
		{
			statistics.playerStats[0].numCatchedBubbles = toml::find_or<unsigned int>(data, StatisticsPlayerAString, StatisticsNumCatchedBubblesString, 0);
			statistics.playerStats[0].numJumps = toml::find_or<unsigned int>(data, StatisticsPlayerAString, StatisticsNumJumpsString, 0);
			statistics.playerStats[0].numDoubleJumps = toml::find_or<unsigned int>(data, StatisticsPlayerAString, StatisticsNumDoubleJumpsString, 0);
			statistics.playerStats[0].numDashes = toml::find_or<unsigned int>(data, StatisticsPlayerAString, StatisticsNumDashesJumpsString, 0);
		}
		// Player 2 statistics
		if (data.contains(StatisticsPlayerBString) && data.at(StatisticsPlayerBString).is_table())
		{
			statistics.playerStats[1].numCatchedBubbles = toml::find_or<unsigned int>(data, StatisticsPlayerBString, StatisticsNumCatchedBubblesString, 0);
			statistics.playerStats[1].numJumps = toml::find_or<unsigned int>(data, StatisticsPlayerBString, StatisticsNumJumpsString, 0);
			statistics.playerStats[1].numDoubleJumps = toml::find_or<unsigned int>(data, StatisticsPlayerBString, StatisticsNumDoubleJumpsString, 0);
			statistics.playerStats[1].numDashes = toml::find_or<unsigned int>(data, StatisticsPlayerBString, StatisticsNumDashesJumpsString, 0);
		}

		return true;
	}
	else
	{
		const auto &err = result.unwrap_err();
		LOGW_X("TOML parse error when loading statistics file: \"%s\"", err.at(0).title().data());
		return false;
	}
}

bool Serializer::saveStatistics(const Statistics &statistics)
{
	toml::value data = toml::value(toml::table{
		{ StatisticsPlayTimeString, statistics.playTime },
		{ StatisticsNumMatchesString, statistics.numMatches },
		{ StatisticsNumDroppedBubblesString, statistics.numDroppedBubles }
	});

	// Player 1 statistics
	data[StatisticsPlayerAString][StatisticsNumCatchedBubblesString] = statistics.playerStats[0].numCatchedBubbles;
	data[StatisticsPlayerAString][StatisticsNumJumpsString] = statistics.playerStats[0].numJumps;
	data[StatisticsPlayerAString][StatisticsNumDoubleJumpsString] = statistics.playerStats[0].numDoubleJumps;
	data[StatisticsPlayerAString][StatisticsNumDashesJumpsString] = statistics.playerStats[0].numDashes;
	// Player 2 statistics
	data[StatisticsPlayerBString][StatisticsNumCatchedBubblesString] = statistics.playerStats[1].numCatchedBubbles;
	data[StatisticsPlayerBString][StatisticsNumJumpsString] = statistics.playerStats[1].numJumps;
	data[StatisticsPlayerBString][StatisticsNumDoubleJumpsString] = statistics.playerStats[1].numDoubleJumps;
	data[StatisticsPlayerBString][StatisticsNumDashesJumpsString] = statistics.playerStats[1].numDashes;

	const nctl::String statisticsFilepath = nc::fs::joinPath(nc::fs::savePath(), Cfg::StatisticsFilename);

	const nctl::String statisticsDirpath = nc::fs::dirName(statisticsFilepath.data());
	if (nc::fs::isDirectory(statisticsDirpath.data()) == false)
		nc::fs::createDir(statisticsDirpath.data());

	nctl::UniquePtr<nc::IFile> file = nc::IFile::createFileHandle(statisticsFilepath.data());
	file->open(nc::IFile::OpenMode::WRITE | nc::IFile::OpenMode::BINARY);
	if (file->isOpened() == false)
	{
		LOGW_X("Cannot open statistics file for writing: %s", statisticsFilepath.data());
		return false;
	}

	const std::string serializedString = toml::format(data);
	file->write(serializedString.data(), serializedString.length());
	file->close();
	return true;
}
#else
bool Serializer::loadSettings(Settings &settings)
{
	return false;
}

bool Serializer::saveSettings(const Settings &settings)
{
	return false;
}

bool Serializer::loadStatistics(Statistics &statistics)
{
	return false;
}

bool Serializer::saveStatistics(const Statistics &statistics)
{
	return false;
}
#endif

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void Serializer::validateSettings(Settings &settings)
{
	if (settings.volume > Cfg::Settings::VolumeGainMax)
		settings.volume = Cfg::Settings::VolumeGainMax;
	else if (settings.volume < Cfg::Settings::VolumeGainMin)
		settings.volume = Cfg::Settings::VolumeGainMin;

	if (settings.numPlayers < 1)
		settings.numPlayers = 1;
	else if (settings.numPlayers > 2)
		settings.numPlayers = 2;

	unsigned int targetMatchTime = Cfg::Settings::MatchTimeMax;
	while (targetMatchTime >= Cfg::Settings::MatchTimeMin)
	{
		if (settings.matchTime >= targetMatchTime)
		{
			settings.matchTime = targetMatchTime;
			targetMatchTime = 0; // to exit the loop
		}
		else
			targetMatchTime -= Cfg::Settings::MatchTimeStep;
	}
}
