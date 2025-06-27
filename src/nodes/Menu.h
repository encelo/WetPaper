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

	void onFrameStart();
	void onQuitRequest();

  private:
	MyEventHandler *eventHandler_;

	nctl::UniquePtr<nc::Sprite> background_;
	nctl::UniquePtr<nc::Sprite> darkForeground_;

	nctl::UniquePtr<nc::Font> titleFont_;
	nctl::UniquePtr<nc::TextNode> gameTitleText_;

	nctl::UniquePtr<nc::Font> smallFont_;
	nctl::UniquePtr<nc::TextNode> versionText_;
	nctl::UniquePtr<nc::TextNode> statusText_;

	nctl::UniquePtr<nc::SceneNode> backgroundRoot_;
	nctl::UniquePtr<nc::SceneNode> sceneRoot_;
	nctl::UniquePtr<nc::SceneNode> foregroundRoot_;

	nctl::UniquePtr<MenuPage> menuPage_;
	static MenuPage::PageConfig mainPage_;
	static MenuPage::PageConfig quitConfirmationPage_;
	static MenuPage::PageConfig settingsPage_;
	static MenuPage::PageConfig statisticsPage_;
	static MenuPage::PageConfig resetStatisticsConfirmationPage_;
	static MenuPage::PageConfig controlsPage_;
	static MenuPage::PageConfig keyboardControlsPageP1_;
	static MenuPage::PageConfig keyboardControlsPageP2_;
	static MenuPage::PageConfig joystickControlsPageP1_;
	static MenuPage::PageConfig joystickControlsPageP2_;
	static MenuPage::PageConfig creditsPage_;

	static const unsigned int NumBubbles = 20;
	nctl::StaticArray<nctl::UniquePtr<nc::Sprite>, NumBubbles> bubbles_;
	nctl::StaticArray<nc::Vector2f, NumBubbles> bubbleDirections_;
	nctl::StaticArray<float, NumBubbles> bubbleSpeeds_;
	nctl::StaticArray<unsigned int, NumBubbles> bubbleVariants_;

	bool requestGame_ = false;
	bool requestShaderEffectsChange_ = false;
	bool shaderEffectsEnabled_ = false;
	void enableShaderEffects(bool enabled);

	void setupPages();

	static void goToMainPage();
	static void goToSettingsPage();
	static void goToControlsPage();
	static void goToStatisticsPage();
	static void simpleSelectFunc(MenuPage::EntryEvent &event);
	static void statisticsTextFunc(MenuPage::EntryEvent &event);
	static void settingsPlayersFunc(MenuPage::EntryEvent &event);
	static void settingsMatchTimeFunc(MenuPage::EntryEvent &event);
	static void settingsVolumeFunc(MenuPage::EntryEvent &event);
	static void settingsShadersFunc(MenuPage::EntryEvent &event);
	static void keyboardControlsFunc(MenuPage::EntryEvent &event);
	static void joystickControlsFunc(MenuPage::EntryEvent &event);
};
