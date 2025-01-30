#pragma once

#include <nctl/UniquePtr.h>
#include <ncine/IAppEventHandler.h>
#include <ncine/IInputEventHandler.h>

namespace ncine {
	class AppConfiguration;
}
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
	void onKeyReleased(const nc::KeyboardEvent &event) override;

	void requestMenu();
	void requestGame();

  private:
	bool requestMenuTransition_ = false;
	bool requestGameTransition_ = false;

	void showMenu();
	void showGame();

	nctl::UniquePtr<Menu> menu_;
	nctl::UniquePtr<Game> game_;
};
