#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(WETPAPER_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "Menu.h"
#include "../main.h"
#include "../Config.h"
#include "../ResourceManager.h"

#include <ncine/Application.h>
#include <ncine/FileSystem.h>
#include <ncine/Sprite.h>
#include <ncine/Font.h>
#include <ncine/TextNode.h>

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

Menu::Menu(SceneNode *parent, nctl::String name, MyEventHandler *eventHandler)
    : LogicNode(parent, name), eventHandler_(eventHandler)
{
	const float screenWidth = nc::theApplication().width();
	const float screenHeight = nc::theApplication().height();
	const nc::Vector2f screenTopRight(screenWidth, screenHeight);

	background_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::StartScreen));
	background_->setPosition(screenTopRight * 0.5f);
	background_->setLayer(Cfg::Layers::Background);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Menu::onTick(float deltaTime)
{
#ifndef WETPAPER_DEBUG
	if (showMenuTimer_.secondsSince() > 5.0f)
		eventHandler_->requestGame();
#endif
}

void Menu::drawGui()
{
#if NCINE_WITH_IMGUI && defined(WETPAPER_DEBUG)
	if (ImGui::Button("PLAY"))
		eventHandler_->requestGame();
#endif
}
