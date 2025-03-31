#include "InputBinder.h"
#include <ncine/Application.h>
#include <ncine/IInputManager.h>

InputBinder &inputBinder()
{
	static InputBinder instance;
	return instance;
}

bool InputBinder::MappedGamepadBinding::operator==(const MappedGamepadBinding &other) const
{
	const bool sameJoyId = (joyId == other.joyId);
	const bool sameButton = (button == other.button);
	const bool unknownAxis = (axis == nc::AxisName::UNKNOWN && axis == other.axis);
	const bool sameAxis = (axisSide == other.axisSide && axis == other.axis);
	const bool unknownButton = (button == nc::ButtonName::UNKNOWN && button == other.button);

	return sameJoyId && ((sameButton && unknownAxis) || (sameAxis && unknownButton));
}

///////////////////////////////////////////////////////////
// STATIC DEFINITIONS
///////////////////////////////////////////////////////////

const float InputBinder::ReleasedAxisThreshold = 0.45f;
const float InputBinder::PressedAxisThreshold = 0.85f;

///////////////////////////////////////////////////////////
// CONSTRUCTORS AND DESTRUCTOR
///////////////////////////////////////////////////////////

InputBinder::InputBinder()
    : actions_(16)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

unsigned int InputBinder::addAction(const char *name)
{
	unsigned int actionId = actions_.size();
	actions_.emplaceBack(name);
	return actionId;
}

unsigned int InputBinder::numActions() const
{
	return actions_.size();
}

const char *InputBinder::actionName(unsigned int actionId) const
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions);

	if (actionId < numActions)
		return actions_[actionId].name.data();
	return nullptr;
}

void InputBinder::clearActions()
{
	actions_.clear();
}

unsigned int InputBinder::addKeyboardBinding(unsigned int actionId, TriggerActivation activation, nc::KeySym key)
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions && key != nc::KeySym::UNKNOWN);

	unsigned int bindingId = InvalidId;
	if (actionId < numActions && key != nc::KeySym::UNKNOWN)
	{
		Binding binding;
		binding.activation = activation;
		binding.key = key;

		bindingId = actions_[actionId].bindings.size();
		actions_[actionId].bindings.pushBack(binding);
	}
	return bindingId;
}

unsigned int InputBinder::addMappedGamepadButtonBinding(unsigned int actionId, int joyId, TriggerActivation activation, nc::ButtonName button)
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions && joyId >= 0 && button != nc::ButtonName::UNKNOWN);

	unsigned int bindingId = InvalidId;
	if (actionId < numActions && joyId >= 0 && button != nc::ButtonName::UNKNOWN)
	{
		Binding binding;
		binding.activation = activation;
		binding.joyId = joyId;
		binding.button = button;

		bindingId = actions_[actionId].bindings.size();
		actions_[actionId].bindings.pushBack(binding);
	}
	return bindingId;
}

unsigned int InputBinder::addMappedGamepadAxisBinding(unsigned int actionId, int joyId, TriggerActivation activation, AxisSide side, nc::AxisName axis)
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions && joyId >= 0 && axis != nc::AxisName::UNKNOWN);

	unsigned int bindingId = InvalidId;
	if (actionId < numActions && joyId >= 0 && axis != nc::AxisName::UNKNOWN)
	{
		Binding binding;
		binding.activation = activation;
		binding.joyId = joyId;
		binding.axisSide = side;
		binding.axis = axis;

		bindingId = actions_[actionId].bindings.size();
		actions_[actionId].bindings.pushBack(binding);
	}
	return bindingId;
}

unsigned int InputBinder::addGamepadButtonBinding(unsigned int actionId, int joyId, TriggerActivation activation, int buttonId)
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions && joyId >= 0 && buttonId >= 0);

	unsigned int bindingId = InvalidId;
	if (actionId < numActions && joyId >= 0 && buttonId >= 0)
	{
		Binding binding;
		binding.activation = activation;
		binding.joyId = joyId;
		binding.buttonId = buttonId;

		bindingId = actions_[actionId].bindings.size();
		actions_[actionId].bindings.pushBack(binding);
	}
	return bindingId;
}

unsigned int InputBinder::addGamepadAxisBinding(unsigned int actionId, int joyId, TriggerActivation activation, AxisSide side, int axisId)
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions && joyId >= 0 && axisId >= 0);

	unsigned int bindingId = InvalidId;
	if (actionId < numActions && joyId >= 0 && axisId >= 0)
	{
		Binding binding;
		binding.activation = activation;
		binding.joyId = joyId;
		binding.axisSide = side;
		binding.axisId = axisId;

		bindingId = actions_[actionId].bindings.size();
		actions_[actionId].bindings.pushBack(binding);
	}
	return bindingId;
}

