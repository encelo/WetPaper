#include "LogicNode.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

LogicNode::LogicNode(SceneNode *parent, nctl::String name)
    : SceneNode(parent)
{
	setName(name.data());
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void LogicNode::update(float interval)
{
	onTick(interval);
	SceneNode::update(interval);
}

void LogicNode::visit(nc::RenderQueue &renderQueue, unsigned int &visitOrderIndex)
{
	onPostTick(renderQueue, visitOrderIndex);
	SceneNode::visit(renderQueue, visitOrderIndex);
}
