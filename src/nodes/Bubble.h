#pragma once

#include "LogicNode.h"

namespace ncine {
	class AnimatedSprite;
}
class Body;

namespace nc = ncine;

class Bubble : public LogicNode
{
  public:
	static nctl::Array<Bubble *> dead;

	Bubble(nc::SceneNode *parent, nctl::String name, nc::Vector2f pos, unsigned int variant);

	void onTick(float deltaTime) override;
	void touched();
	void drawGui(unsigned int index);

  private:
	nctl::UniquePtr<Body> body_;
	nctl::UniquePtr<nc::AnimatedSprite> sprite_;
};
