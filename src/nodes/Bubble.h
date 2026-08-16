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
	Bubble(nc::SceneNode *parent, nctl::String name, nc::Vector2f pos, unsigned int variant, unsigned int poolIndex);

	void onTick(float deltaTime) override;
	void touched();
	void drawGui(unsigned int index);

	void onSpawn();
	void onKilled();

	unsigned int variant() const;
	/// A fixed index for the object's whole lifetime, used as a stable slot to bind a shader to
	unsigned int poolIndex() const;
	Body *body();
	nc::Sprite *sprite();

  private:
	unsigned int variant_;
	unsigned int poolIndex_;
	nctl::UniquePtr<Body> body_;
	nctl::UniquePtr<nc::Sprite> sprite_;
};