unsigned int InputBinder::addKeyboardBinding(unsigned int actionId, nc::KeySym key)
{
	return addKeyboardBinding(actionId, TriggerActivation::PRESSED, key);
}

unsigned int InputBinder::addMappedGamepadButtonBinding(unsigned int actionId, int joyId, nc::ButtonName button)
{
	return addMappedGamepadButtonBinding(actionId, joyId, TriggerActivation::PRESSED, button);
}

unsigned int InputBinder::addMappedGamepadAxisBinding(unsigned int actionId, int joyId, AxisSide side, nc::AxisName axis)
{
	return addMappedGamepadAxisBinding(actionId, joyId, TriggerActivation::PRESSED, side, axis);
}

unsigned int InputBinder::addGamepadButtonBinding(unsigned int actionId, int joyId, int buttonId)
{
	return addGamepadButtonBinding(actionId, joyId, TriggerActivation::PRESSED, buttonId);
}

unsigned int InputBinder::addGamepadAxisBinding(unsigned int actionId, int joyId, AxisSide side, int axisId)
{
	return addGamepadAxisBinding(actionId, joyId, TriggerActivation::PRESSED, side, axisId);
}

unsigned int InputBinder::addMappedGamepadAxisBinding(unsigned int actionId, int joyId, nc::AxisName axis)
{
	return addMappedGamepadAxisBinding(actionId, joyId, TriggerActivation::PRESSED, AxisSide::POSITIVE, axis);
}

unsigned int InputBinder::addGamepadAxisBinding(unsigned int actionId, int joyId, int axisId)
{
	return addGamepadAxisBinding(actionId, joyId, TriggerActivation::PRESSED, AxisSide::POSITIVE, axisId);
}

unsigned int InputBinder::numBindings(unsigned int actionId) const
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions);

	unsigned int numBindings = 0;
	if (actionId < numActions)
		numBindings = actions_[actionId].bindings.size();

	return numBindings;
}

void InputBinder::clearBindings(unsigned int actionId)
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions);

	if (actionId < numActions)
		actions_[actionId].bindings.clear();
}

nc::KeySym InputBinder::retrieveKeyboardBinding(unsigned int actionId) const
{
	nc::KeySym key = nc::KeySym::UNKNOWN;
	const unsigned int bindingId = findKeyboardBinding(actionId);
	if (bindingId != InvalidId)
	{
		const Binding &binding = actions_[actionId].bindings[bindingId];
		key = binding.key;
	}
	return key;
}

InputBinder::MappedGamepadBinding InputBinder::retrieveMappedGamepadBinding(unsigned int actionId) const
{
	MappedGamepadBinding mappedBinding = {};
	const unsigned int bindingId = findMappedGamepadBinding(actionId);
	if (bindingId != InvalidId)
	{
		const Binding &binding = actions_[actionId].bindings[bindingId];
		mappedBinding.joyId = binding.joyId;
		mappedBinding.button = binding.button;
		mappedBinding.axisSide = binding.axisSide;
		mappedBinding.axis = binding.axis;
	}
	return mappedBinding;
}

unsigned int InputBinder::setKeyboardBinding(unsigned int actionId, nc::KeySym key)
{
	const unsigned int bindingId = findKeyboardBinding(actionId);
	if (bindingId != InvalidId)
	{
		Binding &binding = actions_[actionId].bindings[bindingId];
		binding.key = key;
	}
	return bindingId;
}

unsigned int InputBinder::setMappedGamepadBinding(unsigned int actionId, const MappedGamepadBinding &mappedBinding)
{
	const bool bindingIsValid = ((mappedBinding.button != nc::ButtonName::UNKNOWN && mappedBinding.axis == nc::AxisName::UNKNOWN) ||
	                             (mappedBinding.button == nc::ButtonName::UNKNOWN && mappedBinding.axis != nc::AxisName::UNKNOWN));

	const unsigned int bindingId = findMappedGamepadBinding(actionId);
	if (bindingIsValid && bindingId != InvalidId)
	{
		Binding &binding = actions_[actionId].bindings[bindingId];
		if (mappedBinding.joyId >= 0)
			binding.joyId = mappedBinding.joyId;
		binding.button = mappedBinding.button;
		binding.axisSide = mappedBinding.axisSide;
		binding.axis = mappedBinding.axis;
	}
	return bindingId;
}

