#pragma once

#include <ncine/Vector2.h>

namespace nc = ncine;

namespace Configuration
{
	enum Layers
	{
		Background = 0,
		Player = 100,
		Bubble = 128,
		Gui_StaminaBar = 256,
		Gui_StaminaBar_Fill = Gui_StaminaBar + 1,
		Gui_Text = Gui_StaminaBar + 1,
	};

	char const * const WindowStateFilename = ".windowstate";

	namespace Textures
	{
		char const * const StartScreen = "textures/start_screen.png";
		char const * const Background = "textures/background.png";

		char const * const RedBar = "textures/red_bar.png";
		char const * const RedBarFill = "textures/red_bar_fill.png";
		char const * const BlueBar = "textures/blue_bar.png";
		char const * const BlueBarFill = "textures/blue_bar_fill.png";

		const unsigned int NumBubbleVariants = 4;
		char const * const Bubbles[NumBubbleVariants] = {
			"textures/bubble_blue_normal.png",
			"textures/bubble_green_normal.png",
			"textures/bubble_red_normal.png",
			"textures/bubble_grey_normal.png",
		};

		char const * const Players[2] = {
			"textures/crab.png",
			"textures/crane.png"
		};
	}

	struct SpriteData
	{
		SpriteData(const char *tn, const nc::Vector2i &fs, float sc, nc::Vector2f of)
		    : textureName(tn), frameSize(fs), scale(sc), offset(of) {}
		SpriteData(const char *tn, const nc::Vector2i &fs, float sc)
		    : textureName(tn), frameSize(fs), scale(sc), offset(nc::Vector2f::Zero) {}
		SpriteData(const char *tn, const nc::Vector2i &fs)
		    : textureName(tn), frameSize(fs), scale(1.0f), offset(nc::Vector2f::Zero) {}

		const char *textureName;
		nc::Vector2i frameSize;
		float scale = 1.0f;
		nc::Vector2f offset = nc::Vector2f::Zero;
	};

	namespace Sprites
	{
		const SpriteData Players[2] = {
			SpriteData(Textures::Players[0], nc::Vector2i(357, 254), 0.8f),
			SpriteData(Textures::Players[1], nc::Vector2i(488, 424), 0.7f, nc::Vector2f(20, 0))
		};
	}

	namespace Sounds
	{
		const unsigned int NumBubblePops = 5;
		char const * const BubblePops[NumBubblePops] =
		{
			"sounds/bubble01.ogg",
			"sounds/bubble02.ogg",
			"sounds/bubble03.ogg",
			"sounds/bubble04.ogg",
			"sounds/bubble05.ogg"
		};
		/// The number of audio players dedicated for bubble popping sounds
		const unsigned int NumBubblePopPlayers = NumBubblePops * 3;

		const float MinSoundPitchBubble = 0.6f;
		const float MaxSoundPitchBubble = 1.2f;
	}

	namespace Fonts
	{
		char const * const CalibriBold50Fnt = "fonts/calibri_r_bold_50px_outline2px_A.fnt";
		char const * const CalibriBold50Png = "fonts/calibri_r_bold_50px_outline2px_A.png";
	}

	namespace Game
	{
		const nc::Vector2i Resolution(1920, 1080);
		const unsigned int MatchTime = 60;
		const unsigned int NumBubbleForSpawn = 10;
	}

	namespace Gui
	{
		const nc::Vector2f BarScale(0.5f);
		const nc::Vector2f RedBarRelativePos(0.1f, 0.9f);
		const nc::Vector2f BlueBarRelativePos(0.9f, 0.9f);
		const nc::Vector2f TimeTextRelativePos(0.5f, 0.98f);
		const nc::Vector2f PointsATextRelativePos(0.13f, 0.86f);
		const nc::Vector2f PointsBTextRelativePos(0.87f, 0.86f);
	}

	namespace Physics
	{
		const float LinearVelocityDamping = 1.0f;
		const float PlayerMaxVelocity = 10000.0f;
		const float BubbleMaxVelocity = 200.0f;
		const nc::Vector2f ColliderHalfSize(64.0f, 64.0f);
		const nc::Vector2f Gravity(0.0f, -100.0f);
	}
}

namespace Cfg = Configuration;
