#pragma once

#include <nctl/UniquePtr.h>
#include <ncine/IAppEventHandler.h>
#include <ncine/IInputEventHandler.h>

#include "Settings.h"
#include "Statistics.h"

namespace ncine {
	class AppConfiguration;
	class Viewport;
}
class MusicManager;
class ShaderEffects;
class SplashScreen;
class Menu;
class Game;

namespace nc = ncine;

/// My nCine event handler
class MyEventHandler :
    public nc::IAppEventHandler,
    public nc::IInputEventHandler
{
  public:
	void onPreInit(nc::AppConfiguration &config) override;
	void onInit() override;
	void onShutdown() override;
	void onFrameStart() override;
	void onDrawViewport(nc::Viewport &viewport) override;
	void onChangeScalingFactor(float factor) override;

	void onKeyReleased(const nc::KeyboardEvent &event) override;
	void onKeyPressed(const nc::KeyboardEvent &event) override;
	void onJoyMappedButtonPressed(const nc::JoyMappedButtonEvent &event) override;
	void onJoyMappedAxisMoved(const nc::JoyMappedAxisEvent &event) override;

	bool onQuitRequest() override;

	MusicManager &musicManager();
	ShaderEffects &shaderEffects();
	void requestMenu();
	void requestGame();

	const Settings &settings() const;
	const Statistics &statistics() const;
	Settings &settingsMut();
	Statistics &statisticsMut();

  private:
	bool requestMenuTransition_ = false;
	bool requestGameTransition_ = false;

	void showMenu();
	void showGame();

	nctl::UniquePtr<MusicManager> musicManager_;
	nctl::UniquePtr<ShaderEffects> shaderEffects_;
	nctl::UniquePtr<SplashScreen> splashScreen_;
	nctl::UniquePtr<Menu> menu_;
	nctl::UniquePtr<Game> game_;

	Settings settings_;
	Statistics statistics_;
};
