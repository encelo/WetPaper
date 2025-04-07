#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "Game.h"
#include "Player.h"
#include "Bubble.h"
#include "Body.h"
#include "../ResourceManager.h"
#include "../InputBinder.h"
#include "../InputActions.h"
#include "../Settings.h"
#include "../main.h"

#include <ncine/Application.h>
#include <ncine/FileSystem.h>
#include <ncine/Texture.h>
#include <ncine/Sprite.h>
#include <ncine/AudioBufferPlayer.h>
#include <ncine/Font.h>
#include <ncine/TextNode.h>
#include <ncine/Random.h>

namespace {
	Game *gamePtr = nullptr;
	MenuPage *menuPagePtr = nullptr;
	nctl::String auxString(256);

	enum SimpleSelectEntry
	{
		RESUME_GAME,
		GOTO_MAIN_MENU,
		GOTO_QUIT_PAGE_FROM_PAUSE,
		GOTO_QUIT_PAGE_FROM_ENDMATCH,
		GOTO_PAUSE_PAGE,
		GOTO_END_MATCH_PAGE,
		QUIT
	};

	enum StatisticsTextEntry
	{
		NUM_BUBBLES,
		NUM_CATCHED_BUBBLES,
		NUM_JUMPS,
		NUM_DOUBLE_JUMPS,
		NUM_DASHES
	};
}

///////////////////////////////////////////////////////////
// STATIC DEFINITIONS
///////////////////////////////////////////////////////////

MenuPage::PageConfig Game::pausePage_;
MenuPage::PageConfig Game::endMatchPage_;
MenuPage::PageConfig Game::quitConfirmationPausePage_;
MenuPage::PageConfig Game::quitConfirmationEndMatchPage_;

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

Game::Game(SceneNode *parent, nctl::String name, MyEventHandler *eventHandler)
    : LogicNode(parent, name), eventHandler_(eventHandler),
      paused_(false), matchEnded_(false)
{
	gamePtr = this;
	loadScene();
}

