#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(WETPAPER_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "Bubble.h"
#include "Game.h"
#include "Body.h"
#include "../ResourceManager.h"

#include <ncine/Texture.h>
#include <ncine/AnimatedSprite.h>
#include <ncine/RectAnimation.h>

nctl::Array<Bubble *> Bubble::dead;

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

Bubble::Bubble(nc::SceneNode *parent, nctl::String name, nc::Vector2f pos, unsigned int variant)
    : LogicNode(parent, name)
{
	// Setup the physics body
	{
		body_ = nctl::makeUnique<Body>(this, "Body", ColliderKind::CIRCLE, BodyKind::DYNAMIC, BodyId::BUBBLE);

		body_->setPosition(pos);
		body_->linearVelocityDamping_ = 1.0f;
		body_->maxVelocity_ = Cfg::Physics::BubbleMaxVelocity;
		body_->colliderHalfSize_.set(64.0f, 0.0f);
		body_->gravity_.set(0.0f, -100.0f);
	}

	// Setup the sprite
	{
		if (variant >= Cfg::Textures::NumBubbleVariants)
			variant = 0;

		nc::Texture *tex = resourceManager().retrieveTexture(Cfg::Textures::Bubbles[variant]);
		FATAL_ASSERT_MSG(tex != nullptr, "Cannot load texture!");

		sprite_ = nctl::makeUnique<nc::AnimatedSprite>(body_.get(), tex);

		nc::RectAnimation animation(0.12f, nc::RectAnimation::LoopMode::ENABLED, nc::RectAnimation::RewindMode::FROM_START);
		animation.addRects(nc::Vector2i(256, 256), tex->rect());
		sprite_->addAnimation(nctl::move(animation));
		sprite_->setAnimationIndex(0);
		sprite_->setFrame(0);
		sprite_->setPaused(true);
		sprite_->setScale(0.6f);
		sprite_->setLayer(Cfg::Layers::Bubble);
	}
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Bubble::onTick(float deltaTime)
{
	if (body_->isGrounded())
	{
		LOGI("Bubble touched ground");
		dead.pushBack(this);
		Game::playSound();
	}
}

void Bubble::touched()
{
	LOGI("Touched");
	dead.pushBack(this);
	Game::playSound();
}

void Bubble::drawGui(unsigned int index)
{
#if NCINE_WITH_IMGUI && defined(WETPAPER_DEBUG)
	ImGui::Text("Bubble #%d: <%0.1f, %0.1f>, Grounded: %s", index, body_->position().x, body_->position().y, body_->isGrounded() ? "yes" : "no");
	body_->drawGui();
#endif
}
