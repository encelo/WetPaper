#pragma once

namespace ncine {
	enum class KeySym;
	enum class ButtonName : short int;
	enum class AxisName : short int;
}

namespace nc = ncine;

const char *keySymToName(nc::KeySym key);
const char *buttonToName(nc::ButtonName button);
const char *axisToName(nc::AxisName axis);
