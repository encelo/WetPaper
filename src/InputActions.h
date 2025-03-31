#pragma once

/// Input actions and binding ids for the game
class InputActions
{
  public:
	unsigned int UI_SKIP = 0;

	unsigned int UI_NEXT = 0;
	unsigned int UI_PREV = 0;
	unsigned int UI_LEFT = 0;
	unsigned int UI_RIGHT = 0;
	unsigned int UI_ENTER = 0;
	unsigned int UI_BACK = 0;

	unsigned int P1_LEFT = 0;
	unsigned int P1_RIGHT = 0;
	unsigned int P1_JUMP = 0;
	unsigned int P1_DASH = 0;

	unsigned int P2_LEFT = 0;
	unsigned int P2_RIGHT = 0;
	unsigned int P2_JUMP = 0;
	unsigned int P2_DASH = 0;

	void setupBindings();
};

// Constant Meyers' Singleton
extern const InputActions &inputActions();

// Meyers' Singleton
extern InputActions &inputActionsMut();
