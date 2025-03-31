#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "Game.h"
#include "Player.h"
#include "Bubble.h"
#include "Body.h"
#include "../ResourceManager.h"
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
	nctl::String auxString(256);
}

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

Game::Game(SceneNode *parent, nctl::String name, MyEventHandler *eventHandler)
    : LogicNode(parent, name), eventHandler_(eventHandler)
{
	gamePtr = this;
	loadScene();
}

Game::~Game()
{
	gamePtr = nullptr;
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Game::onTick(float deltaTime)
{
	if (matchTimer_.secondsSince() > static_cast<float>(Cfg::Game::MatchTime))
		eventHandler_->requestMenu();

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

	// Stamina bar sprite for player B
	nc::Recti blueRect = blueBar_->texRect();
	blueRect.w *= playerB_->stamina();
	blueBarFill_->setTexRect(blueRect);
	nc::Vector2f bluePos = blueBar_->absPosition();
	bluePos.x -= (blueBar_->width() * (1.0f - playerB_->stamina())) * 0.5f;
	blueBarFill_->setPosition(bluePos);

	auxString.format("%d", playerA_->points());
	pointsAText_->setString(auxString);
	auxString.format("%d", playerB_->points());
	pointsBText_->setString(auxString);

	const float secondsLeft = static_cast<float>(Cfg::Game::MatchTime) - matchTimer_.secondsSince();
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

	const float secondsLeft = static_cast<float>(Cfg::Game::MatchTime) - matchTimer_.secondsSince();
	ImGui::Text("Time left: %d", static_cast<int>(secondsLeft));
	ImGui::SameLine();
	if (ImGui::Button("Reset##Time"))
		matchTimer_.toNow();

	playerA_->drawGui();
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

void Game::playSound()
{
	FATAL_ASSERT(gamePtr != nullptr);
	gamePtr->playPoppingSound();
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

	redBar_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::RedBar));
	redBarFill_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::RedBarFill));
	blueBar_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::BlueBar));
	blueBarFill_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::BlueBarFill));

	redBar_->setLayer(Cfg::Layers::Gui_StaminaBar);
	redBar_->setPosition(screenTopRight * Cfg::Gui::RedBarRelativePos);
	redBar_->setScale(Cfg::Gui::BarScale);
	redBarFill_->setLayer(Cfg::Layers::Gui_StaminaBar_Fill);
	redBarFill_->setPosition(screenTopRight * Cfg::Gui::RedBarRelativePos);
	redBarFill_->setScale(Cfg::Gui::BarScale);

	blueBar_->setLayer(Cfg::Layers::Gui_StaminaBar);
	blueBar_->setPosition(screenTopRight * Cfg::Gui::BlueBarRelativePos);
	blueBar_->setScale(Cfg::Gui::BarScale);
	blueBarFill_->setLayer(Cfg::Layers::Gui_StaminaBar_Fill);
	blueBarFill_->setPosition(screenTopRight * Cfg::Gui::BlueBarRelativePos);
	blueBarFill_->setScale(Cfg::Gui::BarScale);

	const nctl::String fontFntPath_ = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Fonts::Modak50Fnt);
	const nctl::String fontTexPath_ = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Fonts::Modak50Png);
	font_ = nctl::makeUnique<nc::Font>(fontFntPath_.data(), resourceManager().retrieveTexture(Cfg::Fonts::Modak50Png));

	timeText_ = nctl::makeUnique<nc::TextNode>(this, font_.get(), 256);
	timeText_->setLayer(Cfg::Layers::Gui_Text);
	timeText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	auxString.format("%d", Cfg::Game::MatchTime);
	timeText_->setString(auxString);
	timeText_->setPosition((screenTopRight - timeText_->absSize() * 0.5f) * Cfg::Gui::TimeTextRelativePos);

	pointsAText_ = nctl::makeUnique<nc::TextNode>(this, font_.get(), 256);
	pointsAText_->setLayer(Cfg::Layers::Gui_Text);
	pointsAText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	pointsAText_->setString("0");
	pointsAText_->setPosition((screenTopRight - pointsAText_->absSize() * 0.5f) * Cfg::Gui::PointsATextRelativePos);
	pointsBText_ = nctl::makeUnique<nc::TextNode>(this, font_.get(), 256);
	pointsBText_->setLayer(Cfg::Layers::Gui_Text);
	pointsBText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	pointsBText_->setString("0");
	pointsBText_->setPosition((screenTopRight - pointsBText_->absSize() * 0.5f) * Cfg::Gui::PointsBTextRelativePos);

	for (unsigned int i = 0; i < Cfg::Sounds::NumBubblePopPlayers; i++)
	{
		nctl::UniquePtr<nc::AudioBufferPlayer> poppingPlayer =
		    nctl::makeUnique<nc::AudioBufferPlayer>(resourceManager().retrieveAudioBuffer(Cfg::Sounds::BubblePops[i % Cfg::Sounds::NumBubblePops]));
		poppingPlayers_.pushBack(nctl::move(poppingPlayer));
	}

	playerA_ = nctl::makeUnique<Player>(this, "Player A", 0);
	playerB_ = nctl::makeUnique<Player>(this, "Player B", 1);

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
}

void Game::spawnBubbles()
{
	const unsigned int aliveCount = bubbles_.size();

	if (aliveCount < Cfg::Game::NumBubbleForSpawn)
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
