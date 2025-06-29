#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "MusicManager.h"
#include "main.h"
#include "Config.h"
#include "Settings.h"

#include <ncine/Application.h>
#include <ncine/FileSystem.h>
#include <ncine/AudioStreamPlayer.h>

#if NCINE_WITH_OPENAL_EXT
	#include <ncine/AudioFilter.h>
#endif

namespace {

#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
nctl::String auxString(128);

const char *stateToString(MusicManager::States state)
{
	switch(state)
	{
		default:
		case MusicManager::States::STOPPED: return "Stopped";
		case MusicManager::States::MAIN_MENU: return "Main menu";
		case MusicManager::States::GAME: return "Game";
		case MusicManager::States::PAUSED_GAME: return "Paused game";
	}
}
#endif

float targetVolume = 1.0f;
nc::AudioStreamPlayer *crossfadeFrom = nullptr;
nc::AudioStreamPlayer *crossfadeTo = nullptr;
float crossfadeParam = 1.0f;

#if NCINE_WITH_OPENAL_EXT
nc::AudioFilter::Properties filterProperties = nc::AudioFilter::Properties();
float startGain = Cfg::Music::MinGain;
float endGain = 1.0f;
float startGainHF = Cfg::Music::MinGainHF;
float endGainHF = 1.0f;
float effectParam = 1.0f;
#endif

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

MusicManager::MusicManager(MyEventHandler *eventHandler)
    : eventHandler_(eventHandler)
{
	const nctl::String menuAbsolutePath = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Music::MenuMusic);
	menuMusicPlayer_ = nctl::makeUnique<nc::AudioStreamPlayer>(menuAbsolutePath.data());

	const nctl::String gameAbsolutePath = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Music::GameMusic);
	gameMusicPlayer_ = nctl::makeUnique<nc::AudioStreamPlayer>(gameAbsolutePath.data());

	updateVolume();

#if NCINE_WITH_OPENAL_EXT
	nc::IAudioDevice &device = nc::theServiceLocator().audioDevice();
	if (device.hasExtension(nc::IAudioDevice::ALExtensions::EXT_EFX))
	{
		filterProperties.type = nc::AudioFilter::Type::LOWPASS;
		audioFilter_ = nctl::makeUnique<nc::AudioFilter>();
	}
#endif
}

/** \note This is needed here to destruct the stream players at the right time. */
MusicManager::~MusicManager()
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void MusicManager::drawGui()
{
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	if (ImGui::TreeNode("Music"))
	{
		ImGui::Text("State: %s", stateToString(state_));
		if (menuMusicPlayer_->isPlaying())
		{
			const unsigned long sampleOffsetInStream = menuMusicPlayer_->sampleOffsetInStream();
			const unsigned long numSamples = menuMusicPlayer_->numSamples();
			const float streamFraction = sampleOffsetInStream / static_cast<float>(numSamples);
			auxString.format("Stream samples (Menu): %ld / %ld", sampleOffsetInStream, numSamples);
			ImGui::ProgressBar(streamFraction, ImVec2(0.0f, 0.0f), auxString.data());
		}
		if (gameMusicPlayer_->isPlaying())
		{
			const unsigned long sampleOffsetInStream = gameMusicPlayer_->sampleOffsetInStream();
			const unsigned long numSamples = gameMusicPlayer_->numSamples();
			const float streamFraction = sampleOffsetInStream / static_cast<float>(numSamples);
			auxString.format("Stream samples (Game): %ld / %ld", sampleOffsetInStream, numSamples);
			ImGui::ProgressBar(streamFraction, ImVec2(0.0f, 0.0f), auxString.data());
		}

		ImGui::Text("Menu music: %.2f gain", menuMusicPlayer_->gain());
		ImGui::Text("Game music: %.2f gain", gameMusicPlayer_->gain());
		ImGui::Text("Crossfade parameter: %.2f", crossfadeParam);
		if (crossfadeParam < 1.0f)
		{
			if (crossfadeFrom)
			{
				auxString.format("From: %.2f -> %.2f gain", crossfadeFrom->gain(), targetVolume);
				ImGui::ProgressBar(1.0f - crossfadeParam, ImVec2(0.0f, 0.0f), auxString.data());
			}

			if (crossfadeTo)
			{
				auxString.format("To: %.2f -> %.2f gain", crossfadeTo->gain(),targetVolume);
				ImGui::ProgressBar(crossfadeParam, ImVec2(0.0f, 0.0f), auxString.data());
			}
		}

	#if NCINE_WITH_OPENAL_EXT
		ImGui::Text("Filter Gain: %.2f", filterProperties.gain);
		ImGui::Text("Filter GainHF: %.2f", filterProperties.gainHF);
		ImGui::Text("Effect parameter: %.2f", effectParam);
		if (effectParam < 1.0f)
		{
			auxString.format("Gain: %.2f -> %.2f, HF: %.2f -> %.2f",
			                 filterProperties.gain, endGain, filterProperties.gainHF, endGainHF);
			ImGui::ProgressBar(effectParam, ImVec2(0.0f, 0.0f), auxString.data());
		}
	#endif
		ImGui::TreePop();
	}
#endif
}

