#include "Body.h"
#include "../DebugDraw.h"
#include "../Config.h"

nctl::Array<Body *> Body::All = {};
nctl::Array<CollisionPair> Body::Collisions = {};

namespace {

	const char *bodyKindToName(BodyKind kind)
	{
		switch (kind)
		{
			case BodyKind::STATIC: return "STATIC";
			case BodyKind::DYNAMIC: return "DYNAMIC";
			default: ASSERT_MSG_X(false, "Unknown BodyKind: %d", kind);
		}
	}

	const char *bodyIdToName(BodyId id)
	{
		switch (id)
		{
			case BodyId::UNDEFINED: return "UNDEFINED";
			case BodyId::STATIC: return "STATIC";
			case BodyId::BUBBLE: return "BUBBLE";
			case BodyId::PLAYER: return "PLAYER";
			default: ASSERT_MSG_X(false, "Unknown BodyId: %d", id);
		}
	}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

Body::Body(SceneNode *parent, nctl::String name, ColliderKind collKind, BodyKind kind, int bodyId)
    : LogicNode(parent, name),
      linearVelocity_(nc::Vector2f::Zero), linearVelocityDamping_(Cfg::Physics::LinearVelocityDamping),
      maxVelocity_(Cfg::Physics::PlayerMaxVelocity), gravity_(Cfg::Physics::Gravity),
      colliderHalfSize_(Cfg::Physics::ColliderHalfSize),
      bodyId_(bodyId), bodyKind_(kind), colliderKind_(collKind)
{
	All.pushBack(this);
}

Body::~Body()
{
	for (unsigned int i = 0; i < All.size(); i++)
	{
		if (All[i] == this)
		{
			All.unorderedRemoveAt(i);
			break;
		}
	}
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

const char *Body::bodyIdName() const
{
	switch (bodyId_)
	{
		case BodyId::UNDEFINED: return "UNDEFINED";
		case BodyId::STATIC: return "STATIC";
		case BodyId::BUBBLE: return "BUBBLE";
		case BodyId::PLAYER: return "PLAYER";
		default:
			ASSERT_MSG_X(false, "Unknown BodyId: %d", bodyId_);
			return "UNKNOWN";
	}
}

const char *Body::bodyKindName() const
{
	switch (bodyKind_)
	{
		case BodyKind::STATIC: return "STATIC";
		case BodyKind::DYNAMIC: return "DYNAMIC";
		default:
			ASSERT_MSG_X(false, "Unknown BodyKind: %d", bodyKind_);
			return "UNKNOWN";
	}
}

const char *Body::colliderKindName() const
{
	switch (colliderKind_)
	{
		case ColliderKind::NONE: return "NONE";
		case ColliderKind::CIRCLE: return "CIRCLE";
		case ColliderKind::AABB: return "AABB";
		default:
			ASSERT_MSG_X(false, "Unknown ColliderKind: %d", colliderKind_);
			return "UNKNOWN";
	}
}

void Body::onPostTick(nc::RenderQueue &renderQueue, unsigned int &visitOrderIndex)
{
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	const nc::Vector2f center = position() + parent()->position(); // works with scaling factor
	const nc::Color col = nc::Color(0, 255, 0, 255);

	switch (colliderKind_)
	{
		case ColliderKind::NONE:
			break;

		case ColliderKind::CIRCLE:
			DebugDraw::Circle(center, colliderHalfSize_.x, col);
			break;

		case ColliderKind::AABB:
			nc::Vector2f topLeft = center + nc::Vector2f(-colliderHalfSize_.x, -colliderHalfSize_.y);
			nc::Vector2f topRight = center + nc::Vector2f(colliderHalfSize_.x, -colliderHalfSize_.y);

			nc::Vector2f bottomLeft = center + nc::Vector2f(-colliderHalfSize_.x, colliderHalfSize_.y);
			nc::Vector2f bottomRight = center + nc::Vector2f(colliderHalfSize_.x, colliderHalfSize_.y);
			DebugDraw::Line(topLeft, topRight, col);
			DebugDraw::Line(bottomLeft, bottomRight, col);
			DebugDraw::Line(topLeft, bottomLeft, col);
			DebugDraw::Line(topRight, bottomRight, col);
			break;
	}

	// Visualize linear velocity
	DebugDraw::Line(center, center + linearVelocity_, nc::Color(255, 0, 255, 255));
#endif
}

void Body::integrate(float dT)
{
	if (bodyKind_ == BodyKind::STATIC)
		return;

	linearVelocity_ += gravity_ * dT;
	position_ += linearVelocity_ * dT;

	// Apply damping
	if (linearVelocityDamping_ < 1.0f)
		linearVelocity_ *= powf(linearVelocityDamping_, dT);

	// Limit maximum velocity
	if (linearVelocity_.sqrLength() > (maxVelocity_ * maxVelocity_))
		linearVelocity_ = linearVelocity_.normalized() * maxVelocity_;

	setPosition(position_);
}

bool Body::isGrounded()
{
	for (const CollisionPair &pair : Body::Collisions)
	{
		if (pair.a == this && pair.b->bodyKind_ == BodyKind::STATIC)
		{
			if (pair.normal.y > 0.0f)
				return true;
		}

		if (pair.b == this && pair.a->bodyKind_ == BodyKind::STATIC)
		{
			if (pair.normal.y > 0.0f)
				return true;
		}
	}

	return false;
}

void Body::drawGui()
{
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	ImGui::BulletText("Body - pos: <%0.2f, %0.2f>, vel: <%0.2f, %0.2f>, %s (%s)",
	                  position_.x, position_.y, linearVelocity_.x, linearVelocity_.y,
	                  bodyKindName(), bodyIdName());
#endif
}

// ------------------------------------------------------------------------------------------------

void Body::circleVsCircleCollision(Body *bodyA, Body *bodyB)
{
	ASSERT(bodyA->colliderKind_ == ColliderKind::CIRCLE);
	ASSERT(bodyB->colliderKind_ == ColliderKind::CIRCLE);

	const nc::Vector2f posA = bodyA->position();
	const nc::Vector2f posB = bodyB->position();

	const float radiusA = bodyA->colliderHalfSize_.x;
	const float radiusB = bodyB->colliderHalfSize_.x;

	nc::Vector2f aToB = posA - posB;
	const float dist2 = aToB.sqrLength();

	// Do they overlap?
	const float minDist = radiusA + radiusB;

	const float minDist2 = minDist * minDist;

	if (dist2 >= minDist2)
		return; // not overlapping

	// We want a deterministic order
	if (bodyA > bodyB)
	{
		Body *tmp = bodyA;
		bodyA = bodyB;
		bodyB = tmp;

		aToB = aToB * -1.0f;
	}

	// Add to collision pairs
	{
		const CollisionPair pair = { bodyA, bodyB, aToB.normalized() };

		for (const CollisionPair &it : Body::Collisions)
		{
			if (it.a == pair.a && it.b == pair.b)
				return; // avoid duplicates
		}

		Body::Collisions.pushBack(pair);
	}

	const float penetrationAmount = minDist - sqrtf(dist2);

	aToB.normalize();

	const float factor = (bodyA && bodyB) ? 0.5f : 1.0f;
	if (bodyA)
		bodyA->move(aToB * (penetrationAmount * factor));

	if (bodyB)
		bodyB->move(aToB * (-penetrationAmount * factor));
}

void Body::circleVsAabbCollision(Body *bodyA, Body *bodyB)
{
	ASSERT(bodyA->colliderKind_ == ColliderKind::CIRCLE);
	ASSERT(bodyB->colliderKind_ == ColliderKind::AABB);

	const float radiusA = bodyA->colliderHalfSize_.x;
	nc::Vector2f posA = bodyA->position();
	nc::Vector2f posB = bodyB->position();

	const float rectHalfW = bodyB->colliderHalfSize_.x;
	const float rectHalfH = bodyB->colliderHalfSize_.y;

	// Can we exclude this contact?
	const float circleRadius = radiusA;

	const nc::Vector2f circleRelPos = posA - posB;

	if ((fabsf(circleRelPos.x) - circleRadius) > rectHalfW ||
	    (fabsf(circleRelPos.y) - circleRadius) > rectHalfH)
	{
		return; // no collision
	}

	nc::Vector2f contactNormal = nc::Vector2f::Zero;

	nc::Vector2f closestPoint = circleRelPos;
	if (circleRelPos.x > rectHalfW)
	{
		closestPoint.x = rectHalfW;
		contactNormal.x = 1.0f;
	}
	else if (circleRelPos.x < -rectHalfW)
	{
		closestPoint.x = -rectHalfW;
		contactNormal.x = -1.0f;
	}

	if (circleRelPos.y > rectHalfH)
	{
		closestPoint.y = rectHalfH;
		contactNormal.y = 1.0f;
	}
	else if (circleRelPos.y < -rectHalfH)
	{
		closestPoint.y = -rectHalfH;
		contactNormal.y = -1.0f;
	}

	contactNormal.normalize();

	// Check if we're in contact
	const float closestPointToCircleRelPosSquared = (closestPoint - circleRelPos).sqrLength();
	if (closestPointToCircleRelPosSquared >= circleRadius * circleRadius)
	{
		return; // no collision
	}

	// Add to collision pairs
	{
		const CollisionPair pair = { bodyA, bodyB, contactNormal };

		for (const CollisionPair &it : Body::Collisions)
		{
			if (it.a == pair.a && it.b == pair.b)
				return; // avoid duplicates
		}

		Body::Collisions.pushBack(pair);
	}

	// Collision resolution
	const float penetrationAmount = circleRadius - sqrtf(closestPointToCircleRelPosSquared);
	ASSERT(penetrationAmount > 0.0f);

	const float factor = (bodyA->bodyKind_ == BodyKind::DYNAMIC && bodyB->bodyKind_ == BodyKind::DYNAMIC) ? 0.5f : 1.0f;
	if (bodyA->bodyKind_ == BodyKind::DYNAMIC)
		bodyA->move(contactNormal * (penetrationAmount * factor));

	if (bodyB->bodyKind_ == BodyKind::DYNAMIC)
		bodyB->move(contactNormal * (-penetrationAmount * factor));
}