bool InputBinder::isTriggered(unsigned int actionId) const
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions);

	bool triggered = false;
	if (actionId < numActions)
	{
		const nc::IInputManager &input = nc::theApplication().inputManager();
		const Action &action = actions_[actionId];
		for (unsigned int i = 0; i < action.bindings.size(); i++)
		{
			const Binding &binding = action.bindings[i];
			const int joyId = binding.joyId;

			if (binding.key != nc::KeySym::UNKNOWN)
			{
				const nc::KeyboardState &ks = input.keyboardState();
				switch (binding.activation)
				{
					case TriggerActivation::DOWN:
						triggered = ks.isKeyDown(binding.key);
						break;
					case TriggerActivation::PRESSED:
						triggered = ks.isKeyPressed(binding.key);
						break;
					case TriggerActivation::RELEASED:
						triggered = ks.isKeyReleased(binding.key);
						break;
				}
			}
			else if (joyId >= 0 && binding.button != nc::ButtonName::UNKNOWN && input.isJoyMapped(joyId))
			{
				const nc::JoyMappedState &js = input.joyMappedState(joyId);
				switch (binding.activation)
				{
					case TriggerActivation::DOWN:
						triggered = js.isButtonDown(binding.button);
						break;
					case TriggerActivation::PRESSED:
						triggered = js.isButtonPressed(binding.button);
						break;
					case TriggerActivation::RELEASED:
						triggered = js.isButtonReleased(binding.button);
						break;
				}
			}
			else if (joyId >= 0 && binding.axis != nc::AxisName::UNKNOWN && input.isJoyMapped(joyId))
			{
				const nc::JoyMappedState &js = input.joyMappedState(joyId);
				const float value = (binding.axisSide == AxisSide::POSITIVE) ? js.axisValue(binding.axis) : -js.axisValue(binding.axis);
				switch (binding.activation)
				{
					case TriggerActivation::DOWN:
						triggered = (value > PressedAxisThreshold);
						break;
					case TriggerActivation::PRESSED:
						triggered = (value > PressedAxisThreshold && binding.lastAxisValue < ReleasedAxisThreshold);
						break;
					case TriggerActivation::RELEASED:
						triggered = (value < ReleasedAxisThreshold && binding.lastAxisValue > PressedAxisThreshold);
						break;
				}
				if (value < ReleasedAxisThreshold || value > PressedAxisThreshold)
					binding.lastAxisValue = value;
			}
			else if (joyId >= 0 && binding.buttonId >= 0 && input.isJoyPresent(joyId))
			{
				const nc::JoystickState &js = input.joystickState(joyId);
				switch (binding.activation)
				{
					case TriggerActivation::DOWN:
						triggered = js.isButtonDown(binding.buttonId);
						break;
					case TriggerActivation::PRESSED:
						triggered = js.isButtonPressed(binding.buttonId);
						break;
					case TriggerActivation::RELEASED:
						triggered = js.isButtonReleased(binding.buttonId);
						break;
				}
			}
			else if (joyId >= 0 && binding.axisId >= 0 && input.isJoyPresent(joyId))
			{
				const nc::JoystickState &js = input.joystickState(joyId);
				const float value = (binding.axisSide == AxisSide::POSITIVE) ? js.axisNormValue(binding.axisId) : -js.axisNormValue(binding.axisId);
				switch (binding.activation)
				{
					case TriggerActivation::DOWN:
						triggered = (value > PressedAxisThreshold);
						break;
					case TriggerActivation::PRESSED:
						triggered = (value > PressedAxisThreshold && binding.lastAxisValue < ReleasedAxisThreshold);
						break;
					case TriggerActivation::RELEASED:
						triggered = (value < ReleasedAxisThreshold && binding.lastAxisValue > PressedAxisThreshold);
						break;
				}
				if (value < ReleasedAxisThreshold || value > PressedAxisThreshold)
					binding.lastAxisValue = value;
			}

			if (triggered)
				break;
		}
	}
	return triggered;
}