void MusicManager::onFrameStart()
{
	const float frameTime = nc::theApplication().frameTime();

	if (crossfadeParam < 1.0f)
	{
		if (crossfadeFrom && crossfadeFrom->isPlaying())
			crossfadeFrom->setGain((1.0f - crossfadeParam) * targetVolume);
		if (crossfadeTo)
			crossfadeTo->setGain(crossfadeParam * targetVolume);
		crossfadeParam += frameTime / Cfg::Music::CrossfadeTime;
	}
	else
	{
		if (crossfadeFrom)
		{
			crossfadeFrom->setGain(0.0f);
			crossfadeFrom->stop();
		}
		if (crossfadeTo)
			crossfadeTo->setGain(targetVolume);

		crossfadeFrom = nullptr;
		crossfadeTo = nullptr;
		crossfadeParam = 1.0f;
	}

#if NCINE_WITH_OPENAL_EXT
	nc::IAudioDevice &device = nc::theServiceLocator().audioDevice();
	if (device.hasExtension(nc::IAudioDevice::ALExtensions::EXT_EFX))
	{
		if (effectParam < 1.0f)
		{
			filterProperties.gain = startGain + (endGain - startGain) * effectParam;
			filterProperties.gainHF = startGainHF + (endGainHF - startGainHF) * effectParam;
			audioFilter_->applyProperties(filterProperties);
			// The filter needs to be set again on properties change
			gameMusicPlayer_->setDirectFilter(audioFilter_.get());
			effectParam += frameTime / Cfg::Music::EffectTime;
		}
		else
		{
			filterProperties.gainHF = endGainHF;
			effectParam = 1.0f;
			if (state_ == States::GAME)
				gameMusicPlayer_->setDirectFilter(nullptr);
		}
	}
#endif
}

void MusicManager::goToMainMenu()
{
	if (state_ == States::MAIN_MENU)
		return;

	// Starting the application now or going back from game?
	if (state_ == States::STOPPED)
		setupCrossfade(nullptr, menuMusicPlayer_.get());
	else
		setupCrossfade(gameMusicPlayer_.get(), menuMusicPlayer_.get());

	clearFilterEffect();

	state_ = States::MAIN_MENU;
}

void MusicManager::goToGame()
{
	if (state_ == States::GAME)
		return;

	setupCrossfade(menuMusicPlayer_.get(), gameMusicPlayer_.get());

	state_ = States::GAME;
}

void MusicManager::togglePause()
{
	if (state_ == States::GAME)
	{
		setupFilterEffect(1.0f, Cfg::Music::MinGain, 1.0f, Cfg::Music::MinGainHF);
		state_ = States::PAUSED_GAME;
	}
	else if (state_ == States::PAUSED_GAME)
	{
		setupFilterEffect(Cfg::Music::MinGain, 1.0f, Cfg::Music::MinGainHF, 1.0f);
		state_ = States::GAME;
	}
}

void MusicManager::updateVolume()
{
	const Settings &settings = eventHandler_->settings();
	targetVolume = settings.musicVolume * settings.volume;
	menuMusicPlayer_->setGain(targetVolume);
	gameMusicPlayer_->setGain(targetVolume);
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void MusicManager::setupCrossfade(nc::AudioStreamPlayer *from, nc::AudioStreamPlayer *to)
{
	const Settings &settings = eventHandler_->settings();
	targetVolume = settings.musicVolume * settings.volume;

	crossfadeFrom = from;
	crossfadeTo = to;
	crossfadeParam = 0.0f;

	ASSERT(crossfadeFrom == nullptr || crossfadeFrom->isPlaying());
	ASSERT(crossfadeTo != nullptr && crossfadeTo->isPlaying() == false);
	crossfadeTo->play();
}

void MusicManager::setupFilterEffect(float fromGain, float toGain, float fromGainHF, float toGainHF)
{
#if NCINE_WITH_OPENAL_EXT
	nc::IAudioDevice &device = nc::theServiceLocator().audioDevice();
	if (device.hasExtension(nc::IAudioDevice::ALExtensions::EXT_EFX))
	{
		startGain = fromGain;
		endGain = toGain;
		startGainHF = fromGainHF;
		endGainHF = toGainHF;
		effectParam = 0.0f;
		filterProperties.gain = startGain;
		filterProperties.gainHF = startGainHF;
	}
#endif
}

void MusicManager::clearFilterEffect()
{
#if NCINE_WITH_OPENAL_EXT
	nc::IAudioDevice &device = nc::theServiceLocator().audioDevice();
	if (device.hasExtension(nc::IAudioDevice::ALExtensions::EXT_EFX))
	{
		startGain = Cfg::Music::MinGain;
		endGain = 1.0f;
		startGainHF = Cfg::Music::MinGainHF;
		endGainHF = 1.0f;
		effectParam = 1.0f;
		filterProperties.gain = endGain;
		filterProperties.gainHF = endGainHF;
		audioFilter_->applyProperties(filterProperties);
		gameMusicPlayer_->setDirectFilter(nullptr);
	}
#endif
}