Game::~Game()
{
	gamePtr = nullptr;
	menuPagePtr = nullptr;
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Game::onTick(float deltaTime)
{
	if (inputBinder().isTriggered(inputActions().GAME_PAUSE))
		togglePause();

	const float matchTimeFloat = static_cast<float>(eventHandler_->settings().matchTime);
	if (matchEnded_ == false && paused_ == false && matchTimer_.secondsSince() > matchTimeFloat)
		endMatch();

	if (paused_ || matchEnded_)
		return;

	destroyDeadBubbles();
	spawnBubbles();
	Body::Collisions.clear();

	const unsigned int subSteps = 16;
	const float subStepLength = deltaTime / static_cast<float>(subSteps);

	for (unsigned int subStep = 0; subStep < subSteps; subStep++)
	{
		// Integrate all physics bodies
		for (Body *body : Body::All)
			body->integrate(subStepLength);

		// Resolve collisions
		const unsigned int len = Body::All.size();
		for (unsigned int i = 0; i < len; i++)
		{
			Body *bA = Body::All[i];

			for (unsigned int j = i + 1; j < len; j++)
			{
				Body *bB = Body::All[j];

				if (bA->bodyKind() == BodyKind::STATIC && bB->bodyKind() == BodyKind::STATIC)
					continue;

				if (bA->colliderKind() == ColliderKind::CIRCLE && bB->colliderKind() == ColliderKind::CIRCLE)
					Body::circleVsCircleCollision(bA, bB);
				else if (bA->colliderKind() == ColliderKind::CIRCLE && bB->colliderKind() == ColliderKind::AABB)
					Body::circleVsAabbCollision(bA, bB);
				else if (bA->colliderKind() == ColliderKind::AABB && bB->colliderKind() == ColliderKind::CIRCLE)
					Body::circleVsAabbCollision(bB, bA);
				else
					LOGW_X("Unhandled: %s vs %s", bA->colliderKindName(), bB->colliderKindName());
			}
		}
	}

	// Stamina bar sprite for player A
	nc::Recti redRect = redBar_->texRect();
	redRect.w *= playerA_->stamina();
	redRect.x += redBar_->texRect().w - redRect.w;
	redBarFill_->setTexRect(redRect);
	nc::Vector2f redPos = redBar_->absPosition();
	redPos.x += (redBar_->width() * (1.0f - playerA_->stamina())) * 0.5f;
	redBarFill_->setPosition(redPos);

	auxString.format("%d", playerA_->points());
	pointsAText_->setString(auxString);

	if (eventHandler_->settings().numPlayers == 2)
	{
		// Stamina bar sprite for player B
		nc::Recti blueRect = blueBar_->texRect();
		blueRect.w *= playerB_->stamina();
		blueBarFill_->setTexRect(blueRect);
		nc::Vector2f bluePos = blueBar_->absPosition();
		bluePos.x -= (blueBar_->width() * (1.0f - playerB_->stamina())) * 0.5f;
		blueBarFill_->setPosition(bluePos);

		auxString.format("%d", playerB_->points());
		pointsBText_->setString(auxString);
	}

	const float secondsLeft = static_cast<float>(eventHandler_->settings().matchTime) - matchTimer_.secondsSince();
	auxString.format("%d", static_cast<int>(secondsLeft));
	timeText_->setString(auxString);
}

void Game::drawGui()
{
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	if (ImGui::Button("Return To Menu"))
		eventHandler_->requestMenu();
	if (ImGui::Button("Quit"))
		nc::theApplication().quit();

	if (ImGui::TreeNode("Settings"))
	{
		const Settings &settings = eventHandler_->settings();
		ImGui::Text("Volume: %.1f", settings.volume);
		ImGui::Text("Number of players: %d", settings.numPlayers);
		ImGui::Text("Match time: %d", settings.matchTime);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Statistics"))
	{
		ImGui::Text("Bubbles dropped: %d", statistics_.numDroppedBubles);

		ImGui::NewLine();
		const PlayerStatistics &statisticsA = playerA_->statistics();
		ImGui::Text("Jumps P1: %d", statisticsA.numJumps);
		ImGui::Text("Double jumps P1: %d", statisticsA.numDoubleJumps);
		ImGui::Text("Dashes P1: %d", statisticsA.numDashes);
		ImGui::Text("Bubbles catched P1: %d", statisticsA.numCatchedBubbles);

		if (playerB_ != nullptr)
		{
			ImGui::NewLine();
			const PlayerStatistics &statisticsB = playerB_->statistics();
			ImGui::Text("Jumps P2: %d", statisticsB.numJumps);
			ImGui::Text("Double jumps P2: %d", statisticsB.numDoubleJumps);
			ImGui::Text("Dashes P2: %d", statisticsB.numDashes);
			ImGui::Text("Bubbles catched P2: %d", statisticsB.numCatchedBubbles);
		}

		ImGui::TreePop();
	}

	if (paused_)
	{
		ImGui::NewLine();
		menuPage_->drawGui();
		ImGui::NewLine();
	}
	else
	{
		const float secondsLeft = static_cast<float>(eventHandler_->settings().matchTime) - matchTimer_.secondsSince();
		ImGui::Text("Time left: %d", static_cast<int>(secondsLeft));
		ImGui::SameLine();
		if (ImGui::Button("Reset##Time"))
			matchTimer_.toNow();
	}

	playerA_->drawGui();
	if (playerB_ != nullptr)
		playerB_->drawGui();

	if (ImGui::TreeNode("Collisions", "Collisions: %d", Body::Collisions.size()))
	{
		for (unsigned int i = 0; i < Body::Collisions.size(); i++)
		{
			const CollisionPair &pair = Body::Collisions[i];
			ImGui::BulletText("Collision #%d - %s vs %s: %s (%d) and %s (%d), <%.2f, %.2f>",
			                  i, pair.a->colliderKindName(), pair.b->colliderKindName(),
			                  pair.a->name(), pair.a->bodyId(), pair.b->name(), pair.b->bodyId(),
			                  pair.normal.x, pair.normal.y);
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Bubbles"))
	{
		for (unsigned int i = 0; i < bubbles_.size(); i++)
			bubbles_[i]->drawGui(i);
		ImGui::TreePop();
	}
#endif
}

void Game::onQuitRequest()
{
	if (paused_ == false && matchEnded_ == false)
		togglePause();

	if (paused_)
		menuPage_->setup(quitConfirmationPausePage_);
	else if (matchEnded_)
		menuPage_->setup(quitConfirmationEndMatchPage_);
}

void Game::playSound()
{
	FATAL_ASSERT(gamePtr != nullptr);
	gamePtr->playPoppingSound();
}

void Game::incrementDroppedBubble()
{
	FATAL_ASSERT(gamePtr != nullptr);
	gamePtr->statistics_.numDroppedBubles++;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

/// Loads the main scene of the game
void Game::loadScene()
{
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
	darkForeground_->setLayer(Cfg::Layers::Menu_Page - 128);
	darkForeground_->setEnabled(false);

	const nctl::String fontFntPath_ = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Fonts::Modak50Fnt);
	const nctl::String fontTexPath_ = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Fonts::Modak50Png);
	font_ = nctl::makeUnique<nc::Font>(fontFntPath_.data(), resourceManager().retrieveTexture(Cfg::Fonts::Modak50Png));

	redBar_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::RedBar));
	redBarFill_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::RedBarFill));

	redBar_->setLayer(Cfg::Layers::Gui_StaminaBar);
	redBar_->setPosition(screenTopRight * Cfg::Gui::RedBarRelativePos);
	redBar_->setScale(Cfg::Gui::BarScale);
	redBarFill_->setLayer(Cfg::Layers::Gui_StaminaBar_Fill);
	redBarFill_->setPosition(screenTopRight * Cfg::Gui::RedBarRelativePos);
	redBarFill_->setScale(Cfg::Gui::BarScale);

	pointsAText_ = nctl::makeUnique<nc::TextNode>(this, font_.get(), 256);
	pointsAText_->setLayer(Cfg::Layers::Gui_Text);
	pointsAText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	pointsAText_->setString("0");
	pointsAText_->setPosition((screenTopRight - pointsAText_->absSize() * 0.5f) * Cfg::Gui::PointsATextRelativePos);

	playerA_ = nctl::makeUnique<Player>(this, "Player A", 0);

	if (eventHandler_->settings().numPlayers == 2)
	{
		blueBar_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::BlueBar));
		blueBarFill_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::BlueBarFill));

		blueBar_->setLayer(Cfg::Layers::Gui_StaminaBar);
		blueBar_->setPosition(screenTopRight * Cfg::Gui::BlueBarRelativePos);
		blueBar_->setScale(Cfg::Gui::BarScale);
		blueBarFill_->setLayer(Cfg::Layers::Gui_StaminaBar_Fill);
		blueBarFill_->setPosition(screenTopRight * Cfg::Gui::BlueBarRelativePos);
		blueBarFill_->setScale(Cfg::Gui::BarScale);

		pointsBText_ = nctl::makeUnique<nc::TextNode>(this, font_.get(), 256);
		pointsBText_->setLayer(Cfg::Layers::Gui_Text);
		pointsBText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
		pointsBText_->setString("0");
		pointsBText_->setPosition((screenTopRight - pointsBText_->absSize() * 0.5f) * Cfg::Gui::PointsBTextRelativePos);

		playerB_ = nctl::makeUnique<Player>(this, "Player B", 1);
	}

	timeText_ = nctl::makeUnique<nc::TextNode>(this, font_.get(), 256);
	timeText_->setLayer(Cfg::Layers::Gui_Text);
	timeText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	auxString.format("%d", eventHandler_->settings().matchTime);
	timeText_->setString(auxString);
	timeText_->setPosition((screenTopRight - timeText_->absSize() * 0.5f) * Cfg::Gui::TimeTextRelativePos);

	for (unsigned int i = 0; i < Cfg::Sounds::NumBubblePopPlayers; i++)
	{
		nctl::UniquePtr<nc::AudioBufferPlayer> poppingPlayer =
		    nctl::makeUnique<nc::AudioBufferPlayer>(resourceManager().retrieveAudioBuffer(Cfg::Sounds::BubblePops[i % Cfg::Sounds::NumBubblePops]));
		poppingPlayers_.pushBack(nctl::move(poppingPlayer));
	}

	// Floor
	obstacle1_ = nctl::makeUnique<Body>(this, "Floor", ColliderKind::AABB, BodyKind::STATIC, BodyId::STATIC);
	obstacle1_->setPosition(screenWidth * 0.5f, 0.0f);
	obstacle1_->colliderHalfSize_ = nc::Vector2f(4096.0f, Cfg::Game::FloorHeight);
	obstacle1Gfx_ = nctl::makeUnique<nc::Sprite>(obstacle1_.get(), nullptr);
	obstacle1Gfx_->setSize(obstacle1_->colliderHalfSize_ * 2.0f);
	obstacle1Gfx_->setAlphaF(0.3f);

	// Left limit
	obstacle2_ = nctl::makeUnique<Body>(this, "Obstacle", ColliderKind::AABB, BodyKind::STATIC, BodyId::STATIC);
	obstacle2_->setPosition(0.0f, 0.0f);
	obstacle2_->colliderHalfSize_ = nc::Vector2f(Cfg::Game::FloorHeight, 4096.0f);
