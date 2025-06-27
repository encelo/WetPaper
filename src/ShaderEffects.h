#pragma once

#include <nctl/UniquePtr.h>
#include "Config.h"

namespace ncine {
	class Viewport;
	class SceneNode;
	class Texture;
	class Shader;
	class ShaderState;
	class Sprite;
}

namespace nc = ncine;

class ShaderEffects
{
  public:
	enum class ViewportSetup
	{
		NONE,
		MENU,
		GAME
	};

	ShaderEffects();
	~ShaderEffects();

	inline bool isInitialized() const { return initialized_; }
	void onDrawViewport(nc::Viewport &viewport);

	void setupMenuViewports(nc::SceneNode *menuNode, nc::SceneNode *backgroundNode, nc::SceneNode *sceneNode, nc::SceneNode *foregroundNode);
	void setupGameViewports(nc::SceneNode *gameNode, nc::SceneNode *backgroundNode, nc::SceneNode *sceneNode, nc::SceneNode *foregroundNode);
	void setupGameViewportsPause(bool paused);
	void resetViewports();
	void setBubbleShader(nc::Sprite *sprite, unsigned int index);
	void clearBubbleShader(unsigned int index);

  private:
	bool initialized_;
	int numBlurPasses_;
	ViewportSetup currentViewportSetup_;

	nctl::UniquePtr<nc::Texture> texture0_;
	nctl::UniquePtr<nc::Texture> texture1_;
	nctl::UniquePtr<nc::Texture> textureFront_;
	nctl::UniquePtr<nc::Texture> textureExtra_;

	nctl::UniquePtr<nc::Viewport> backViewport_;
	nctl::UniquePtr<nc::Viewport> sceneViewport_;
	nctl::UniquePtr<nc::Viewport> blendingViewportBack_;
	nctl::UniquePtr<nc::Viewport> pingViewport_;
	nctl::UniquePtr<nc::Viewport> pongViewport_;
	nctl::UniquePtr<nc::Viewport> frontViewport_;
	nctl::UniquePtr<nc::Viewport> blendingViewportFront_;

	nctl::UniquePtr<nc::Sprite> screenSprite_;
	nctl::UniquePtr<nc::Sprite> vpBlendingSpriteBack_;
	nctl::UniquePtr<nc::Sprite> vpPingSprite_;
	nctl::UniquePtr<nc::Sprite> vpPongSprite_;
	nctl::UniquePtr<nc::Sprite> vpBlendingSpriteFront_;

	nctl::UniquePtr<nc::Shader> vpBlurShader_;
	nctl::UniquePtr<nc::ShaderState> vpPingSpriteShaderState_;
	nctl::UniquePtr<nc::ShaderState> vpPongSpriteShaderState_;

	nctl::UniquePtr<nc::Shader> vpDispersionShader_;
	nctl::UniquePtr<nc::Shader> vpBatchedDispersionShader_;
	nctl::UniquePtr<nc::ShaderState> vpDispersionShaderState_[Cfg::Game::BubblePoolSize];

	nc::SceneNode *updateNode_;

	bool initialize();
	bool compileShaders();
};
