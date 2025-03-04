#pragma once

#include <nctl/String.h>
#include <ncine/SceneNode.h>

namespace nc = ncine;

/// LogicNode is the base class for all nodes handling the game logic
class LogicNode : public nc::SceneNode
{
  public:
	LogicNode(nc::SceneNode *parent, nctl::String name);

	void update(float interval) override;
	void visit(nc::RenderQueue &renderQueue, unsigned int &visitOrderIndex) override;

  protected:
	virtual void onTick(float deltaTime) {}
	virtual void onPostTick(nc::RenderQueue &renderQueue, unsigned int &visitOrderIndex) {}
};

static inline float lerp(float v0, float v1, float t)
{
	return fmaf(t, v1, fmaf(-t, v0, v0));
}