#if NCPROJECT_DEBUG
	obstacle2Gfx_ = nctl::makeUnique<nc::Sprite>(obstacle2_.get(), nullptr);
	obstacle2Gfx_->setSize(obstacle2_->colliderHalfSize_ * 2.0f);
	obstacle2Gfx_->setAlphaF(0.2f);
#endif

	// Right limit
	obstacle3_ = nctl::makeUnique<Body>(this, "Obstacle", ColliderKind::AABB, BodyKind::STATIC, BodyId::STATIC);
	obstacle3_->setPosition(screenWidth, screenHeight);
	obstacle3_->colliderHalfSize_ = nc::Vector2f(Cfg::Game::FloorHeight, 4096.0f);
#if NCPROJECT_DEBUG
	obstacle3Gfx_ = nctl::makeUnique<nc::Sprite>(obstacle3_.get(), nullptr);
	obstacle3Gfx_->setSize(obstacle3_->colliderHalfSize_ * 2.0f);
	obstacle3Gfx_->setAlphaF(0.2f);
#endif

	menuPage_ = nctl::makeUnique<MenuPage>(this, "MenuPage");
	menuPagePtr = menuPage_.get();
	menuPage_->setPosition(screenTopRight * Cfg::Menu::MenuPageRelativePos);
	menuPage_->setEnabled(false);
	setupPages();
}

