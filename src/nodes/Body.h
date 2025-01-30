#pragma once

#include "LogicNode.h"
#include <ncine/Vector2.h>

enum class ColliderKind
{
	NONE,
	CIRCLE,
	AABB,
};

class Body;
struct CollisionPair
{
	Body *a;
	Body *b;
	nc::Vector2f normal;
};

enum class BodyKind
{
	STATIC,
	DYNAMIC,
};

enum BodyId
{
	UNDEFINED = 0,
	STATIC,
	BUBBLE,
	PLAYER,
};

class Body : public LogicNode
{
  public:
	static nctl::Array<Body *> All;
	static nctl::Array<CollisionPair> Collisions;

	nc::Vector2f linearVelocity_;
	float linearVelocityDamping_;
	float maxVelocity_;
	nc::Vector2f gravity_;

	/// In case of a circle, the `x` is used to represent its radius
	nc::Vector2f colliderHalfSize_;

	Body(SceneNode *parent, nctl::String name, ColliderKind collKind, BodyKind kind, int bodyId);
	~Body() override;

	inline int bodyId() const { return bodyId_; };
	inline BodyKind bodyKind() const { return bodyKind_; };
	inline ColliderKind colliderKind() const { return colliderKind_; };

	void onPostTick(nc::RenderQueue &renderQueue, unsigned int &visitOrderIndex) override;
	void integrate(float dT);

	bool isGrounded();

	void drawGui();

	static const char *colliderKindToName(ColliderKind kind);
	static void circleVsCircleCollision(Body *bodyA, Body *bodyB);
	static void circleVsAabbCollision(Body *bodyA, Body *bodyB);

  private:
	int bodyId_;
	BodyKind bodyKind_;
	ColliderKind colliderKind_;
};
