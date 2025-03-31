#pragma once

#include "LogicNode.h"
#include "MenuPage.h"
#include <nctl/StaticArray.h>
#include <ncine/TimeStamp.h>

namespace ncine {
	class KeyboardEvent;
	class JoyMappedButtonEvent;
	class JoyMappedAxisEvent;

	class Sprite;
	class Font;
	class TextNode;
}
class MyEventHandler;

namespace nc = ncine;

class Menu : public LogicNode
{
  public:
	Menu(SceneNode *parent, nctl::String name, MyEventHandler *eventHandler);
	~Menu() override;

	void onTick(float deltaTime) override;
	void drawGui();

	void onKeyPressed(const nc::KeyboardEvent &event);
	void onJoyMappedButtonPressed(const nc::JoyMappedButtonEvent &event);
	void onJoyMappedAxisMoved(const nc::JoyMappedAxisEvent &event);

  private:
	MyEventHandler *eventHandler_;
	nctl::UniquePtr<nc::Sprite> background_;
	nctl::UniquePtr<nc::Sprite> darkForeground_;

	nctl::UniquePtr<nc::Font> titleFont_;
	nctl::UniquePtr<nc::TextNode> gameTitleText_;

	nctl::UniquePtr<nc::Font> smallFont_;
	nctl::UniquePtr<nc::TextNode> versionText_;
	nctl::UniquePtr<nc::TextNode> statusText_;

	nctl::UniquePtr<MenuPage> menuPage_;
	static MenuPage::PageConfig mainPage_;
	static MenuPage::PageConfig settingsPage_;
	static MenuPage::PageConfig keyboardControlsPageP1_;
	static MenuPage::PageConfig keyboardControlsPageP2_;
	static MenuPage::PageConfig joystickControlsPageP1_;
	static MenuPage::PageConfig joystickControlsPageP2_;

	static const unsigned int NumBubbles = 20;
	nctl::StaticArray<nctl::UniquePtr<nc::Sprite>, NumBubbles> bubbles_;
	nctl::StaticArray<nc::Vector2f, NumBubbles> bubbleDirections_;
	nctl::StaticArray<float, NumBubbles> bubbleSpeeds_;

	void setupPages();

	static void goToMainPage();
	static void goToSettingsPage();
	static void simpleSelectFunc(MenuPage::EntryEvent &event);
	static void settingsVolumeFunc(MenuPage::EntryEvent &event);
	static void keyboardControlsFunc(MenuPage::EntryEvent &event);
	static void joystickControlsFunc(MenuPage::EntryEvent &event);

#if defined(NCPROJECT_DEBUG)
	static bool selectEventReplyFunc(MenuPage::EventType type);
	static bool selectTextEventReplyFunc(MenuPage::EventType type);
	static bool leftRightTextEventReplyFunc(MenuPage::EventType type);
#endif
};
