#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "version.h"

#include "Menu.h"
#include "../main.h"
#include "../Config.h"
#include "../ResourceManager.h"
#include "../InputBinder.h"
#include "../InputActions.h"
#include "../InputNames.h"
#include "../Settings.h"
#include "../Statistics.h"

#include <nctl/HashSet.h>
#include <ncine/InputEvents.h>
#include <ncine/Application.h>
#include <ncine/FileSystem.h>
#include <ncine/Random.h>
#include <ncine/Sprite.h>
#include <ncine/Font.h>
#include <ncine/TextNode.h>

namespace {
	Menu *menuPtr = nullptr;
	MenuPage *menuPagePtr = nullptr;

	enum SimpleSelectEntry
	{
		START_GAME,
		GOTO_QUIT_PAGE,
		GOTO_SETTINGS_PAGE,
		GOTO_STATISTICS_PAGE,
		GOTO_RESET_STATISTICS_PAGE,
		GOTO_CONTROLS_PAGE,
		GOTO_KEYBOARD_CONTROLS_P1_PAGE,
		GOTO_KEYBOARD_CONTROLS_P2_PAGE,
		GOTO_JOYSTICK_CONTROLS_P1_PAGE,
		GOTO_JOYSTICK_CONTROLS_P2_PAGE,
		GOTO_MAIN_PAGE,
		RESET_STATISTICS,
		QUIT
	};

	enum StatisticsTextEntry
	{
		NUM_MATCHES,
		NUM_BUBBLES,
		NUM_CATCHED_BUBBLES,
		NUM_JUMPS,
		NUM_DOUBLE_JUMPS,
		NUM_DASHES
	};

	enum class RebindingState
	{
		NOT_REBINDING,
		REBINDING_KEY,
		REBINDING_JOY,
		JUST_REBINDED,
	};

	unsigned int waitFrame = 0;
	RebindingState rebindingState = RebindingState::NOT_REBINDING;
	unsigned int rebindActionId = InputBinder::InvalidId;
	nctl::HashSet<nc::KeySym> invalidKeys(32);
	nctl::HashSet<nc::KeySym> assignedKeys(32);
	nctl::HashSet<nc::ButtonName> invalidButtons(32);
	nctl::HashSet<InputBinder::MappedGamepadBinding> assignedMappedControls[2] = {
	    nctl::HashSet<InputBinder::MappedGamepadBinding>(32),
	    nctl::HashSet<InputBinder::MappedGamepadBinding>(32)
	};

	void initInvalidKeys()
	{
		invalidKeys.clear();

		invalidKeys.insert(nc::KeySym::LALT);
		invalidKeys.insert(nc::KeySym::RALT);
		invalidKeys.insert(nc::KeySym::LSHIFT);
		invalidKeys.insert(nc::KeySym::RSHIFT);
		invalidKeys.insert(nc::KeySym::LCTRL);
		invalidKeys.insert(nc::KeySym::RCTRL);
		invalidKeys.insert(nc::KeySym::LSUPER);
		invalidKeys.insert(nc::KeySym::RSUPER);
		invalidKeys.insert(nc::KeySym::BACKSPACE);
		invalidKeys.insert(nc::KeySym::DELETE);
		invalidKeys.insert(nc::KeySym::FUNCTION_KEY);
		invalidKeys.insert(nc::KeySym::TAB);
		invalidKeys.insert(nc::KeySym::ESCAPE);
		invalidKeys.insert(nc::KeySym::CAPS_LOCK);
		invalidKeys.insert(nc::KeySym::PAUSE);
	}

	void initInvalidButtons()
	{
		invalidButtons.clear();

		invalidButtons.insert(nc::ButtonName::BACK);
		invalidButtons.insert(nc::ButtonName::START);
		invalidButtons.insert(nc::ButtonName::GUIDE);
	}

	void initAssignedKeys()
	{
		const InputBinder &ib = inputBinder();
		const InputActions &ia = inputActions();

		assignedKeys.clear();

		assignedKeys.insert(ib.retrieveKeyboardBinding(ia.P1_JUMP));
		assignedKeys.insert(ib.retrieveKeyboardBinding(ia.P1_DASH));
		assignedKeys.insert(ib.retrieveKeyboardBinding(ia.P1_LEFT));
		assignedKeys.insert(ib.retrieveKeyboardBinding(ia.P1_RIGHT));
		assignedKeys.insert(ib.retrieveKeyboardBinding(ia.P2_JUMP));
		assignedKeys.insert(ib.retrieveKeyboardBinding(ia.P2_DASH));
		assignedKeys.insert(ib.retrieveKeyboardBinding(ia.P2_LEFT));
		assignedKeys.insert(ib.retrieveKeyboardBinding(ia.P2_RIGHT));
	}

	void initAssignedGamepadControls()
	{
		const InputBinder &ib = inputBinder();
		const InputActions &ia = inputActions();

		assignedMappedControls[0].clear();
		assignedMappedControls[1].clear();

		const unsigned int Actions[8] = { ia.P1_JUMP, ia.P1_DASH, ia.P1_LEFT, ia.P1_RIGHT,
		                                  ia.P2_JUMP, ia.P2_DASH, ia.P2_LEFT, ia.P2_RIGHT };
		for (unsigned int i = 0; i < 8; i++)
		{
			const InputBinder::MappedGamepadBinding mappedBinding = ib.retrieveMappedGamepadBinding(Actions[i]);
			const unsigned int playerIndex = (i < 4) ? 0 : 1;
			assignedMappedControls[playerIndex].insert(mappedBinding);
		}
	}

