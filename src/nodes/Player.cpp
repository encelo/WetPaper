#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "Player.h"
#include "Body.h"
#include "Game.h"
#include "Bubble.h"
#include "../ResourceManager.h"
#include "../Config.h"

#include <ncine/Texture.h>
#include <ncine/Application.h>
#include <ncine/IInputManager.h>
#include <ncine/InputEvents.h>
#include <ncine/FileSystem.h>
#include <ncine/AnimatedSprite.h>
#include <ncine/RectAnimation.h>

namespace {
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	const ImVec4 Green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	const ImVec4 Red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	nctl::String auxString(256);
#endif
}

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

Player::Player(nc::SceneNode *parent, nctl::String name, int playerIndex)
    : LogicNode(parent, name),
      index_(playerIndex), stamina_(1.0f), points_(0),
      dashEnergy_(0.0f), dashDir_(0.0f, 0.0f), jumpCount_(0)
{
	// Setup the physics body
	{
		body_ = nctl::makeUnique<Body>(this, "Body", ColliderKind::CIRCLE, BodyKind::DYNAMIC, BodyId::PLAYER);

		if (playerIndex == 0)
			body_->setPosition(body_->colliderHalfSize_.x * 2.0f, body_->colliderHalfSize_.y * 2.1f);
		if (playerIndex == 1)
			body_->setPosition(nc::theApplication().gfxDevice().width() - body_->colliderHalfSize_.x * 2.0f, body_->colliderHalfSize_.y * 2.1f);

		body_->linearVelocityDamping_ = 0.01f;
		body_->maxVelocity_ = 2000.0f;
		body_->colliderHalfSize_ = nc::Vector2f(64.0f, 0.0f);
	}

	// Setup the sprite frames
	{
		const Cfg::SpriteData &spriteData = Cfg::Sprites::Players[playerIndex];

		nc::Texture *tex = resourceManager().retrieveTexture(spriteData.textureName);
		FATAL_ASSERT_MSG_X(tex != nullptr, "Cannot load texture \"%s\"!", spriteData.textureName);

		sprite_ = nctl::makeUnique<nc::AnimatedSprite>(body_.get(), tex);

		nc::RectAnimation animation(0.06f, nc::RectAnimation::LoopMode::ENABLED, nc::RectAnimation::RewindMode::FROM_START);
		animation.addRects(spriteData.frameSize, tex->rect());
		sprite_->addAnimation(nctl::move(animation));
		sprite_->setAnimationIndex(0);
		sprite_->setFrame(0);
		sprite_->setPaused(true);
		sprite_->setAbsAnchorPoint(spriteData.offset);
		sprite_->setScale(spriteData.scale);
		sprite_->setLayer(Cfg::Layers::Player);
	}
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Player::onTick(float deltaTime)
{
	// Compute the new movement direction
	{
		const nc::IInputManager &input = nc::theApplication().inputManager();
		const nc::KeyboardState &ks = input.keyboardState();

		bool leftDown = false;
		bool rightDown= false;
		bool jumpPressed = false;
		bool dashPressed = false;

		if (index_ == 0)
		{
			leftDown = ks.isKeyDown(nc::KeySym::A);
			rightDown = ks.isKeyDown(nc::KeySym::D);
			jumpPressed = ks.isKeyPressed(nc::KeySym::W);
			dashPressed = ks.isKeyPressed(nc::KeySym::S);
		}
		else
		{
			leftDown = ks.isKeyDown(nc::KeySym::J);
			rightDown = ks.isKeyDown(nc::KeySym::L);
			jumpPressed = ks.isKeyPressed(nc::KeySym::I);
			dashPressed = ks.isKeyPressed(nc::KeySym::K);
		}

		if (input.isJoyPresent(index_))
		{
			const nc::JoyMappedState &state = input.joyMappedState(index_);
			if (state.axisValue(nc::AxisName::LX) < -0.5f)
				leftDown = true;

			if (state.axisValue(nc::AxisName::LX) > 0.5f)
				rightDown = true;

			if (state.isButtonPressed(nc::ButtonName::A))
				jumpPressed = true;

			if (state.isButtonPressed(nc::ButtonName::B))
				dashPressed = true;
		}

		if (body_->isGrounded())
		{
			if (body_->linearVelocity_.y < 0.0f)
			{
				// We're going down and touching the floor, let's reset the jump count
				jumpCount_ = 0; // reset the jump count
			}

			body_->linearVelocityDamping_ = 0.2f; // drag active
			body_->gravity_ = nc::Vector2f::Zero; // no gravity

			if (leftDown)
			{
				if (body_->linearVelocity_.x > 0.0f)
					body_->linearVelocity_.x *= 0.8f;

				body_->linearVelocity_ += nc::Vector2f(-1.0f, 0.0f) * maxGroundMoveSpeed;
			}

			if (rightDown)
			{
				if (body_->linearVelocity_.x < 0.0f)
					body_->linearVelocity_.x *= 0.8f;

				body_->linearVelocity_ += nc::Vector2f(1.0f, 0.0f) * maxGroundMoveSpeed;
			}

			if (jumpPressed)
			{
				jumpCount_++;

				body_->linearVelocity_.y = 0.0f; // removing the Y component
				body_->linearVelocity_ += nc::Vector2f(0.0f, 1.0f) * jumpVel;
			}
		}
		else
		{
			body_->linearVelocityDamping_ = 1.0f; // no drag
			body_->gravity_ = nc::Vector2f(0.0f, -1250.0f); // normal gravity

			if (leftDown)
			{
				if (body_->linearVelocity_.x > 0.0f)
					body_->linearVelocity_.x *= 0.8f;

				body_->linearVelocity_ += nc::Vector2f(-1.0f, 0.0f) * maxAirMoveSpeed;
			}

			if (rightDown)
			{
				if (body_->linearVelocity_.x < 0.0f)
					body_->linearVelocity_.x *= 0.8f;

				body_->linearVelocity_ += nc::Vector2f(1.0f, 0.0f) * maxAirMoveSpeed;
			}

			if (jumpPressed && jumpCount_ < maxJumps)
			{
				jumpCount_++;

				body_->linearVelocity_.y = 0.0f; // removing the Y component
				body_->linearVelocity_ += nc::Vector2f(0.0f, 1.0f) * jumpVel;
			}
		}

		if (dashPressed && stamina_ >= dashStaminaCost)
		{
			stamina_ -= dashStaminaCost;
			dashEnergy_ = dashDuration;

			if (leftDown)
			{
				dashDir_ = nc::Vector2f(-1.0f, 0.0f);
				if (body_->linearVelocity_.x > 0)
					body_->linearVelocity_.x *= 0.5f;
			}
			else if (rightDown)
			{
				dashDir_ = nc::Vector2f(1.0f, 0.0f);
				if (body_->linearVelocity_.x < 0)
					body_->linearVelocity_.x *= 0.5f;
			}
			else
			{
				// no explicit direction, let's use current velocity
				dashDir_ = nc::Vector2f((body_->linearVelocity_.x <= 0.0f) ? -1.0f : 1.0f, 0.0f);
			}
		}

		if (dashEnergy_ > 0.0f)
		{
			dashEnergy_ -= deltaTime;
			body_->linearVelocity_ += dashDir_ * maxDashVel;
		}

		const float regen = (maxStamina / staminaRegenTime) * deltaTime;
		stamina_ = fminf(stamina_ + regen, maxStamina);
	}

	// check collisions with bubbles
	for (const CollisionPair &coll : Body::Collisions)
	{
		if (coll.a == body_.get() && coll.b->bodyId() == BodyId::BUBBLE)
		{
			Body *bubbleBody = coll.b;
			OnBubbleTouched(static_cast<Bubble *>(bubbleBody->parent()));
		}

		if (coll.b == body_.get() && coll.a->bodyId() == BodyId::BUBBLE)
		{
			Body *bubbleBody = coll.a;
			OnBubbleTouched(static_cast<Bubble *>(bubbleBody->parent()));
		}
	}

	const bool isIdle = (fabsf(body_->linearVelocity_.x) < 25.0f);

	if (body_->linearVelocity_.x < -10.0f)
		sprite_->setFlippedX(false);
	else if (body_->linearVelocity_.x > 10.0f)
		sprite_->setFlippedX(true);

	sprite_->setPaused(isIdle);
	if (isIdle)
		sprite_->setFrame(0);
}

void Player::drawGui()
{
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	auxString.format("Player %d", index_);
	if (ImGui::TreeNodeEx(auxString.data(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::ProgressBar(stamina_, ImVec2(0.0f, 0.0f), "Stamina");
		ImGui::Text("Points (%d): %d", index_, points_);
		ImGui::Text("Jumps (%d): %d", index_, jumpCount_);

		ImGui::TextUnformatted("Dashing: ");
		ImGui::SameLine();
		if (dashEnergy_ > 0.0f)
			ImGui::TextColored(Green, "yes");
		else
			ImGui::TextColored(Red, "no");

		ImGui::TextUnformatted("Grounded: ");
		ImGui::SameLine();
		if (body_->isGrounded())
			ImGui::TextColored(Green, "yes");
		else
			ImGui::TextColored(Red, "no");

		body_->drawGui();

		ImGui::TreePop();
	}
#endif
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void Player::OnBubbleTouched(Bubble *bubble)
{
	bubble->touched();
	points_++;
}