void Game::spawnBubbles()
{
	const unsigned int aliveCount = bubbles_.size();
	const unsigned int spawnTarget = Cfg::Game::NumBubbleForSpawnPerPlayer * eventHandler_->settings().numPlayers;

	if (aliveCount < spawnTarget)
		spawnBubble();
}

void Game::spawnBubble()
{
	const float screenWidth = nc::theApplication().gfxDevice().width();
	const float screenHeight = nc::theApplication().gfxDevice().height();

	const nc::Vector2f pos = nc::Vector2f(lerp(screenWidth * 0.1f, screenWidth - screenWidth * 0.1f, nc::random().real()),
	                                      screenHeight + lerp(screenHeight * 0.2f, screenHeight * 1.0f, nc::random().real()));

	const unsigned int variant = nc::random().integer(0, Cfg::Textures::NumBubbleVariants);
	nctl::UniquePtr<Bubble> bubble = nctl::makeUnique<Bubble>(this, "Bubble", pos, variant);
	bubbles_.pushBack(nctl::move(bubble));
}

void Game::destroyDeadBubbles()
{
	// Destroy all dead nodes from last frame
	for (const Bubble *it : Bubble::dead)
	{
		for (int i = bubbles_.size() - 1; i >= 0; i--)
		{
			if (bubbles_[i].get() == it)
			{
				bubbles_.unorderedRemoveAt(i);
				break;
			}
		}
	}
	Bubble::dead.clear();
}

void Game::playPoppingSound()
{
	const unsigned int playerIndex = nc::random().fastInteger(0, Cfg::Sounds::NumBubblePopPlayers);
	FATAL_ASSERT(poppingPlayers_.size() > playerIndex);
	nc::AudioBufferPlayer &player = *poppingPlayers_[playerIndex];

	// Random pitch variation for every time a popping sound is played
	const float pitch = nc::random().fastReal(Cfg::Sounds::MinSoundPitchBubble, Cfg::Sounds::MaxSoundPitchBubble);
	player.setPitch(pitch);

	player.play();
}