	nctl::String secondsToTimeString(unsigned int numSeconds)
	{
		nctl::String string;

		unsigned int numHours = 0;
		unsigned int numMinutes = 0;

		while (numSeconds >= 3600)
		{
			numHours++;
			numSeconds -= 3600;
		}
		while (numSeconds >= 60)
		{
			numMinutes++;
			numSeconds -= 60;
		}

		if (numHours > 0)
			string.format("%02d:%02d:%02d", numHours, numMinutes, numSeconds);
		else
			string.format("%02d:%02d", numMinutes, numSeconds);

		return string;
	}
}

///////////////////////////////////////////////////////////
// STATIC DEFINITIONS
///////////////////////////////////////////////////////////

MenuPage::PageConfig Menu::mainPage_;
MenuPage::PageConfig Menu::quitConfirmationPage_;
MenuPage::PageConfig Menu::settingsPage_;
MenuPage::PageConfig Menu::statisticsPage_;
MenuPage::PageConfig Menu::resetStatisticsConfirmationPage_;
MenuPage::PageConfig Menu::controlsPage_;
MenuPage::PageConfig Menu::keyboardControlsPageP1_;
MenuPage::PageConfig Menu::keyboardControlsPageP2_;
MenuPage::PageConfig Menu::joystickControlsPageP1_;
MenuPage::PageConfig Menu::joystickControlsPageP2_;

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

Menu::Menu(SceneNode *parent, nctl::String name, MyEventHandler *eventHandler)
    : LogicNode(parent, name), eventHandler_(eventHandler)
{
	menuPtr = this;

#ifndef __EMSCRIPTEN__
	const float screenWidth = nc::theApplication().gfxDevice().width();
	const float screenHeight = nc::theApplication().gfxDevice().height();
#else
	const float screenWidth = nc::theApplication().gfxDevice().drawableWidth();
	const float screenHeight = nc::theApplication().gfxDevice().drawableHeight();
#endif
	const nc::Vector2f screenTopRight(screenWidth, screenHeight);

	const float windowScaling = nc::theApplication().gfxDevice().windowScalingFactor();
	this->setScale(windowScaling);

	background_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::Background));
	background_->setPosition(screenTopRight * 0.5f);
	background_->setLayer(Cfg::Layers::Background);
#ifdef __EMSCRIPTEN__
	background_->setSize(screenTopRight);
#endif

	darkForeground_ = nctl::makeUnique<nc::Sprite>(this, nullptr);
	darkForeground_->setSize(screenTopRight);
	darkForeground_->setPosition(screenTopRight * 0.5f);
	darkForeground_->setColor(0, 0, 0, 128);
	darkForeground_->setLayer(Cfg::Layers::Background + 2);

	const nctl::String titleFontFntPath_ = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Fonts::Modak200Fnt);
	titleFont_ = nctl::makeUnique<nc::Font>(titleFontFntPath_.data(), resourceManager().retrieveTexture(Cfg::Fonts::Modak200Png));

	const nctl::String smallFontFntPath_ = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Fonts::Modak20Fnt);
	smallFont_ = nctl::makeUnique<nc::Font>(smallFontFntPath_.data(), resourceManager().retrieveTexture(Cfg::Fonts::Modak20Png));

	gameTitleText_ = nctl::makeUnique<nc::TextNode>(this, titleFont_.get(), 32);
	gameTitleText_->setLayer(Cfg::Layers::Gui_Text);
	gameTitleText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	gameTitleText_->setString("Wet Paper");
	gameTitleText_->setPosition(screenTopRight.x * 0.5f, screenTopRight.y * 0.75f);

	nctl::String versionString(64);
	versionString.format("Wet Paper r%s.%s (%s)", VersionStrings::GitRevCount, VersionStrings::GitShortHash, VersionStrings::CompilationDate);
	versionText_ = nctl::makeUnique<nc::TextNode>(this, smallFont_.get(), 64);
	versionText_->setLayer(Cfg::Layers::Gui_Text);
	versionText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	versionText_->setString(versionString);
	versionText_->setPosition(screenTopRight.x - versionText_->width() * 0.5f, versionText_->height() * 0.75f);

	statusText_ = nctl::makeUnique<nc::TextNode>(this, smallFont_.get(), 64);
	statusText_->setLayer(Cfg::Layers::Gui_Text);
	statusText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	statusText_->setEnabled(false);

	menuPage_ = nctl::makeUnique<MenuPage>(this, "MenuPage");
	menuPagePtr = menuPage_.get();
	menuPage_->setPosition(screenTopRight * Cfg::Menu::MenuPageRelativePos);
	setupPages();
	menuPage_->setup(mainPage_);

	for (unsigned int i = 0; i < NumBubbles; i++)
	{
		const nc::Vector2f pos(lerp(screenWidth * 0.15f, screenWidth - screenWidth * 0.15f, nc::random().real()),
		                       lerp(screenHeight * 0.15f, screenHeight - screenHeight * 0.15f, nc::random().real()));

		nc::Vector2f direction(nc::Vector2f(nc::random().real(), nc::random().real()).normalized());
		bubbleDirections_.pushBack(direction);
		const float speedVar = Cfg::Menu::BackgroundBubbleSpeedVariance;
		bubbleSpeeds_.pushBack(nc::random().real(1.0f - speedVar, 1.0f + speedVar) * Cfg::Menu::BackgroundBubbleSpeed);

		const unsigned int variant = nc::random().integer(0, Cfg::Textures::NumBubbleVariants);
		nc::Texture *tex = resourceManager().retrieveTexture(Cfg::Textures::Bubbles[variant]);
		nctl::UniquePtr<nc::Sprite> bubble = nctl::makeUnique<nc::Sprite>(this, tex, pos);
		bubble->setLayer(Cfg::Layers::Background + 1);

		bubbles_.pushBack(nctl::move(bubble));
	}
}

