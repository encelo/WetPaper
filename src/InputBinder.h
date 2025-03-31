#pragma once

#include <nctl/String.h>
#include <nctl/Array.h>
#include <ncine/InputEvents.h>

namespace nc = ncine;

/// Binds actions to the corresponding input
class InputBinder
{
  public:
	static const unsigned int InvalidId = ~0u;
	static const float ReleasedAxisThreshold;
	static const float PressedAxisThreshold;

	enum class TriggerActivation
	{
		DOWN,
		PRESSED,
		RELEASED
	};

	enum class AxisSide
	{
		POSITIVE,
		NEGATIVE
	};

	struct MappedGamepadBinding
	{
		bool operator==(const MappedGamepadBinding &other) const;

		int joyId = -1;
		nc::ButtonName button = nc::ButtonName::UNKNOWN;
		AxisSide axisSide = AxisSide::POSITIVE;
		nc::AxisName axis = nc::AxisName::UNKNOWN;
	};

	InputBinder();

	unsigned int addAction(const char *name);
	unsigned int numActions() const;
	const char *actionName(unsigned int actionId) const;
	void clearActions();

	unsigned int addKeyboardBinding(unsigned int actionId, TriggerActivation activation, nc::KeySym key);
	unsigned int addMappedGamepadButtonBinding(unsigned int actionId, int joyId, TriggerActivation activation, nc::ButtonName button);
	unsigned int addMappedGamepadAxisBinding(unsigned int actionId, int joyId, TriggerActivation activation, AxisSide side, nc::AxisName axis);
	unsigned int addGamepadButtonBinding(unsigned int actionId, int joyId, TriggerActivation activation, int buttonId);
	unsigned int addGamepadAxisBinding(unsigned int actionId, int joyId, TriggerActivation activation, AxisSide side, int axisId);

	unsigned int addKeyboardBinding(unsigned int actionId, nc::KeySym key);
	unsigned int addMappedGamepadButtonBinding(unsigned int actionId, int joyId, nc::ButtonName button);
	unsigned int addMappedGamepadAxisBinding(unsigned int actionId, int joyId, AxisSide side, nc::AxisName axis);
	unsigned int addGamepadButtonBinding(unsigned int actionId, int joyId, int buttonId);
	unsigned int addGamepadAxisBinding(unsigned int actionId, int joyId, AxisSide side, int axisId);

	unsigned int addMappedGamepadAxisBinding(unsigned int actionId, int joyId, nc::AxisName axis);
	unsigned int addGamepadAxisBinding(unsigned int actionId, int joyId, int axisId);

	unsigned int numBindings(unsigned int actionId) const;
	void clearBindings(unsigned int actionId);

	nc::KeySym retrieveKeyboardBinding(unsigned int actionId) const;
	MappedGamepadBinding retrieveMappedGamepadBinding(unsigned int actionId) const;
	unsigned int setKeyboardBinding(unsigned int actionId, nc::KeySym key);
	unsigned int setMappedGamepadBinding(unsigned int actionId, const MappedGamepadBinding &mappedBinding);

	bool isTriggered(unsigned int actionId) const;
	float value(unsigned int actionId) const;

  private:
	struct Binding
	{
		TriggerActivation activation = TriggerActivation::PRESSED;
		nc::KeySym key = nc::KeySym::UNKNOWN;
		int joyId = -1;
		nc::ButtonName button = nc::ButtonName::UNKNOWN;
		AxisSide axisSide = AxisSide::POSITIVE;
		nc::AxisName axis = nc::AxisName::UNKNOWN;
		int buttonId = -1;
		int axisId = -1;
		mutable float lastAxisValue = 0.0f;
	};

	struct Action
	{
		explicit Action(const char *nn)
		    : name(nn), bindings(4) {}

		nctl::String name;
		nctl::Array<Binding> bindings;
	};

	nctl::Array<Action> actions_;

	unsigned int findKeyboardBinding(unsigned int actionId, nc::KeySym key) const;
	unsigned int findKeyboardBinding(unsigned int actionId) const;

	unsigned int findMappedGamepadBinding(unsigned int actionId, const MappedGamepadBinding &mappedBinding) const;
	unsigned int findMappedGamepadBinding(unsigned int actionId) const;
	unsigned int findMappedGamepadButtonBinding(unsigned int actionId, nc::ButtonName button) const;
	unsigned int findMappedGamepadAxisBinding(unsigned int actionId, nc::AxisName axis) const;
};

// Meyers' Singleton
extern InputBinder &inputBinder();
