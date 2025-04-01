#include "InputActions.h"
#include "InputBinder.h"

const InputActions &inputActions()
{
	return inputActionsMut();
}

InputActions &inputActionsMut()
{
	static InputActions instance;
	return instance;
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void InputActions::setupBindings()
{
	InputBinder &ib = inputBinder();

	ASSERT(ib.numActions() == 0);
	if (ib.numActions() > 0)
		return;

	UI_SKIP = ib.addAction("UI Skip");
	ib.addKeyboardBinding(UI_SKIP, nc::KeySym::ESCAPE);
	ib.addMappedGamepadButtonBinding(UI_SKIP, 0, nc::ButtonName::START);

	UI_NEXT = ib.addAction("UI Next");
	ib.addKeyboardBinding(UI_NEXT, nc::KeySym::DOWN);
	ib.addMappedGamepadButtonBinding(UI_NEXT, 0, nc::ButtonName::DPAD_DOWN);
	ib.addMappedGamepadAxisBinding(UI_NEXT, 0, InputBinder::TriggerActivation::PRESSED, InputBinder::AxisSide::POSITIVE, nc::AxisName::LY);

	UI_PREV = ib.addAction("UI Previous");
	ib.addKeyboardBinding(UI_PREV, nc::KeySym::UP);
	ib.addMappedGamepadButtonBinding(UI_PREV, 0, nc::ButtonName::DPAD_UP);
	ib.addMappedGamepadAxisBinding(UI_PREV, 0, InputBinder::TriggerActivation::PRESSED, InputBinder::AxisSide::NEGATIVE, nc::AxisName::LY);

	UI_LEFT = ib.addAction("UI Left");
	ib.addKeyboardBinding(UI_LEFT, nc::KeySym::LEFT);
	ib.addMappedGamepadButtonBinding(UI_LEFT, 0, nc::ButtonName::DPAD_LEFT);
	ib.addMappedGamepadAxisBinding(UI_LEFT, 0, InputBinder::TriggerActivation::PRESSED, InputBinder::AxisSide::NEGATIVE, nc::AxisName::LX);

	UI_RIGHT = ib.addAction("UI Right");
	ib.addKeyboardBinding(UI_RIGHT, nc::KeySym::RIGHT);
	ib.addMappedGamepadButtonBinding(UI_RIGHT, 0, nc::ButtonName::DPAD_RIGHT);
	ib.addMappedGamepadAxisBinding(UI_RIGHT, 0, InputBinder::TriggerActivation::PRESSED, InputBinder::AxisSide::POSITIVE, nc::AxisName::LX);

	UI_ENTER = ib.addAction("UI Enter");
	ib.addKeyboardBinding(UI_ENTER, nc::KeySym::RETURN);
	ib.addMappedGamepadButtonBinding(UI_ENTER, 0, nc::ButtonName::A);

	UI_BACK = ib.addAction("UI Back");
	ib.addKeyboardBinding(UI_BACK, nc::KeySym::ESCAPE);
	ib.addMappedGamepadButtonBinding(UI_BACK, 0, nc::ButtonName::B);
	ib.addMappedGamepadButtonBinding(UI_BACK, 0, nc::ButtonName::BACK);

	P1_LEFT = ib.addAction("P1 Left");
	ib.addKeyboardBinding(P1_LEFT, InputBinder::TriggerActivation::DOWN, nc::KeySym::A);
	ib.addMappedGamepadAxisBinding(P1_LEFT, 0, InputBinder::TriggerActivation::DOWN, InputBinder::AxisSide::NEGATIVE, nc::AxisName::LX);

	P1_RIGHT = ib.addAction("P1 Right");
	ib.addKeyboardBinding(P1_RIGHT, InputBinder::TriggerActivation::DOWN, nc::KeySym::D);
	ib.addMappedGamepadAxisBinding(P1_RIGHT, 0, InputBinder::TriggerActivation::DOWN, InputBinder::AxisSide::POSITIVE, nc::AxisName::LX);

	P1_JUMP = ib.addAction("P1 Jump");
	ib.addKeyboardBinding(P1_JUMP, nc::KeySym::W);
	ib.addMappedGamepadButtonBinding(P1_JUMP, 0, nc::ButtonName::A);

	P1_DASH = ib.addAction("P1 Dash");
	ib.addKeyboardBinding(P1_DASH, nc::KeySym::S);
	ib.addMappedGamepadButtonBinding(P1_DASH, 0, nc::ButtonName::B);

	P2_LEFT = ib.addAction("P2 Left");
	ib.addKeyboardBinding(P2_LEFT, InputBinder::TriggerActivation::DOWN, nc::KeySym::LEFT);
	ib.addMappedGamepadAxisBinding(P2_LEFT, 1, InputBinder::TriggerActivation::DOWN, InputBinder::AxisSide::NEGATIVE, nc::AxisName::LX);

	P2_RIGHT = ib.addAction("P2 Right");
	ib.addKeyboardBinding(P2_RIGHT, InputBinder::TriggerActivation::DOWN, nc::KeySym::RIGHT);
	ib.addMappedGamepadAxisBinding(P2_RIGHT, 1, InputBinder::TriggerActivation::DOWN, InputBinder::AxisSide::POSITIVE, nc::AxisName::LX);

	P2_JUMP = ib.addAction("P2 Jump");
	ib.addKeyboardBinding(P2_JUMP, nc::KeySym::UP);
	ib.addMappedGamepadButtonBinding(P2_JUMP, 1, nc::ButtonName::A);

	P2_DASH = ib.addAction("P2 Dash");
	ib.addKeyboardBinding(P2_DASH, nc::KeySym::DOWN);
	ib.addMappedGamepadButtonBinding(P2_DASH, 1, nc::ButtonName::B);

	GAME_PAUSE = ib.addAction("Game Pause");
	ib.addKeyboardBinding(GAME_PAUSE, nc::KeySym::ESCAPE);
	ib.addMappedGamepadButtonBinding(GAME_PAUSE, 0, nc::ButtonName::START);
	ib.addMappedGamepadButtonBinding(GAME_PAUSE, 1, nc::ButtonName::START);
}
