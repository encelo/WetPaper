#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "Bubble.h"
#include "Game.h"
#include "Body.h"
#include "../ResourceManager.h"

#include <ncine/Texture.h>
#include <ncine/Sprite.h>

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

Bubble::Bubble(nc::SceneNode *parent, nctl::String name, nc::Vector2f pos, unsigned int variant)
    : LogicNode(parent, name), variant_(variant)
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
		if (variant_ >= Cfg::Textures::NumBubbleVariants)
			variant_ = 0;

		nc::Texture *tex = resourceManager().retrieveTexture(Cfg::Textures::Bubbles[variant_]);
		FATAL_ASSERT_MSG(tex != nullptr, "Cannot load texture!");

		sprite_ = nctl::makeUnique<nc::Sprite>(body_.get(), tex);
		sprite_->setScale(0.6f);
		sprite_->setLayer(Cfg::Layers::Bubble);

		// Correct the collider radius based on sprite size
		body_->colliderHalfSize_.set(sprite_->width() * 0.5f, 0.0f);
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
		Game::killBubble(this);
		Game::incrementDroppedBubble();
		Game::playSound();
	}
}

void Bubble::touched()
{
	LOGI("Touched");
	Game::killBubble(this);
	Game::playSound();
}

void Bubble::drawGui(unsigned int index)
{
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	ImGui::Text("Bubble #%d: <%0.1f, %0.1f>, Grounded: %s", index, body_->position().x, body_->position().y, body_->isGrounded() ? "yes" : "no");
	body_->drawGui();
#endif
}

void Bubble::onSpawn()
{
	setEnabled(true);
	Body::All.pushBack(body_.get());
}

void Bubble::onKilled()
{
	setEnabled(false);
	body_->removeFromAll();
}

unsigned int Bubble::variant() const
{
	return variant_;
}

Body *Bubble::body()
{
	return body_.get();
}

nc::Sprite *Bubble::sprite()
{
	return sprite_.get();
}
