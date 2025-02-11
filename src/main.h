#pragma once

#include <ncine/IAppEventHandler.h>
#include <ncine/IInputEventHandler.h>
#include <ncine/SceneNode.h>

#include "nodes/Game.h"
#include "nodes/Menu.h"

namespace ncine {

class AppConfiguration;

}

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
	void onChangeScalingFactor(float factor) override;

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
