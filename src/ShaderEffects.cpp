#include "ShaderEffects.h"
#include "nodes/Menu.h"
#include "nodes/Game.h"
#include <ncine/Application.h>
#include <ncine/Viewport.h>
#include <ncine/Shader.h>
#include <ncine/ShaderState.h>
#include <ncine/Texture.h>
#include <ncine/Sprite.h>

#include "shader_sources.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

ShaderEffects::ShaderEffects()
    : initialized_(false), currentViewportSetup_(ViewportSetup::NONE),
      numBlurPasses_(2), updateNode_(nullptr)
{
	initialized_ = initialize();
}

ShaderEffects::~ShaderEffects()
{
	if (initialized_)
		resetViewports();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void ShaderEffects::onDrawViewport(nc::Viewport &viewport)
{
	if (initialized_ == false)
		return;

	// Dirtying the uniform cache value at each blur pass
	if (&viewport == pingViewport_.get())
	{
		vpPingSpriteShaderState_->setUniformFloat(nullptr, "uDirection", 1.0f, 0.0f);
		vpPongSpriteShaderState_->setUniformFloat(nullptr, "uDirection", 0.0f, 1.0f);
	}

	if (&viewport == backViewport_.get() && updateNode_ != nullptr)
	{

		if (currentViewportSetup_ == ViewportSetup::MENU)
		{
			Menu *menuPtr = reinterpret_cast<Menu *>(updateNode_);
			menuPtr->onTick(nc::theApplication().frameTime());
		}
		else if (currentViewportSetup_ == ViewportSetup::GAME)
		{
			Game *gamePtr = reinterpret_cast<Game *>(updateNode_);
			gamePtr->onTick(nc::theApplication().frameTime());
		}
	}
}

void ShaderEffects::setupMenuViewports(nc::SceneNode *menuNode, nc::SceneNode *backgroundNode, nc::SceneNode *sceneNode, nc::SceneNode *foregroundNode)
{
	if (initialized_ == false || currentViewportSetup_ == ViewportSetup::MENU)
		return;

	updateNode_ = menuNode;
	if (backgroundNode == nullptr)
		backgroundNode = &nc::theApplication().rootNode();
	backViewport_->setRootNode(backgroundNode);

	if (sceneNode == nullptr)
		sceneNode = &nc::theApplication().rootNode();
	sceneViewport_->setRootNode(sceneNode);

	if (foregroundNode == nullptr)
		foregroundNode = &nc::theApplication().rootNode();
	frontViewport_->setRootNode(foregroundNode);

	currentViewportSetup_ = ViewportSetup::MENU;
	nc::Viewport::chain().clear();

	nc::theApplication().screenViewport().setRootNode(screenSprite_.get());

	nc::Viewport::chain().pushBack(blendingViewportFront_.get());
	nc::Viewport::chain().pushBack(frontViewport_.get());

	nc::Viewport::chain().pushBack(blendingViewportBack_.get());
	nc::Viewport::chain().pushBack(sceneViewport_.get());
	nc::Viewport::chain().pushBack(backViewport_.get());
}

void ShaderEffects::setupGameViewports(nc::SceneNode *gameNode, nc::SceneNode *backgroundNode, nc::SceneNode *sceneNode, nc::SceneNode *foregroundNode)
{
	if (initialized_ == false || currentViewportSetup_ == ViewportSetup::GAME)
		return;

	updateNode_ = gameNode;
	if (backgroundNode == nullptr)
		backgroundNode = &nc::theApplication().rootNode();
	backViewport_->setRootNode(backgroundNode);

	if (sceneNode == nullptr)
		sceneNode = &nc::theApplication().rootNode();
	sceneViewport_->setRootNode(sceneNode);

	if (foregroundNode == nullptr)
		foregroundNode = &nc::theApplication().rootNode();
	frontViewport_->setRootNode(foregroundNode);

	currentViewportSetup_ = ViewportSetup::GAME;
	setupGameViewportsPause(false);
}

void ShaderEffects::setupGameViewportsPause(bool paused)
{
	// The viewport should already be setup for the game
	if (initialized_ == false || currentViewportSetup_ != ViewportSetup::GAME)
		return;

	nc::Viewport::chain().clear();

	nc::theApplication().screenViewport().setRootNode(screenSprite_.get());

	nc::Viewport::chain().pushBack(blendingViewportFront_.get());
	nc::Viewport::chain().pushBack(frontViewport_.get());

	if (paused)
	{
		// Ping-pong passes of the separable blur shader
		for (unsigned int i = 0; i < numBlurPasses_; i++)
		{
			nc::Viewport::chain().pushBack(pongViewport_.get());
			nc::Viewport::chain().pushBack(pingViewport_.get());
		}
	}

	nc::Viewport::chain().pushBack(blendingViewportBack_.get());
	nc::Viewport::chain().pushBack(sceneViewport_.get());
	nc::Viewport::chain().pushBack(backViewport_.get());
}

void ShaderEffects::resetViewports()
{
	if (initialized_ == false || currentViewportSetup_ == ViewportSetup::NONE)
		return;

	currentViewportSetup_ = ViewportSetup::NONE;
	nc::Viewport::chain().clear();

	nc::theApplication().screenViewport().setRootNode(&nc::theApplication().rootNode());
	updateNode_ = nullptr;
}

void ShaderEffects::setBubbleShader(nc::Sprite *sprite, unsigned int index)
{
	if (initialized_ == false || index >= Cfg::Game::BubblePoolSize)
		return;

	// Set a node first with `setNode()`, then its shader with `setShader()`
	vpDispersionShaderState_[index]->setNode(sprite);
	vpDispersionShaderState_[index]->setShader(vpDispersionShader_.get());
	vpDispersionShaderState_[index]->setUniformFloat(nullptr, "winResolution", static_cast<float>(nc::theApplication().width()), static_cast<float>(nc::theApplication().height()));

	// Storing old values before altering the sprite
	const nc::Texture *spriteTexture = sprite->texture();
	const nc::Vector2f spriteSize = sprite->absSize();

	// Set the sprite texture, then the texture rectangle, then its size
	sprite->setTexture(texture0_.get());
	sprite->setTexRect(nc::Recti(0, 0, texture0_->width(), texture0_->height()));
	sprite->setSize(spriteSize * 1.0f / sprite->absScale().x);

	vpDispersionShaderState_[index]->setTexture(0, texture0_.get()); // GL_TEXTURE0
	vpDispersionShaderState_[index]->setUniformInt(nullptr, "uTexture", 0); // GL_TEXTURE0
	vpDispersionShaderState_[index]->setTexture(1, spriteTexture); // GL_TEXTURE1
	vpDispersionShaderState_[index]->setUniformInt(nullptr, "uTexture1", 1); // GL_TEXTURE1
}

void ShaderEffects::clearBubbleShader(unsigned int index)
{
	if (initialized_ == false || index >= Cfg::Game::BubblePoolSize)
		return;

	// Remove a shader first with `setShader(nullptr)`, then the node with `setNode(nullptr)`
	vpDispersionShaderState_[index]->setShader(nullptr);
	vpDispersionShaderState_[index]->setNode(nullptr);
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

bool ShaderEffects::initialize()
{
	const bool compiled = compileShaders();
	if (compiled == false)
		return false;

	texture0_ = nctl::makeUnique<nc::Texture>("Ping texture", nc::Texture::Format::RGB8, nc::theApplication().resolutionInt());
	texture1_ = nctl::makeUnique<nc::Texture>("Pong texture", nc::Texture::Format::RGB8, nc::theApplication().resolutionInt());
	textureFront_ = nctl::makeUnique<nc::Texture>("Front texture", nc::Texture::Format::RGBA8, nc::theApplication().resolutionInt());
	textureExtra_ = nctl::makeUnique<nc::Texture>("Extra texture", nc::Texture::Format::RGBA8, nc::theApplication().resolutionInt());

	screenSprite_ = nctl::makeUnique<nc::Sprite>(nullptr, texture0_.get(), nc::theApplication().width() * 0.5f, nc::theApplication().height() * 0.5f);

	backViewport_ = nctl::makeUnique<nc::Viewport>(texture0_.get());
	sceneViewport_ = nctl::makeUnique<nc::Viewport>(textureExtra_.get());
	blendingViewportBack_ = nctl::makeUnique<nc::Viewport>(texture0_.get());
	pingViewport_ = nctl::makeUnique<nc::Viewport>(texture1_.get());
	pongViewport_ = nctl::makeUnique<nc::Viewport>(texture0_.get());
	frontViewport_ = nctl::makeUnique<nc::Viewport>(textureFront_.get());
	blendingViewportFront_ = nctl::makeUnique<nc::Viewport>(textureExtra_.get());

	sceneViewport_->setRootNode(&nc::theApplication().rootNode());
	sceneViewport_->setClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	vpBlendingSpriteBack_ = nctl::makeUnique<nc::Sprite>(nullptr, textureExtra_.get(), nc::theApplication().width() * 0.5f, nc::theApplication().height() * 0.5f);
	blendingViewportBack_->setTexture(texture0_.get());
	blendingViewportBack_->setRootNode(vpBlendingSpriteBack_.get());
	blendingViewportBack_->setClearMode(nc::Viewport::ClearMode::NEVER);

	vpPingSprite_ = nctl::makeUnique<nc::Sprite>(nullptr, texture0_.get(), nc::theApplication().width() * 0.5f, nc::theApplication().height() * 0.5f);
	pingViewport_->setRootNode(vpPingSprite_.get());
	vpPingSpriteShaderState_ = nctl::makeUnique<nc::ShaderState>(vpPingSprite_.get(), vpBlurShader_.get());
	vpPingSpriteShaderState_->setUniformFloat(nullptr, "uResolution", static_cast<float>(texture0_->width()), static_cast<float>(texture0_->height()));

	vpPongSprite_ = nctl::makeUnique<nc::Sprite>(nullptr, texture1_.get(), nc::theApplication().width() * 0.5f, nc::theApplication().height() * 0.5f);
	pongViewport_->setRootNode(vpPongSprite_.get());
	vpPongSpriteShaderState_ = nctl::makeUnique<nc::ShaderState>(vpPongSprite_.get(), vpBlurShader_.get());
	vpPongSpriteShaderState_->setUniformFloat(nullptr, "uResolution", static_cast<float>(texture1_->width()), static_cast<float>(texture1_->height()));

	frontViewport_->setRootNode(&nc::theApplication().rootNode());
	frontViewport_->setClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	vpBlendingSpriteFront_ = nctl::makeUnique<nc::Sprite>(nullptr, textureFront_.get(), nc::theApplication().width() * 0.5f, nc::theApplication().height() * 0.5f);
	blendingViewportFront_->setTexture(texture0_.get());
	blendingViewportFront_->setRootNode(vpBlendingSpriteFront_.get());
	blendingViewportFront_->setClearMode(nc::Viewport::ClearMode::NEVER);

	for (unsigned int i = 0; i < Cfg::Game::BubblePoolSize; i++)
		vpDispersionShaderState_[i] = nctl::makeUnique<nc::ShaderState>(nullptr, vpDispersionShader_.get());

	return true;
}

bool ShaderEffects::compileShaders()
{
	bool compiled = true;

	if (compiled == true)
	{
		vpBlurShader_ = nctl::makeUnique<nc::Shader>("SeparableBlur_Shader", nc::Shader::LoadMode::STRING, nc::Shader::DefaultVertex::SPRITE, sprite_blur_fs);
		ASSERT(vpBlurShader_->isLinked());
		compiled &= vpBlurShader_->isLinked();
	}

	if (compiled == true)
	{
		vpDispersionShader_ = nctl::makeUnique<nc::Shader>("Dispersion_Shader", nc::Shader::LoadMode::STRING, sprite_dispersion_vs, sprite_dispersion_fs);
		ASSERT(vpDispersionShader_->isLinked());
		compiled &= vpDispersionShader_->isLinked();
	}

	if (compiled == true)
	{
		vpBatchedDispersionShader_ = nctl::makeUnique<nc::Shader>("Batched_Dispersion_Shader", nc::Shader::LoadMode::STRING, nc::Shader::Introspection::NO_UNIFORMS_IN_BLOCKS, batched_sprite_dispersion_vs, sprite_dispersion_fs);
		FATAL_ASSERT(vpBatchedDispersionShader_->isLinked());
		vpDispersionShader_->registerBatchedShader(*vpBatchedDispersionShader_);
	}

	return compiled;
}
