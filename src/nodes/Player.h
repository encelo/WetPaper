#pragma once

#include "LogicNode.h"

namespace ncine {
	class AnimatedSprite;
}
class Body;
class Bubble;

namespace nc = ncine;

class Player : public LogicNode
{
  public:
	Player(SceneNode *parent, nctl::String name, int playerIndex);

	inline float stamina() const { return stamina_; }
	inline int points() const { return points_; }

	void onTick(float deltaTime) override;
	void drawGui();

  private:
	int index_;
	/// Normalised (0..1)
	float stamina_;
	int points_;

	nctl::UniquePtr<Body> body_;
	nctl::UniquePtr<nc::AnimatedSprite> sprite_;

	float dashEnergy_;
	nc::Vector2f dashDir_;

	int jumpCount_;

	void onBubbleTouched(Bubble *bubble);
};
