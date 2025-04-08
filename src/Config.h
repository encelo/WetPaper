#pragma once

#include <ncine/Vector2.h>

namespace nc = ncine;

namespace Configuration
{
	namespace Layers
	{
		enum
		{
			Background = 0,
			Player = 100,
			Bubble = 128,
			Gui_StaminaBar = 256,
			Gui_StaminaBar_Fill = Gui_StaminaBar + 1,
			Gui_Text = Gui_StaminaBar + 1,
			Menu_Page = 512,
		};
	}

	char const * const SettingsFilename = "WetPaper/Settings.toml";
	char const * const StatisticsFilename = "WetPaper/Statistics.toml";

	namespace Textures
	{
		char const *const nCineLogo = "textures/nCineLogo_1024.png";
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
			SpriteData(Textures::Players[1], nc::Vector2i(488, 424), 0.8f, nc::Vector2f(20.0f, 10.0f))
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
		char const * const Modak200Fnt = "fonts/Modak-200.fnt";
		char const * const Modak200Png = "fonts/Modak-200.png";

		char const * const Modak50Fnt = "fonts/Modak-50.fnt";
		char const * const Modak50Png = "fonts/Modak-50.png";

		char const * const Modak20Fnt = "fonts/Modak-20.fnt";
		char const * const Modak20Png = "fonts/Modak-20.png";
	}

	namespace Splash
	{
		const float FadeTime = 1.5f;
		const float FastFadeTime = 0.5f;
		const float SustainTime = 1.0f;
	}

	namespace Menu
	{
		const nc::Vector2f MenuPageRelativePos(0.5f, 0.6f);
		const float BackgroundBubbleSpeed = 150.0f;
		const float BackgroundBubbleSpeedVariance = 0.25f;
		const unsigned int MaxMenuEntryLength = 32;
	}

	namespace Settings
	{
		const float VolumeGainMin = 0.0f;
		const float VolumeGainMax = 1.0f;
		const float VolumeGainStep = 0.1f;

		const unsigned int MatchTimeMin = 30;
		const unsigned int MatchTimeMax = 120;
		const unsigned int MatchTimeStep = 30;
	}

	namespace Game
	{
		const nc::Vector2i Resolution(1920, 1080);
		const unsigned int NumBubbleForSpawnPerPlayer = 5;
		const float FloorHeight = 32.0f;
	}

	namespace Player
	{
		const float MaxAirMoveSpeed = 10.0f;
		const float MaxGroundMoveSpeed = 20.0f;
		const float JumpVelocity = 600.0f;
		const float MaxDashVelocity = 100.0f;
		const float DashDuration = 0.1f; // Max time (in seconds) for the dash
		const float MaxStamina = 1.0f;
		const float DashStaminaCost = MaxStamina * 0.5f;
		const float StaminaRegenTime = 2.0f;
		const int MaxJumpCount = 2;
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