Menu::~Menu()
{
	menuPtr = nullptr;
	menuPagePtr = nullptr;
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Menu::onTick(float deltaTime)
{
	const float screenWidth = nc::theApplication().gfxDevice().width();
	const float screenHeight = nc::theApplication().gfxDevice().height();

	for (unsigned int i = 0; i < NumBubbles; i++)
	{
		nc::Sprite &bubble = *bubbles_[i];
		const nc::Vector2f &pos = bubble.position();
		nc::Vector2f newPos = bubble.position();
		const nc::Vector2f &halfSize = bubble.size() * 0.5f;
		nc::Vector2f &bubbleDirection = bubbleDirections_[i];

		// Collision response
		if (pos.x - halfSize.x <= 0.0f)
			newPos.x = halfSize.x;
		if (pos.x + halfSize.x >= screenWidth)
			newPos.x = screenWidth - halfSize.x;
		if (pos.y - halfSize.y <= 0.0f)
			newPos.y = halfSize.y;
		if (pos.y + halfSize.y >= screenHeight)
			newPos.y = screenHeight - halfSize.y;

		if (pos.x - halfSize.x <= 0.0f || pos.x + halfSize.x >= screenWidth)
			bubbleDirection.x *= -1.0f;
		if (pos.y - halfSize.y <= 0.0f || pos.y + halfSize.y >= screenHeight)
			bubbleDirection.y *= -1.0f;

		bubble.setPosition(newPos);
		const float speed = bubbleSpeeds_[i] * deltaTime;
		bubble.move(bubbleDirection * speed);
	}

	if (statusText_->isEnabled())
		statusText_->setPosition(screenWidth * 0.5f - statusText_->width() * 0.5f, statusText_->height() * 0.75f * 10);

	if (rebindingState == RebindingState::JUST_REBINDED)
	{
		waitFrame++;
		if (waitFrame == 2)
		{
			waitFrame = 0;
			rebindingState = RebindingState::NOT_REBINDING;
			menuPagePtr->enableActions(true);
		}
	}
}

void Menu::drawGui()
{
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	ImGui::Text("Version: r%s.%s (%s)", VersionStrings::GitRevCount, VersionStrings::GitShortHash, VersionStrings::CompilationDate);
	ImGui::NewLine();

	menuPage_->drawGui();
#endif
}

void Menu::onKeyPressed(const nc::KeyboardEvent &event)
{
	FATAL_ASSERT(menuPtr != nullptr);
	FATAL_ASSERT(menuPagePtr != nullptr);

	// Also accepting `REBINDING_JOY` state to use Escape when binding a gamepad control
	if ((rebindingState != RebindingState::REBINDING_KEY && rebindingState != RebindingState::REBINDING_JOY) || rebindActionId == InputBinder::InvalidId)
		return;

	InputBinder &ib = inputBinder();
	const nc::KeySym key = event.sym;
	const nc::KeySym oldKey = ib.retrieveKeyboardBinding(rebindActionId);

	// Filtering the key before assigning it
	const bool keyIsValid = (invalidKeys.contains(key) == false && assignedKeys.contains(key) == false);
	if (keyIsValid)
	{
		ib.setKeyboardBinding(rebindActionId, key);
		assignedKeys.remove(oldKey);
		assignedKeys.insert(key);
	}

	if (keyIsValid || key == oldKey || key == nc::KeySym::ESCAPE)
	{
		rebindingState = RebindingState::JUST_REBINDED;
		rebindActionId = InputBinder::InvalidId;
		menuPtr->statusText_->setEnabled(false);
	}

	menuPagePtr->updateAllEntriesText();
}

void Menu::onJoyMappedButtonPressed(const nc::JoyMappedButtonEvent &event)
{
	FATAL_ASSERT(menuPtr != nullptr);
	FATAL_ASSERT(menuPagePtr != nullptr);

	const bool joyIdIsValid = (event.joyId >= 0 && event.joyId <= 1);
	if (rebindingState != RebindingState::REBINDING_JOY || rebindActionId == InputBinder::InvalidId || !joyIdIsValid)
		return;

	InputBinder &ib = inputBinder();
	const InputBinder::MappedGamepadBinding oldMappedBinding = ib.retrieveMappedGamepadBinding(rebindActionId);
	if (event.joyId != oldMappedBinding.joyId)
		return;

	InputBinder::MappedGamepadBinding mappedBinding = oldMappedBinding;
	mappedBinding.button = event.buttonName;
	mappedBinding.axisSide = InputBinder::AxisSide::POSITIVE;
	mappedBinding.axis = nc::AxisName::UNKNOWN;

	// Filtering the mapping before assigning it
	const bool mappingIsValid = (invalidButtons.contains(event.buttonName) == false &&
	                             assignedMappedControls[event.joyId].contains(mappedBinding) == false);
	if (mappingIsValid)
	{
		ib.setMappedGamepadBinding(rebindActionId, mappedBinding);
		assignedMappedControls[event.joyId].remove(oldMappedBinding);
		assignedMappedControls[event.joyId].insert(mappedBinding);
	}

	if (mappingIsValid || event.buttonName == oldMappedBinding.button)
	{
		rebindingState = RebindingState::JUST_REBINDED;
		rebindActionId = InputBinder::InvalidId;
		menuPtr->statusText_->setEnabled(false);
	}

	menuPagePtr->updateAllEntriesText();
}

void Menu::onJoyMappedAxisMoved(const nc::JoyMappedAxisEvent &event)
{
	FATAL_ASSERT(menuPtr != nullptr);
	FATAL_ASSERT(menuPagePtr != nullptr);

	const bool axisMoved = (event.value >= InputBinder::PressedAxisThreshold) || (event.value <= -InputBinder::PressedAxisThreshold);
	const bool joyIdIsValid = (event.joyId >= 0 && event.joyId <= 1);
	if (rebindingState != RebindingState::REBINDING_JOY || rebindActionId == InputBinder::InvalidId || !axisMoved || !joyIdIsValid)
		return;

	InputBinder &ib = inputBinder();
	const InputBinder::MappedGamepadBinding oldMappedBinding = ib.retrieveMappedGamepadBinding(rebindActionId);
	if (event.joyId != oldMappedBinding.joyId)
		return;

	InputBinder::MappedGamepadBinding mappedBinding = oldMappedBinding;
	mappedBinding.button = nc::ButtonName::UNKNOWN;
	mappedBinding.axisSide = (event.value >= InputBinder::PressedAxisThreshold) ? InputBinder::AxisSide::POSITIVE : InputBinder::AxisSide::NEGATIVE;
	mappedBinding.axis = event.axisName;

	// Filtering the mapping before assigning it
	const bool mappingIsValid = (assignedMappedControls[event.joyId].contains(mappedBinding) == false);
	if (mappingIsValid)
	{
		ib.setMappedGamepadBinding(rebindActionId, mappedBinding);
		assignedMappedControls[event.joyId].remove(oldMappedBinding);
		assignedMappedControls[event.joyId].insert(mappedBinding);
	}

	if (mappingIsValid || event.axisName == oldMappedBinding.axis)
	{
		rebindingState = RebindingState::JUST_REBINDED;
		rebindActionId = InputBinder::InvalidId;
		menuPtr->statusText_->setEnabled(false);
	}

	menuPagePtr->updateAllEntriesText();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void Menu::setupPages()
{
	// Main menu page
	{
		MenuPage::PageEntry startEntry("Start", reinterpret_cast<void *>(SimpleSelectEntry::START_GAME), Menu::simpleSelectFunc);
		MenuPage::PageEntry settingsEntry("Settings", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_SETTINGS_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry statisticsEntry("Statistics", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_STATISTICS_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry quitEntry("Quit", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_QUIT_PAGE), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		startEntry.eventReplyFunc = Menu::selectEventReplyFunc;
		settingsEntry.eventReplyFunc = Menu::selectEventReplyFunc;
		statisticsEntry.eventReplyFunc = Menu::selectEventReplyFunc;
		quitEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		mainPage_ = {}; // clear the static variable
		mainPage_.entries.pushBack(startEntry);
		mainPage_.entries.pushBack(settingsEntry);
		mainPage_.entries.pushBack(statisticsEntry);
		mainPage_.entries.pushBack(quitEntry);
	}

	// Quit confirmation page
	{
		MenuPage::PageEntry noEntry("No", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_MAIN_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry yesEntry("Yes", reinterpret_cast<void *>(SimpleSelectEntry::QUIT), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		noEntry.eventReplyFunc = Menu::selectEventReplyFunc;
		yesEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		quitConfirmationPage_ = {}; // clear the static variable
		quitConfirmationPage_.entries.pushBack(noEntry);
		quitConfirmationPage_.entries.pushBack(yesEntry);
		quitConfirmationPage_.title = "Are you sure you want to quit?";
		quitConfirmationPage_.backFunc = Menu::goToMainPage;
	}

	// Settings page
	{
		MenuPage::PageEntry playersEntry("Players", nullptr, Menu::settingsPlayersFunc);
		MenuPage::PageEntry matchTimeEntry("Match Time", nullptr, Menu::settingsMatchTimeFunc);
		MenuPage::PageEntry volumeEntry("Volume", nullptr, Menu::settingsVolumeFunc);
		MenuPage::PageEntry controlsEntry("Controls", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_CONTROLS_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry backEntry("Back", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_MAIN_PAGE), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		playersEntry.eventReplyFunc = Menu::leftRightTextEventReplyFunc;
		matchTimeEntry.eventReplyFunc = Menu::leftRightTextEventReplyFunc;
		volumeEntry.eventReplyFunc = Menu::leftRightTextEventReplyFunc;
		controlsEntry.eventReplyFunc = Menu::selectEventReplyFunc;
		backEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		settingsPage_ = {}; // clear the static variable
		settingsPage_.entries.pushBack(playersEntry);
		settingsPage_.entries.pushBack(matchTimeEntry);
		settingsPage_.entries.pushBack(volumeEntry);
		settingsPage_.entries.pushBack(controlsEntry);
		settingsPage_.entries.pushBack(backEntry);
		settingsPage_.title = "Settings";
		settingsPage_.backFunc = Menu::goToMainPage;
	}

	// Statistics page
	{
		MenuPage::PageEntry numMatchesEntry("Matches", reinterpret_cast<void *>(StatisticsTextEntry::NUM_MATCHES), Menu::statisticsTextFunc);
		MenuPage::PageEntry numBubblesEntry("Bubbles", reinterpret_cast<void *>(StatisticsTextEntry::NUM_BUBBLES), Menu::statisticsTextFunc);
		MenuPage::PageEntry numCatchedBubblesEntry("Catched Bubbles", reinterpret_cast<void *>(StatisticsTextEntry::NUM_CATCHED_BUBBLES), Menu::statisticsTextFunc);
		MenuPage::PageEntry numJumpsEntry("Jumps", reinterpret_cast<void *>(StatisticsTextEntry::NUM_JUMPS), Menu::statisticsTextFunc);
		MenuPage::PageEntry numDoubleJumpsEntry("Double Jumps", reinterpret_cast<void *>(StatisticsTextEntry::NUM_DOUBLE_JUMPS), Menu::statisticsTextFunc);
		MenuPage::PageEntry numDashesEntry("Dashes", reinterpret_cast<void *>(StatisticsTextEntry::NUM_DASHES), Menu::statisticsTextFunc);
		MenuPage::PageEntry resetEntry("Reset", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_RESET_STATISTICS_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry backEntry("Back", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_MAIN_PAGE), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		numMatchesEntry.eventReplyFunc = Menu::textEventReplyFunc;
		numBubblesEntry.eventReplyFunc = Menu::textEventReplyFunc;
		numCatchedBubblesEntry.eventReplyFunc = Menu::textEventReplyFunc;
		numJumpsEntry.eventReplyFunc = Menu::textEventReplyFunc;
		numDoubleJumpsEntry.eventReplyFunc = Menu::textEventReplyFunc;
		numDashesEntry.eventReplyFunc = Menu::textEventReplyFunc;
		resetEntry.eventReplyFunc = Menu::selectEventReplyFunc;
		backEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		statisticsPage_ = {}; // clear the static variable
		statisticsPage_.entries.pushBack(numMatchesEntry);
		statisticsPage_.entries.pushBack(numBubblesEntry);
		statisticsPage_.entries.pushBack(numCatchedBubblesEntry);
		statisticsPage_.entries.pushBack(numJumpsEntry);
		statisticsPage_.entries.pushBack(numDoubleJumpsEntry);
		statisticsPage_.entries.pushBack(numDashesEntry);
		statisticsPage_.entries.pushBack(resetEntry);
		statisticsPage_.entries.pushBack(backEntry);
		statisticsPage_.title = "Statistics";
		statisticsPage_.backFunc = Menu::goToMainPage;
	}

	// Reset statistics confirmation page
	{
		MenuPage::PageEntry noEntry("No", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_STATISTICS_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry yesEntry("Yes", reinterpret_cast<void *>(SimpleSelectEntry::RESET_STATISTICS), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		noEntry.eventReplyFunc = Menu::selectEventReplyFunc;
		yesEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		resetStatisticsConfirmationPage_ = {}; // clear the static variable
		resetStatisticsConfirmationPage_.entries.pushBack(noEntry);
		resetStatisticsConfirmationPage_.entries.pushBack(yesEntry);
		resetStatisticsConfirmationPage_.title = "Are you sure you want to reset statistics?";
		resetStatisticsConfirmationPage_.backFunc = Menu::goToStatisticsPage;
	}

	// Controls page
	{
		MenuPage::PageEntry keyboardP1Entry("Keyboard P1", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_KEYBOARD_CONTROLS_P1_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry keyboardP2Entry("Keyboard P2", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_KEYBOARD_CONTROLS_P2_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry joystickP1Entry("Joystick P1", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_JOYSTICK_CONTROLS_P1_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry joystickP2Entry("Joystick P2", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_JOYSTICK_CONTROLS_P2_PAGE), Menu::simpleSelectFunc);
		MenuPage::PageEntry backEntry("Back", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_SETTINGS_PAGE), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		keyboardP1Entry.eventReplyFunc = Menu::selectEventReplyFunc;
		keyboardP2Entry.eventReplyFunc = Menu::selectEventReplyFunc;
		joystickP1Entry.eventReplyFunc = Menu::selectEventReplyFunc;
		joystickP2Entry.eventReplyFunc = Menu::selectEventReplyFunc;
		backEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		controlsPage_ = {}; // clear the static variable
		controlsPage_.entries.pushBack(keyboardP1Entry);
		controlsPage_.entries.pushBack(keyboardP2Entry);
		controlsPage_.entries.pushBack(joystickP1Entry);
		controlsPage_.entries.pushBack(joystickP2Entry);
		controlsPage_.entries.pushBack(backEntry);
		controlsPage_.title = "Controls";
		controlsPage_.backFunc = Menu::goToSettingsPage;
	}

	const InputActions &ia = inputActions();
	// Keyboard controls P1 page
	{
		MenuPage::PageEntry keyboardP1JumpEntry("P1 Jump", reinterpret_cast<void *>(ia.P1_JUMP), keyboardControlsFunc);
		MenuPage::PageEntry keyboardP1DashEntry("P1 Dash", reinterpret_cast<void *>(ia.P1_DASH), keyboardControlsFunc);
		MenuPage::PageEntry keyboardP1LeftEntry("P1 Left", reinterpret_cast<void *>(ia.P1_LEFT), keyboardControlsFunc);
		MenuPage::PageEntry keyboardP1RightEntry("P1 Right", reinterpret_cast<void *>(ia.P1_RIGHT), keyboardControlsFunc);
		MenuPage::PageEntry backEntry("Back", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_CONTROLS_PAGE), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		keyboardP1JumpEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		keyboardP1DashEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		keyboardP1LeftEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		keyboardP1RightEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		backEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		keyboardControlsPageP1_ = {}; // clear the static variable
		keyboardControlsPageP1_.entries.pushBack(keyboardP1JumpEntry);
		keyboardControlsPageP1_.entries.pushBack(keyboardP1DashEntry);
		keyboardControlsPageP1_.entries.pushBack(keyboardP1LeftEntry);
		keyboardControlsPageP1_.entries.pushBack(keyboardP1RightEntry);
		keyboardControlsPageP1_.entries.pushBack(backEntry);
		keyboardControlsPageP1_.title = "Keyboard P1";
		keyboardControlsPageP1_.backFunc = Menu::goToControlsPage;
	}

	// Keyboard controls P2 page
	{
		MenuPage::PageEntry keyboardP2JumpEntry("P2 Jump", reinterpret_cast<void *>(ia.P2_JUMP), keyboardControlsFunc);
		MenuPage::PageEntry keyboardP2DashEntry("P2 Dash", reinterpret_cast<void *>(ia.P2_DASH), keyboardControlsFunc);
		MenuPage::PageEntry keyboardP2LeftEntry("P2 Left", reinterpret_cast<void *>(ia.P2_LEFT), keyboardControlsFunc);
		MenuPage::PageEntry keyboardP2RightEntry("P2 Right", reinterpret_cast<void *>(ia.P2_RIGHT), keyboardControlsFunc);
		MenuPage::PageEntry backEntry("Back", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_CONTROLS_PAGE), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		keyboardP2JumpEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		keyboardP2DashEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		keyboardP2LeftEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		keyboardP2RightEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		backEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		keyboardControlsPageP2_ = {}; // clear the static variable
		keyboardControlsPageP2_.entries.pushBack(keyboardP2JumpEntry);
		keyboardControlsPageP2_.entries.pushBack(keyboardP2DashEntry);
		keyboardControlsPageP2_.entries.pushBack(keyboardP2LeftEntry);
		keyboardControlsPageP2_.entries.pushBack(keyboardP2RightEntry);
		keyboardControlsPageP2_.entries.pushBack(backEntry);
		keyboardControlsPageP2_.title = "Keyboard P2";
		keyboardControlsPageP2_.backFunc = Menu::goToControlsPage;
	}

	// Joystick controls P1 page
	{
		MenuPage::PageEntry joyP1JumpEntry("P1 Jump", reinterpret_cast<void *>(ia.P1_JUMP), joystickControlsFunc);
		MenuPage::PageEntry joyP1DashEntry("P1 Dash", reinterpret_cast<void *>(ia.P1_DASH), joystickControlsFunc);
		MenuPage::PageEntry joyP1LeftEntry("P1 Left", reinterpret_cast<void *>(ia.P1_LEFT), joystickControlsFunc);
		MenuPage::PageEntry joyP1RightEntry("P1 Right", reinterpret_cast<void *>(ia.P1_RIGHT), joystickControlsFunc);
		MenuPage::PageEntry backEntry("Back", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_CONTROLS_PAGE), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		joyP1JumpEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		joyP1DashEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		joyP1LeftEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		joyP1RightEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		backEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		joystickControlsPageP1_ = {}; // clear the static variable
		joystickControlsPageP1_.entries.pushBack(joyP1JumpEntry);
		joystickControlsPageP1_.entries.pushBack(joyP1DashEntry);
		joystickControlsPageP1_.entries.pushBack(joyP1LeftEntry);
		joystickControlsPageP1_.entries.pushBack(joyP1RightEntry);
		joystickControlsPageP1_.entries.pushBack(backEntry);
		joystickControlsPageP1_.title = "Joystick P1";
		joystickControlsPageP1_.backFunc = Menu::goToControlsPage;
	}

	// Joystick controls P2 page
	{
		MenuPage::PageEntry joyP2JumpEntry("P2 Jump", reinterpret_cast<void *>(ia.P2_JUMP), joystickControlsFunc);
		MenuPage::PageEntry joyP2DashEntry("P2 Dash", reinterpret_cast<void *>(ia.P2_DASH), joystickControlsFunc);
		MenuPage::PageEntry joyP2LeftEntry("P2 Left", reinterpret_cast<void *>(ia.P2_LEFT), joystickControlsFunc);
		MenuPage::PageEntry joyP2RightEntry("P2 Right", reinterpret_cast<void *>(ia.P2_RIGHT), joystickControlsFunc);
		MenuPage::PageEntry backEntry("Back", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_CONTROLS_PAGE), Menu::simpleSelectFunc);

#if defined(NCPROJECT_DEBUG)
		joyP2JumpEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		joyP2DashEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		joyP2LeftEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		joyP2RightEntry.eventReplyFunc = Menu::selectTextEventReplyFunc;
		backEntry.eventReplyFunc = Menu::selectEventReplyFunc;
#endif

		joystickControlsPageP2_ = {}; // clear the static variable
		joystickControlsPageP2_.entries.pushBack(joyP2JumpEntry);
		joystickControlsPageP2_.entries.pushBack(joyP2DashEntry);
		joystickControlsPageP2_.entries.pushBack(joyP2LeftEntry);
		joystickControlsPageP2_.entries.pushBack(joyP2RightEntry);
		joystickControlsPageP2_.entries.pushBack(backEntry);
		joystickControlsPageP2_.title = "Joystick P2";
		joystickControlsPageP2_.backFunc = Menu::goToControlsPage;
	}

	initInvalidKeys();
	initAssignedKeys();
	initInvalidButtons();
	initAssignedGamepadControls();
}

void Menu::goToMainPage()
{
	FATAL_ASSERT(menuPagePtr != nullptr);
	menuPagePtr->setup(mainPage_);
}

void Menu::goToSettingsPage()
{
	FATAL_ASSERT(menuPagePtr != nullptr);
	menuPagePtr->setup(settingsPage_);
}

void Menu::goToControlsPage()
{
	FATAL_ASSERT(menuPagePtr != nullptr);
	menuPagePtr->setup(controlsPage_);
}

void Menu::goToStatisticsPage()
{
	FATAL_ASSERT(menuPagePtr != nullptr);
	menuPagePtr->setup(statisticsPage_);
}

void Menu::simpleSelectFunc(MenuPage::EntryEvent &event)
{
	if (event.type != MenuPage::EventType::SELECT)
		return;

	FATAL_ASSERT(menuPtr != nullptr);
	FATAL_ASSERT(menuPagePtr != nullptr);

	const SimpleSelectEntry entry = static_cast<SimpleSelectEntry>(reinterpret_cast<uintptr_t>(event.entryData));

	switch (entry)
	{
		case START_GAME:
			menuPtr->eventHandler_->requestGame();
			break;
		case GOTO_QUIT_PAGE:
			menuPagePtr->setup(quitConfirmationPage_);
			break;
		case GOTO_SETTINGS_PAGE:
			menuPagePtr->setup(settingsPage_);
			break;
		case GOTO_STATISTICS_PAGE:
			menuPagePtr->setup(statisticsPage_);
			break;
		case GOTO_RESET_STATISTICS_PAGE:
			menuPagePtr->setup(resetStatisticsConfirmationPage_);
			break;
		case GOTO_CONTROLS_PAGE:
			menuPagePtr->setup(controlsPage_);
			break;
		case GOTO_KEYBOARD_CONTROLS_P1_PAGE:
			menuPagePtr->setup(keyboardControlsPageP1_);
			break;
		case GOTO_KEYBOARD_CONTROLS_P2_PAGE:
			menuPagePtr->setup(keyboardControlsPageP2_);
			break;
		case GOTO_JOYSTICK_CONTROLS_P1_PAGE:
			menuPagePtr->setup(joystickControlsPageP1_);
			break;
		case GOTO_JOYSTICK_CONTROLS_P2_PAGE:
			menuPagePtr->setup(joystickControlsPageP2_);
			break;
		case GOTO_MAIN_PAGE:
			menuPagePtr->setup(mainPage_);
			break;
		case RESET_STATISTICS:
			menuPtr->eventHandler_->statisticsMut() = {};
			menuPagePtr->setup(statisticsPage_);
			break;
		case QUIT:
			nc::theApplication().quit();
			break;
		default:
			break;
	}
}

void Menu::statisticsTextFunc(MenuPage::EntryEvent &event)
{
	if (event.type != MenuPage::EventType::TEXT)
		return;

	FATAL_ASSERT(menuPtr != nullptr);
	const StatisticsTextEntry entry = static_cast<StatisticsTextEntry>(reinterpret_cast<uintptr_t>(event.entryData));
	const Statistics &statistics = menuPtr->eventHandler_->statistics();
	const PlayerStatistics &statisticsA = statistics.playerStats[0];
	const PlayerStatistics &statisticsB = statistics.playerStats[1];

	const unsigned int numCatchedBubbles = statisticsA.numCatchedBubbles + statisticsB.numCatchedBubbles;
	const unsigned int numJumps = statisticsA.numJumps + statisticsB.numJumps;
	const unsigned int numDoubleJumps = statisticsA.numDoubleJumps + statisticsB.numDoubleJumps;
	const unsigned int numDashes = statisticsA.numDashes + statisticsB.numDashes;

	switch (entry)
	{
		case NUM_MATCHES:
		{
			const nctl::String timeString = secondsToTimeString(statistics.playTime);
			event.entryText.format("Matches: %d / Time: %s", statistics.numMatches, timeString.data());
			break;
		}
		case NUM_BUBBLES:
			event.entryText.format("Bubbles: %d catched, %d dropped", numCatchedBubbles, statistics.numDroppedBubles);
			break;
		case NUM_CATCHED_BUBBLES:
			event.entryText.format("Catched Bubbles: P1 %d / P2 %d", statisticsA.numCatchedBubbles, statisticsB.numCatchedBubbles);
			break;
		case NUM_JUMPS:
			event.entryText.format("Jumps: %d (P1 %d / P2 %d)", numJumps, statisticsA.numJumps, statisticsB.numJumps);
			break;
		case NUM_DOUBLE_JUMPS:
			event.entryText.format("Double Jumps: %d (P1 %d / P2 %d)", numDoubleJumps, statisticsA.numDoubleJumps, statisticsB.numDoubleJumps);
			break;
		case NUM_DASHES:
			event.entryText.format("Dashes: %d (P1 %d / P2 %d)", numDashes, statisticsA.numDashes, statisticsB.numDashes);
			break;
		default:
			break;
	}
}

void Menu::settingsPlayersFunc(MenuPage::EntryEvent &event)
{
	FATAL_ASSERT(menuPtr != nullptr);
	const Settings &settings = menuPtr->eventHandler_->settings();
	unsigned int numPlayers = settings.numPlayers;

	switch (event.type)
	{
		case MenuPage::EventType::LEFT:
			if (numPlayers == 2)
				numPlayers = 1;
			break;
		case MenuPage::EventType::RIGHT:
			if (numPlayers == 1)
				numPlayers = 2;
			break;
		default:
			break;
	}

	switch (event.type)
	{
		case MenuPage::EventType::LEFT:
		case MenuPage::EventType::RIGHT:
			if (numPlayers != settings.numPlayers)
			{
				Settings &settingsMut = menuPtr->eventHandler_->settingsMut();
				settingsMut.numPlayers = numPlayers;
				event.shouldUpdateEntryText = true;
			}
			break;
		case MenuPage::EventType::TEXT:
		{
			event.entryText.format("%sPlayers: %u%s", (numPlayers == 2) ? "< " : "", numPlayers, (numPlayers == 1) ? " >" : "");
			break;
		}
		default:
			break;
	}
}

void Menu::settingsMatchTimeFunc(MenuPage::EntryEvent &event)
{
	FATAL_ASSERT(menuPtr != nullptr);
	const Settings &settings = menuPtr->eventHandler_->settings();
	unsigned int matchTime = settings.matchTime;

	switch (event.type)
	{
		case MenuPage::EventType::LEFT:
			if (matchTime > Cfg::Settings::MatchTimeMin)
				matchTime -= Cfg::Settings::MatchTimeStep;
			break;
		case MenuPage::EventType::RIGHT:
			if (matchTime < Cfg::Settings::MatchTimeMax)
				matchTime += Cfg::Settings::MatchTimeStep;
			break;
		default:
			break;
	}

	switch (event.type)
	{
		case MenuPage::EventType::LEFT:
		case MenuPage::EventType::RIGHT:
			if (matchTime != settings.matchTime)
			{
				Settings &settingsMut = menuPtr->eventHandler_->settingsMut();
				settingsMut.matchTime = matchTime;
				event.shouldUpdateEntryText = true;
			}
			break;
		case MenuPage::EventType::TEXT:
		{
			event.entryText.format("%sMatch Time: %u%s", (matchTime > Cfg::Settings::MatchTimeMin) ? "< " : "",
			                       matchTime, (matchTime < Cfg::Settings::MatchTimeMax) ? " >" : "");
			break;
		}
		default:
			break;
	}
}

void Menu::settingsVolumeFunc(MenuPage::EntryEvent &event)
{
	FATAL_ASSERT(menuPtr != nullptr);
	const Settings &settings = menuPtr->eventHandler_->settings();
	float gain = settings.volume;

	switch (event.type)
	{
		case MenuPage::EventType::LEFT:
			gain -= Cfg::Settings::VolumeGainStep;
			if (gain < Cfg::Settings::VolumeGainMin)
				gain = Cfg::Settings::VolumeGainMin;
			break;
		case MenuPage::EventType::RIGHT:
			gain += Cfg::Settings::VolumeGainStep;
			if (gain > Cfg::Settings::VolumeGainMax)
				gain = Cfg::Settings::VolumeGainMax;
			break;
		default:
			break;
	}

	switch (event.type)
	{
		case MenuPage::EventType::LEFT:
		case MenuPage::EventType::RIGHT:
			if (gain != settings.volume)
			{
				nc::IAudioDevice &audioDevice = nc::theServiceLocator().audioDevice();
				audioDevice.setGain(gain);
				Settings &settingsMut = menuPtr->eventHandler_->settingsMut();
				settingsMut.volume = gain;
				event.shouldUpdateEntryText = true;
			}
			break;
		case MenuPage::EventType::TEXT:
		{
			const unsigned gainPercentagePart = (static_cast<unsigned int>(gain * 100) + 1);
			const unsigned gainPercentage = gainPercentagePart - gainPercentagePart % 10;

			event.entryText.format("%sVolume: %u%%%s", (gainPercentage > 0) ? "< " : "",
			                       gainPercentage, (gainPercentage < 100) ? " >" : "");
			break;
		}
		default:
			break;
	}
}

void Menu::keyboardControlsFunc(MenuPage::EntryEvent &event)
{
	InputBinder &ib = inputBinder();

	const unsigned int actionId = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(event.entryData));
	if (event.type == MenuPage::EventType::SELECT)
	{
		rebindingState = RebindingState::REBINDING_KEY;
		rebindActionId = actionId;
		menuPagePtr->enableActions(false);
		event.shouldUpdateEntryText = true;

		nc::TextNode &statusText = *menuPtr->statusText_;
		statusText.setString("Press a key to assign it, or Escape to cancel");
		statusText.setEnabled(true);
	}
	else if (event.type == MenuPage::EventType::TEXT)
	{
		const nc::KeySym key = ib.retrieveKeyboardBinding(actionId);
		if (rebindingState == RebindingState::REBINDING_KEY && rebindActionId == actionId)
			event.entryText.format("%s: __", ib.actionName(actionId));
		else
			event.entryText.format("%s: \"%s\" (%u)", ib.actionName(actionId), keySymToName(key), key);
	}
}

void Menu::joystickControlsFunc(MenuPage::EntryEvent &event)
{
	InputBinder &ib = inputBinder();

	const unsigned int actionId = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(event.entryData));
	if (event.type == MenuPage::EventType::SELECT)
	{
		rebindingState = RebindingState::REBINDING_JOY;
		rebindActionId = actionId;
		menuPagePtr->enableActions(false);
		event.shouldUpdateEntryText = true;

		nc::TextNode &statusText = *menuPtr->statusText_;
		statusText.setString("Press a button or move an axis to assign it, or Escape to cancel");
		statusText.setEnabled(true);
	}
	else if (event.type == MenuPage::EventType::TEXT)
	{
		const InputBinder::MappedGamepadBinding mappedBinding = ib.retrieveMappedGamepadBinding(actionId);
		if (rebindingState == RebindingState::REBINDING_JOY && rebindActionId == actionId)
			event.entryText.format("%s: __", ib.actionName(actionId));
		else
		{
			if (mappedBinding.button != nc::ButtonName::UNKNOWN)
				event.entryText.format("%s: \"%s\"", ib.actionName(actionId), buttonToName(mappedBinding.button));
			else if (mappedBinding.axis != nc::AxisName::UNKNOWN)
				event.entryText.format("%s: \"%s %s\"", ib.actionName(actionId), axisToName(mappedBinding.axis),
				                       mappedBinding.axisSide == InputBinder::AxisSide::POSITIVE ? "+" : "-");
		}
	}
}

#if defined(NCPROJECT_DEBUG)
bool Menu::selectEventReplyFunc(MenuPage::EventType type)
{
	return (type == MenuPage::EventType::SELECT);
}

bool Menu::textEventReplyFunc(MenuPage::EventType type)
{
	return (type == MenuPage::EventType::TEXT);
}

bool Menu::selectTextEventReplyFunc(MenuPage::EventType type)
{
	return (type == MenuPage::EventType::SELECT ||
	        type == MenuPage::EventType::TEXT);
}

bool Menu::leftRightTextEventReplyFunc(MenuPage::EventType type)
{
	return (type == MenuPage::EventType::LEFT ||
	        type == MenuPage::EventType::RIGHT ||
	        type == MenuPage::EventType::TEXT);
}
#endif
