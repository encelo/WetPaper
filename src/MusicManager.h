#pragma once

#include <nctl/UniquePtr.h>

namespace ncine {
	class AudioStreamPlayer;
	class AudioFilter;
}
class MyEventHandler;

namespace nc = ncine;

class MusicManager
{
  public:
	enum class States
	{
		STOPPED,
		MAIN_MENU,
		GAME,
		PAUSED_GAME,
	};

	explicit MusicManager(MyEventHandler *eventHandler);
	~MusicManager();

	inline States state() const { return state_; }

	void drawGui();
	void onFrameStart();

	void goToMainMenu();
	void goToGame();
	void togglePause();

	void updateVolume();

  private:
	MyEventHandler *eventHandler_ = nullptr;

	States state_ = States::STOPPED;
	nctl::UniquePtr<nc::AudioStreamPlayer> menuMusicPlayer_;
	nctl::UniquePtr<nc::AudioStreamPlayer> gameMusicPlayer_;
	nctl::UniquePtr<nc::AudioFilter> audioFilter_;

	void setupCrossfade(nc::AudioStreamPlayer *from, nc::AudioStreamPlayer *to);
	void setupFilterEffect(float fromGain, float toGain, float fromGainHF, float toGainHF);
	void clearFilterEffect();
};

