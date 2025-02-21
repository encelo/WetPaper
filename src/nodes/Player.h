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
	const float maxAirMoveSpeed = 10.0f;
	const float maxGroundMoveSpeed = 20.0f;
	const float jumpVel = 600.0f;
	const float maxDashVel = 100.0f;
	/// Max time (in seconds) for the dash
	const float dashDuration = 0.1f;
	const float maxStamina = 1.0f;
	const float dashStaminaCost = maxStamina * 0.5f;
	const float staminaRegenTime = 2.0f;
	const float maxJumps = 2;

	int index_;
	/// Normalised (0..1)
	float stamina_;
	int points_;

	nctl::UniquePtr<Body> body_;
	nctl::UniquePtr<nc::AnimatedSprite> sprite_;

	float dashEnergy_;
	nc::Vector2f dashDir_;

	int jumpCount_;

	void OnBubbleTouched(Bubble *bubble);
};