void Game::togglePause()
{
	// The following code allows the `GAME_PAUSE` action key to be shared with the `UI_BACK` one
	static unsigned int lastToggleFrame = 0;
	if (lastToggleFrame == nc::theApplication().numFrames())
		return;
	lastToggleFrame = nc::theApplication().numFrames();

	paused_ = !paused_;
	playerA_->setUpdateEnabled(!paused_);
	if (playerB_ != nullptr)
		playerB_->setUpdateEnabled(!paused_);

	menuPage_->setup(pausePage_);
	menuPage_->setEnabled(paused_);
	darkForeground_->setEnabled(paused_);

	nc::IAudioDevice &audioDevice = nc::theServiceLocator().audioDevice();
	if (paused_)
	{
		pauseTime_ = nc::TimeStamp::now();
		audioDevice.pauseDevice();
	}
	else
	{
		audioDevice.resumeDevice();
		matchTimer_ += (nc::TimeStamp::now() - pauseTime_);
	}
}

void Game::endMatch()
{
	saveStatistics();

	matchEnded_ = true;
	playerA_->setUpdateEnabled(false);
	if (playerB_ != nullptr)
		playerB_->setUpdateEnabled(false);

	menuPage_->setup(endMatchPage_);
	menuPage_->setEnabled(true);
	darkForeground_->setEnabled(true);
}

void Game::saveStatistics()
{
	Statistics &statistics = eventHandler_->statisticsMut();
	statistics.playTime += eventHandler_->settings().matchTime;
	statistics.numMatches++;
	statistics.numDroppedBubles += statistics_.numDroppedBubles;

	const PlayerStatistics &statisticsA = playerA_->statistics();
	statistics.playerStats[0].numCatchedBubbles += statisticsA.numCatchedBubbles;
	statistics.playerStats[0].numJumps += statisticsA.numJumps;
	statistics.playerStats[0].numDoubleJumps += statisticsA.numDoubleJumps;
	statistics.playerStats[0].numDashes += statisticsA.numDashes;

	if (playerB_ != nullptr)
	{
		const PlayerStatistics &statisticsB = playerB_->statistics();
		statistics.playerStats[1].numCatchedBubbles += statisticsB.numCatchedBubbles;
		statistics.playerStats[1].numJumps += statisticsB.numJumps;
		statistics.playerStats[1].numDoubleJumps += statisticsB.numDoubleJumps;
		statistics.playerStats[1].numDashes += statisticsB.numDashes;
	}
}