float InputBinder::value(unsigned int actionId) const
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions);

	float value = 0.0f;
	if (actionId < numActions)
	{
		const nc::IInputManager &input = nc::theApplication().inputManager();
		const Action &action = actions_[actionId];
		for (unsigned int i = 0; i < action.bindings.size(); i++)
		{
			const Binding &binding = action.bindings[i];
			const int joyId = binding.joyId;

			if (binding.key != nc::KeySym::UNKNOWN)
			{
				const nc::KeyboardState &ks = input.keyboardState();
				switch (binding.activation)
				{
					case TriggerActivation::DOWN:
						value = (ks.isKeyDown(binding.key) ? 1.0f : 0.0f);
						break;
					case TriggerActivation::PRESSED:
						value = (ks.isKeyPressed(binding.key) ? 1.0f : 0.0f);
						break;
					case TriggerActivation::RELEASED:
						value = (ks.isKeyReleased(binding.key) ? 1.0f : 0.0f);
						break;
				}
			}
			else if (joyId >= 0 && binding.button != nc::ButtonName::UNKNOWN && input.isJoyMapped(joyId))
			{
				const nc::JoyMappedState &js = input.joyMappedState(joyId);
				switch (binding.activation)
				{
					case TriggerActivation::DOWN:
						value = (js.isButtonDown(binding.button) ? 1.0f : 0.0f);
						break;
					case TriggerActivation::PRESSED:
						value = (js.isButtonPressed(binding.button) ? 1.0f : 0.0f);
						break;
					case TriggerActivation::RELEASED:
						value = (js.isButtonReleased(binding.button) ? 1.0f : 0.0f);
						break;
				}
			}
			else if (joyId >= 0 && binding.axis != nc::AxisName::UNKNOWN && input.isJoyMapped(joyId))
			{
				const nc::JoyMappedState &js = input.joyMappedState(joyId);
				value = js.axisValue(binding.axis);
			}
			else if (joyId >= 0 && binding.buttonId >= 0 && input.isJoyPresent(joyId))
			{
				const nc::JoystickState &js = input.joystickState(joyId);
				switch (binding.activation)
				{
					case TriggerActivation::DOWN:
						value = (js.isButtonDown(binding.buttonId) ? 1.0f : 0.0f);
						break;
					case TriggerActivation::PRESSED:
						value = (js.isButtonPressed(binding.buttonId) ? 1.0f : 0.0f);
						break;
					case TriggerActivation::RELEASED:
						value = (js.isButtonReleased(binding.buttonId) ? 1.0f : 0.0f);
						break;
				}
			}
			else if (joyId >= 0 && binding.axisId >= 0 && input.isJoyPresent(joyId))
			{
				const nc::JoystickState &js = input.joystickState(joyId);
				value = js.axisNormValue(binding.axisId);
			}

			if (value != 0.0f)
				break;
		}
	}
	return value;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

unsigned int InputBinder::findKeyboardBinding(unsigned int actionId, nc::KeySym key) const
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions);

	unsigned int bindingId = InvalidId;
	if (actionId < numActions)
	{
		const Action &action = actions_[actionId];
		for (unsigned int i = 0; i < action.bindings.size(); i++)
		{
			const Binding &binding = action.bindings[i];
			// Skipping non-keyboard bindings
			if (binding.key == nc::KeySym::UNKNOWN)
				continue;
			// Taking the first binding as valid if the passed key is `nc::KeySym::UNKNOWN`
			else if (key == nc::KeySym::UNKNOWN || key == binding.key)
			{
				bindingId = i;
				break;
			}
		}
	}
	return bindingId;
}

unsigned int InputBinder::findKeyboardBinding(unsigned int actionId) const
{
	return findKeyboardBinding(actionId, nc::KeySym::UNKNOWN);
}

unsigned int InputBinder::findMappedGamepadBinding(unsigned int actionId, const MappedGamepadBinding &mappedBinding) const
{
	const unsigned int numActions = actions_.size();
	ASSERT(actionId < numActions);

	unsigned int bindingId = InvalidId;
	if (actionId < numActions)
	{
		const Action &action = actions_[actionId];
		for (unsigned int i = 0; i < action.bindings.size(); i++)
		{
			const Binding &binding = action.bindings[i];

			// Taking the first binding as valid if the passed binding has unknown values
			const bool anyIsValid = (mappedBinding.button == nc::ButtonName::UNKNOWN &&
			                         mappedBinding.axis == nc::AxisName::UNKNOWN);
			const bool sameButton = (binding.button == mappedBinding.button &&
			                         mappedBinding.axis == nc::AxisName::UNKNOWN);
			const bool sameAxis = (binding.axis == mappedBinding.axis && binding.axisSide == mappedBinding.axisSide &&
			                       mappedBinding.button == nc::ButtonName::UNKNOWN);

			// Skipping non-gamepad bindings
			if (binding.joyId < 0)
				continue;
			else if ((anyIsValid || sameButton || sameAxis) &&
			         (mappedBinding.joyId < 0 || mappedBinding.joyId == binding.joyId))
			{
				bindingId = i;
				break;
			}
		}
	}
	return bindingId;
}

unsigned int InputBinder::findMappedGamepadBinding(unsigned int actionId) const
{
	MappedGamepadBinding mappedBinding = {};
	return findMappedGamepadBinding(actionId, mappedBinding);
}

unsigned int InputBinder::findMappedGamepadButtonBinding(unsigned int actionId, nc::ButtonName button) const
{
	MappedGamepadBinding mappedBinding = {};
	mappedBinding.button = button;
	return findMappedGamepadBinding(actionId, mappedBinding);
}

unsigned int InputBinder::findMappedGamepadAxisBinding(unsigned int actionId, nc::AxisName axis) const
{
	MappedGamepadBinding mappedBinding = {};
	mappedBinding.axis = axis;
	return findMappedGamepadBinding(actionId, mappedBinding);
}
