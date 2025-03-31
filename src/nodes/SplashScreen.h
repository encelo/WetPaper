#pragma once

#include "LogicNode.h"
#include <ncine/TimeStamp.h>

namespace ncine {
	class Sprite;
	class Font;
	class TextNode;
}
class MyEventHandler;

namespace nc = ncine;

class SplashScreen : public LogicNode
{
  public:
	SplashScreen(SceneNode *parent, nctl::String name, MyEventHandler *eventHandler);

	void onTick(float deltaTime) override;
	void drawGui();

  private:
	enum class AnimState
	{
		FADE_IN,
		SUSTAIN,
		FADE_OUT,
		FAST_FADE_OUT,
		END,
	};

	MyEventHandler *eventHandler_;
	AnimState state_;

	nctl::UniquePtr<nc::Sprite> background_;
	nctl::UniquePtr<nc::Sprite> nCineLogo_;

	nctl::UniquePtr<nc::Font> smallFont_;
	nctl::UniquePtr<nc::TextNode> smallText_;

	nc::TimeStamp stateTimer_;
};