void Game::setupPages()
{
	const unsigned char selectEventMask = (1 << MenuPage::PageEntry::SelectBitPos);
	const MenuPage::PageEntry::EventReplyBitsT selectEventReplyBits = MenuPage::PageEntry::EventReplyBitsT(selectEventMask);

	const unsigned char textEventMask = (1 << MenuPage::PageEntry::TextBitPos);
	const MenuPage::PageEntry::EventReplyBitsT textEventReplyBits = MenuPage::PageEntry::EventReplyBitsT(textEventMask);

	// Pause menu page
	{
		MenuPage::PageEntry resumeEntry("Resume", reinterpret_cast<void *>(SimpleSelectEntry::RESUME_GAME), Game::simpleSelectFunc, selectEventReplyBits);
		MenuPage::PageEntry menuEntry("Main Menu", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_MAIN_MENU), Game::simpleSelectFunc, selectEventReplyBits);
		MenuPage::PageEntry quitEntry("Quit", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_QUIT_PAGE_FROM_PAUSE), Game::simpleSelectFunc, selectEventReplyBits);

		pausePage_ = {}; // clear the static variable
		pausePage_.entries.pushBack(resumeEntry);
		pausePage_.entries.pushBack(menuEntry);
		pausePage_.entries.pushBack(quitEntry);
		pausePage_.title = "Pause";
		pausePage_.backFunc = Game::resumeGame;
	}

	// End match menu page
	{
		MenuPage::PageEntry numBubblesEntry("Bubbles", reinterpret_cast<void *>(StatisticsTextEntry::NUM_BUBBLES), Game::statisticsTextFunc, textEventReplyBits);
		MenuPage::PageEntry numCatchedBubblesEntry("Catched Bubbles", reinterpret_cast<void *>(StatisticsTextEntry::NUM_CATCHED_BUBBLES), Game::statisticsTextFunc, textEventReplyBits);
		MenuPage::PageEntry numJumpsEntry("Jumps", reinterpret_cast<void *>(StatisticsTextEntry::NUM_JUMPS), Game::statisticsTextFunc, textEventReplyBits);
		MenuPage::PageEntry numDoubleJumpsEntry("Double Jumps", reinterpret_cast<void *>(StatisticsTextEntry::NUM_DOUBLE_JUMPS), Game::statisticsTextFunc, textEventReplyBits);
		MenuPage::PageEntry numDashesEntry("Dashes", reinterpret_cast<void *>(StatisticsTextEntry::NUM_DASHES), Game::statisticsTextFunc, textEventReplyBits);
		MenuPage::PageEntry menuEntry("Main Menu", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_MAIN_MENU), Game::simpleSelectFunc, selectEventReplyBits);
		MenuPage::PageEntry quitEntry("Quit", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_QUIT_PAGE_FROM_ENDMATCH), Game::simpleSelectFunc, selectEventReplyBits);

		endMatchPage_ = {}; // clear the static variable
		endMatchPage_.entries.pushBack(numBubblesEntry);
		endMatchPage_.entries.pushBack(numCatchedBubblesEntry);
		endMatchPage_.entries.pushBack(numJumpsEntry);
		endMatchPage_.entries.pushBack(numDoubleJumpsEntry);
		endMatchPage_.entries.pushBack(numDashesEntry);
		endMatchPage_.entries.pushBack(menuEntry);
		endMatchPage_.entries.pushBack(quitEntry);
		endMatchPage_.title = "End Match";
		endMatchPage_.backFunc = Game::goToMainMenu;
	}

	// Quit confirmation page (from pause)
	{
		MenuPage::PageEntry noEntry("No", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_PAUSE_PAGE), Game::simpleSelectFunc, selectEventReplyBits);
		MenuPage::PageEntry yesEntry("Yes", reinterpret_cast<void *>(SimpleSelectEntry::QUIT), Game::simpleSelectFunc, selectEventReplyBits);

		quitConfirmationPausePage_ = {}; // clear the static variable
		quitConfirmationPausePage_.entries.pushBack(noEntry);
		quitConfirmationPausePage_.entries.pushBack(yesEntry);
		quitConfirmationPausePage_.title = "Are you sure you want to quit?";
		quitConfirmationPausePage_.backFunc = Game::goToPausePage;
	}

	// Quit confirmation page (from end match)
	{
		MenuPage::PageEntry noEntry("No", reinterpret_cast<void *>(SimpleSelectEntry::GOTO_END_MATCH_PAGE), Game::simpleSelectFunc, selectEventReplyBits);
		MenuPage::PageEntry yesEntry("Yes", reinterpret_cast<void *>(SimpleSelectEntry::QUIT), Game::simpleSelectFunc, selectEventReplyBits);

		quitConfirmationEndMatchPage_ = {}; // clear the static variable
		quitConfirmationEndMatchPage_.entries.pushBack(noEntry);
		quitConfirmationEndMatchPage_.entries.pushBack(yesEntry);
		quitConfirmationEndMatchPage_.title = "Are you sure you want to quit?";
		quitConfirmationEndMatchPage_.backFunc = Game::goToEndMatchPage;
	}
}

void Game::goToPausePage()
{
	FATAL_ASSERT(menuPagePtr != nullptr);
	ASSERT(gamePtr->paused_ == true);
	menuPagePtr->setup(gamePtr->pausePage_);
}

void Game::goToEndMatchPage()
{
	FATAL_ASSERT(menuPagePtr != nullptr);
	menuPagePtr->setup(gamePtr->endMatchPage_);
}

