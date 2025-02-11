#pragma once

#include <nctl/StaticArray.h>
#include <ncine/TimeStamp.h>
#include "LogicNode.h"
#include "../Config.h"

namespace ncine {
	class Sprite;
	class AudioBufferPlayer;
	class Font;
	class TextNode;
}

class Player;
class Body;
class Bubble;
class MyEventHandler;

namespace nc = ncine;

class Game : public LogicNode
{
  public:
	Game(SceneNode *parent, nctl::String name, MyEventHandler *eventHandler);
	~Game() override;

	void onTick(float deltaTime) override;
	void drawGui();

	static void playSound();

  private:
	MyEventHandler *eventHandler_;

	nctl::UniquePtr<nc::Sprite> background_;
	nctl::UniquePtr<Player> playerA_;
	nctl::UniquePtr<Player> playerB_;

	nctl::UniquePtr<nc::Sprite> redBar_;
	nctl::UniquePtr<nc::Sprite> redBarFill_;
	nctl::UniquePtr<nc::Sprite> blueBar_;
	nctl::UniquePtr<nc::Sprite> blueBarFill_;

	nctl::UniquePtr<Body> obstacle1_;
	nctl::UniquePtr<Body> obstacle2_;
	nctl::UniquePtr<Body> obstacle3_;

	nctl::UniquePtr<nc::Sprite> obstacle1Gfx_;
#if NCPROJECT_DEBUG
	nctl::UniquePtr<nc::Sprite> obstacle2Gfx_;
	nctl::UniquePtr<nc::Sprite> obstacle3Gfx_;
#endif

	nctl::Array<nctl::UniquePtr<Bubble>> bubbles_;
	nctl::StaticArray<nctl::UniquePtr<nc::AudioBufferPlayer>, Cfg::Sounds::NumBubblePopPlayers> poppingPlayers_;

	nctl::UniquePtr<nc::Font> font_;
	nctl::UniquePtr<nc::TextNode> timeText_;
	nctl::UniquePtr<nc::TextNode> pointsAText_;
	nctl::UniquePtr<nc::TextNode> pointsBText_;

	nc::TimeStamp matchTimer_;

	void loadScene();
	void spawnBubbles();
	void spawnBubble();
	void destroyDeadBubbles();
	void playPoppingSound();
};
