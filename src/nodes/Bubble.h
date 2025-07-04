#pragma once

#include "LogicNode.h"

namespace ncine {
	class Sprite;
}
class Body;

namespace nc = ncine;

class Bubble : public LogicNode
{
  public:
	Bubble(nc::SceneNode *parent, nctl::String name, nc::Vector2f pos, unsigned int variant);

	void onTick(float deltaTime) override;
	void touched();
	void drawGui(unsigned int index);

	void onSpawn();
	void onKilled();

	unsigned int variant() const;
	Body *body();
	nc::Sprite *sprite();

  private:
	unsigned int variant_;
	nctl::UniquePtr<Body> body_;
	nctl::UniquePtr<nc::Sprite> sprite_;
};