void Game::goToMainMenu()
{
	FATAL_ASSERT(gamePtr != nullptr);
	gamePtr->eventHandler_->requestMenu();
}

void Game::resumeGame()
{
	FATAL_ASSERT(gamePtr != nullptr);
	ASSERT(gamePtr->paused_ == true);
	gamePtr->togglePause();
}

void Game::simpleSelectFunc(MenuPage::EntryEvent &event)
{
	if (event.type != MenuPage::EventType::SELECT)
		return;

	FATAL_ASSERT(gamePtr != nullptr);
	FATAL_ASSERT(menuPagePtr != nullptr);
	const SimpleSelectEntry entry = static_cast<SimpleSelectEntry>(reinterpret_cast<uintptr_t>(event.entryData));

	switch (entry)
	{
		case RESUME_GAME:
			ASSERT(gamePtr->paused_ == true);
			gamePtr->togglePause();
			break;
		case GOTO_MAIN_MENU:
			gamePtr->goToMainMenu();
			break;
		case GOTO_QUIT_PAGE_FROM_PAUSE:
			menuPagePtr->setup(gamePtr->quitConfirmationPausePage_);
			break;
		case GOTO_QUIT_PAGE_FROM_ENDMATCH:
			menuPagePtr->setup(gamePtr->quitConfirmationEndMatchPage_);
			break;
		case GOTO_PAUSE_PAGE:
			menuPagePtr->setup(gamePtr->pausePage_);
			break;
		case GOTO_END_MATCH_PAGE:
			menuPagePtr->setup(gamePtr->endMatchPage_);
			break;
		case QUIT:
			nc::theApplication().quit();
			break;
		default:
			break;
	}
}

void Game::statisticsTextFunc(MenuPage::EntryEvent &event)
{
	if (event.type != MenuPage::EventType::TEXT)
		return;

	FATAL_ASSERT(gamePtr != nullptr);
	const StatisticsTextEntry entry = static_cast<StatisticsTextEntry>(reinterpret_cast<uintptr_t>(event.entryData));
	const Statistics &statistics = gamePtr->statistics_;
	const PlayerStatistics &statisticsA = gamePtr->playerA_->statistics();

	if (gamePtr->playerB_ != nullptr)
	{
		const PlayerStatistics &statisticsB = gamePtr->playerB_->statistics();
		const unsigned int numCatchedBubbles = statisticsA.numCatchedBubbles + statisticsB.numCatchedBubbles;

		switch (entry)
		{
			case NUM_BUBBLES:
				event.entryText.format("Bubbles: %d catched, %d dropped", numCatchedBubbles, statistics.numDroppedBubles);
				break;
			case NUM_CATCHED_BUBBLES:
				event.entryText.format("Catched Bubbles: P1 %d / P2 %d", statisticsA.numCatchedBubbles, statisticsB.numCatchedBubbles);
				break;
			case NUM_JUMPS:
				event.entryText.format("Jumps: P1 %d / P2 %d", statisticsA.numJumps, statisticsB.numJumps);
				break;
			case NUM_DOUBLE_JUMPS:
				event.entryText.format("Double Jumps: P1 %d / P2 %d", statisticsA.numDoubleJumps, statisticsB.numDoubleJumps);
				break;
			case NUM_DASHES:
				event.entryText.format("Dashes: P1 %d / P2 %d", statisticsA.numDashes, statisticsB.numDashes);
				break;
			default:
				break;
		}
	}
	else
	{
		switch (entry)
		{
			case NUM_BUBBLES:
				event.entryText.format("Dropped Bubbles: %d", statistics.numDroppedBubles);
				break;
			case NUM_CATCHED_BUBBLES:
				event.entryText.format("Catched Bubbles: %d", statisticsA.numCatchedBubbles);
				break;
			case NUM_JUMPS:
				event.entryText.format("Jumps: %d", statisticsA.numJumps);
				break;
			case NUM_DOUBLE_JUMPS:
				event.entryText.format("Double Jumps: %d", statisticsA.numDoubleJumps);
				break;
			case NUM_DASHES:
				event.entryText.format("Dashes: %d", statisticsA.numDashes);
				break;
			default:
				break;
		}
	}
}
