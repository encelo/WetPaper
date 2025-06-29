#pragma once

#include <nctl/StaticArray.h>
#include <ncine/TimeStamp.h>
#include "LogicNode.h"
#include "MenuPage.h"
#include "../Config.h"
#include "../Statistics.h"

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

	void onFrameStart();
	void onQuitRequest();

	static void playSound();
	static void killBubble(Bubble *bubblePtr);
	static void incrementDroppedBubble();

  private:
	MyEventHandler *eventHandler_;

	nctl::UniquePtr<nc::Sprite> background_;
	nctl::UniquePtr<nc::Sprite> darkForeground_;
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

	nctl::StaticArray<nctl::UniquePtr<Bubble>, Cfg::Game::BubblePoolSize> bubblePool_;
	nctl::StaticArray<nctl::UniquePtr<Bubble>, Cfg::Game::BubblePoolSize> bubbles_;
	nctl::StaticArray<Bubble*, Cfg::Game::BubblePoolSize> deadBubbles_;
	nctl::StaticArray<nctl::UniquePtr<nc::AudioBufferPlayer>, Cfg::Sounds::NumBubblePopPlayers> poppingPlayers_;

	nctl::UniquePtr<nc::Font> font_;
	nctl::UniquePtr<nc::TextNode> timeText_;
	nctl::UniquePtr<nc::TextNode> pointsAText_;
	nctl::UniquePtr<nc::TextNode> pointsBText_;

	nctl::UniquePtr<nc::SceneNode> backgroundRoot_;
	nctl::UniquePtr<nc::SceneNode> sceneRoot_;
	nctl::UniquePtr<nc::SceneNode> foregroundRoot_;

	nc::TimeStamp matchTimer_;
	nc::TimeStamp pauseTime_;
	bool paused_;
	bool matchEnded_;
	Statistics statistics_;

	nctl::UniquePtr<MenuPage> menuPage_;
	static MenuPage::PageConfig pausePage_;
	static MenuPage::PageConfig endMatchPage_;
	static MenuPage::PageConfig quitConfirmationPausePage_;
	static MenuPage::PageConfig quitConfirmationEndMatchPage_;

	void loadScene();
	void spawnBubbles();
	void spawnBubble();
	void destroyDeadBubbles();
	void playPoppingSound();
	void setSfxVolume();

	void togglePause();
	void endMatch();
	void saveStatistics();

	bool requestMenu_ = false;
	bool requestShaderEffectsChange_ = false;
	bool requestPauseShaderEffectsChange_ = false;
	bool shaderEffectsEnabled_ = false;
	void enableShaderEffects(bool enabled);

	void setupPages();
	static void goToPausePage();
	static void goToEndMatchPage();
	static void goToMainMenu();
	static void resumeGame();
	static void simpleSelectFunc(MenuPage::EntryEvent &event);
	static void statisticsTextFunc(MenuPage::EntryEvent &event);
};
