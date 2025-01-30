#pragma once

#include "LogicNode.h"

#include <ncine/TimeStamp.h>

namespace ncine {
	class Sprite;
}
class MyEventHandler;

namespace nc = ncine;

class Menu : public LogicNode
{
  public:
	Menu(SceneNode *parent, nctl::String name, MyEventHandler *eventHandler);

	void onTick(float deltaTime) override;
	void drawGui();

  private:
	MyEventHandler *eventHandler_;
	nctl::UniquePtr<nc::Sprite> background_;

	nctl::UniquePtr<nc::Sprite> startBtnOff_;
	nctl::UniquePtr<nc::Sprite> startBtnOn_;

	nc::TimeStamp showMenuTimer_;
};
