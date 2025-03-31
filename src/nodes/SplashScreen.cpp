#include <ncine/config.h>
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	#include <ncine/imgui.h>
#endif

#include "SplashScreen.h"
#include "../main.h"
#include "../Config.h"
#include "../ResourceManager.h"
#include "../InputBinder.h"
#include "../InputActions.h"

#include <ncine/Application.h>
#include <ncine/FileSystem.h>
#include <ncine/Sprite.h>
#include <ncine/Font.h>
#include <ncine/TextNode.h>

namespace {
	float FastFadeTime = 0.0f;
	float FastFadeStartAlpha = 1.0f;
}

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

SplashScreen::SplashScreen(SceneNode *parent, nctl::String name, MyEventHandler *eventHandler)
    : LogicNode(parent, name), eventHandler_(eventHandler), state_(AnimState::FADE_IN)
{
#ifndef __EMSCRIPTEN__
	const float screenWidth = nc::theApplication().gfxDevice().width();
	const float screenHeight = nc::theApplication().gfxDevice().height();
#else
	const float screenWidth = nc::theApplication().gfxDevice().drawableWidth();
	const float screenHeight = nc::theApplication().gfxDevice().drawableHeight();
#endif
	const nc::Vector2f screenTopRight(screenWidth, screenHeight);

	const float windowScaling = nc::theApplication().gfxDevice().windowScalingFactor();
	this->setScale(windowScaling);

	background_ = nctl::makeUnique<nc::Sprite>(this, nullptr);
	background_->setSize(screenTopRight);
	background_->setPosition(screenTopRight * 0.5f);
	background_->setColor(0x1F, 0x1F, 0x1F, 0xFF);
	background_->setLayer(Cfg::Layers::Background);

	nCineLogo_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::RedBar));
	nCineLogo_ = nctl::makeUnique<nc::Sprite>(this, resourceManager().retrieveTexture(Cfg::Textures::nCineLogo));
	nCineLogo_->setScale(0.5f);
	nCineLogo_->setPosition(screenTopRight * 0.5f);
	nCineLogo_->setLayer(Cfg::Layers::Background + 1);

	const nctl::String smallFontFntPath_ = nc::fs::joinPath(nc::fs::dataPath(), Cfg::Fonts::Modak20Fnt);
	smallFont_ = nctl::makeUnique<nc::Font>(smallFontFntPath_.data(), resourceManager().retrieveTexture(Cfg::Fonts::Modak20Png));

	smallText_ = nctl::makeUnique<nc::TextNode>(this, smallFont_.get(), 32);
	smallText_->setLayer(Cfg::Layers::Gui_Text);
	smallText_->setRenderMode(nc::Font::RenderMode::GLYPH_SPRITE);
	smallText_->setString("Made with nCine");
	smallText_->setPosition(screenTopRight.x * 0.5f, screenTopRight.y * 0.25f);

	stateTimer_.toNow();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void SplashScreen::onTick(float deltaTime)
{
	if (inputBinder().isTriggered(inputActions().UI_SKIP))
	{
		state_ = AnimState::FAST_FADE_OUT;
		stateTimer_.toNow();
		FastFadeStartAlpha = nCineLogo_->alpha() / 255.0f;
		FastFadeTime = Cfg::Splash::FastFadeTime * FastFadeStartAlpha;
	}

	const float timePassed = stateTimer_.secondsSince();

	switch (state_)
	{
		case AnimState::FADE_IN:
			if (timePassed < Cfg::Splash::FadeTime)
			{
				const float fraction = timePassed / Cfg::Splash::FadeTime;
				nCineLogo_->setColorF(1.0f, 1.0f, 1.0f, fraction);
				smallText_->setColorF(1.0f, 1.0f, 1.0f, fraction);
			}
			else
			{
				nCineLogo_->setColorF(1.0f, 1.0f, 1.0f, 1.0f);
				smallText_->setColorF(1.0f, 1.0f, 1.0f, 1.0f);
				state_ = AnimState::SUSTAIN;
				stateTimer_.toNow();
			}
			break;
		case AnimState::SUSTAIN:
			if (timePassed >= Cfg::Splash::SustainTime)
			{
				state_ = AnimState::FADE_OUT;
				stateTimer_.toNow();
			}
			break;
		case AnimState::FADE_OUT:
			if (timePassed < Cfg::Splash::FadeTime)
			{
				const float fraction = timePassed / Cfg::Splash::FadeTime;
				nCineLogo_->setColorF(1.0f, 1.0f, 1.0f, 1.0f - fraction);
				smallText_->setColorF(1.0f, 1.0f, 1.0f, 1.0f - fraction);
			}
			else
			{
				nCineLogo_->setColorF(1.0f, 1.0f, 1.0f, 0.0f);
				smallText_->setColorF(1.0f, 1.0f, 1.0f, 0.0f);
				state_ = AnimState::END;
			}
			break;
		case AnimState::FAST_FADE_OUT:
			if (timePassed < FastFadeTime)
			{
				const float fraction = timePassed / FastFadeTime;
				nCineLogo_->setColorF(1.0f, 1.0f, 1.0f, FastFadeStartAlpha - fraction);
				smallText_->setColorF(1.0f, 1.0f, 1.0f, FastFadeStartAlpha - fraction);
			}
			else
			{
				nCineLogo_->setColorF(1.0f, 1.0f, 1.0f, 0.0f);
				smallText_->setColorF(1.0f, 1.0f, 1.0f, 0.0f);
				state_ = AnimState::END;
			}
			break;
		case AnimState::END:
			eventHandler_->requestMenu();
			break;
	}
}

void SplashScreen::drawGui()
{
#if NCINE_WITH_IMGUI && defined(NCPROJECT_DEBUG)
	const char *stateName = [&] {
		switch (state_)
		{
			case AnimState::FADE_IN: return "Fade In";
			case AnimState::SUSTAIN: return "Sustain";
			case AnimState::FADE_OUT: return "Fade Out";
			case AnimState::FAST_FADE_OUT: return "Fast Fade Out";
			case AnimState::END: return "End";
		}
		return "Unknown";
	}();

	const float fraction = [&] {
		const float timePassed = stateTimer_.secondsSince();
		switch (state_)
		{
			case AnimState::FADE_IN:
			case AnimState::FADE_OUT:
				return timePassed / Cfg::Splash::FadeTime;
			case AnimState::FAST_FADE_OUT:
				return timePassed / FastFadeTime;
			case AnimState::SUSTAIN:
				return timePassed / Cfg::Splash::SustainTime;
			case AnimState::END: return 1.0f;
		}
		return 0.0f;
	}();

	ImGui::Text("State: %s", stateName);
	ImGui::ProgressBar(fraction, ImVec2(0.0f, 0.0f), "Time");

	if (ImGui::Button("Skip"))
		state_ = AnimState::END;
#endif
}
