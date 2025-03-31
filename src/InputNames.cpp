#include "InputNames.h"
#include <ncine/InputEvents.h>

const char *keySymToName(ncine::KeySym key)
{
	switch (key)
	{
		case nc::KeySym::UNKNOWN: return "Unknown";

		case nc::KeySym::BACKSPACE: return "Backspace";
		case nc::KeySym::TAB: return "Tab";
		case nc::KeySym::RETURN: return "Return";
		case nc::KeySym::ESCAPE: return "Escape";
		case nc::KeySym::SPACE: return "Space";
		case nc::KeySym::QUOTE: return "\"";
		case nc::KeySym::PLUS: return "+";
		case nc::KeySym::COMMA: return ",";
		case nc::KeySym::MINUS: return "-";
		case nc::KeySym::PERIOD: return ".";
		case nc::KeySym::SLASH: return "/";
		case nc::KeySym::N0: return "0";
		case nc::KeySym::N1: return "1";
		case nc::KeySym::N2: return "2";
		case nc::KeySym::N3: return "3";
		case nc::KeySym::N4: return "4";
		case nc::KeySym::N5: return "5";
		case nc::KeySym::N6: return "6";
		case nc::KeySym::N7: return "7";
		case nc::KeySym::N8: return "8";
		case nc::KeySym::N9: return "9";
		case nc::KeySym::SEMICOLON: return ";";
		case nc::KeySym::LEFTBRACKET: return "[";
		case nc::KeySym::BACKSLASH: return "\\";
		case nc::KeySym::RIGHTBRACKET: return "]";
		case nc::KeySym::BACKQUOTE: return "`";

		case nc::KeySym::A: return "A";
		case nc::KeySym::B: return "B";
		case nc::KeySym::C: return "C";
		case nc::KeySym::D: return "D";
		case nc::KeySym::E: return "E";
		case nc::KeySym::F: return "F";
		case nc::KeySym::G: return "G";
		case nc::KeySym::H: return "H";
		case nc::KeySym::I: return "I";
		case nc::KeySym::J: return "J";
		case nc::KeySym::K: return "K";
		case nc::KeySym::L: return "L";
		case nc::KeySym::M: return "M";
		case nc::KeySym::N: return "N";
		case nc::KeySym::O: return "O";
		case nc::KeySym::P: return "P";
		case nc::KeySym::Q: return "Q";
		case nc::KeySym::R: return "R";
		case nc::KeySym::S: return "S";
		case nc::KeySym::T: return "T";
		case nc::KeySym::U: return "U";
		case nc::KeySym::V: return "V";
		case nc::KeySym::W: return "W";
		case nc::KeySym::X: return "X";
		case nc::KeySym::Y: return "Y";
		case nc::KeySym::Z: return "Z";
		case nc::KeySym::DELETE: return "Delete";

		case nc::KeySym::KP0: return "KP0";
		case nc::KeySym::KP1: return "KP1";
		case nc::KeySym::KP2: return "KP2";
		case nc::KeySym::KP3: return "KP3";
		case nc::KeySym::KP4: return "KP4";
		case nc::KeySym::KP5: return "KP5";
		case nc::KeySym::KP6: return "KP6";
		case nc::KeySym::KP7: return "KP7";
		case nc::KeySym::KP8: return "KP8";
		case nc::KeySym::KP9: return "KP9";
		case nc::KeySym::KP_PERIOD: return "KP_.";
		case nc::KeySym::KP_DIVIDE: return "KP_/";
		case nc::KeySym::KP_MULTIPLY: return "KP_*";
		case nc::KeySym::KP_MINUS: return "KP_-";
		case nc::KeySym::KP_PLUS: return "KP_+";
		case nc::KeySym::KP_ENTER: return "KP_Enter";
		case nc::KeySym::KP_EQUALS: return "KP_=";

		case nc::KeySym::UP: return "Up Arrow";
		case nc::KeySym::DOWN: return "Down Arrow";
		case nc::KeySym::RIGHT: return "Right Arrow";
		case nc::KeySym::LEFT: return "Left Arrow";
		case nc::KeySym::INSERT: return "Insert";
		case nc::KeySym::HOME: return "Home";
		case nc::KeySym::END: return "End";
		case nc::KeySym::PAGEUP: return "PgUp";
		case nc::KeySym::PAGEDOWN: return "PgDn";

		case nc::KeySym::F1: return "F1";
		case nc::KeySym::F2: return "F2";
		case nc::KeySym::F3: return "F3";
		case nc::KeySym::F4: return "F4";
		case nc::KeySym::F5: return "F5";
		case nc::KeySym::F6: return "F6";
		case nc::KeySym::F7: return "F7";
		case nc::KeySym::F8: return "F8";
		case nc::KeySym::F9: return "F9";
		case nc::KeySym::F10: return "F10";
		case nc::KeySym::F11: return "F11";
		case nc::KeySym::F12: return "F12";
		case nc::KeySym::F13: return "F13";
		case nc::KeySym::F14: return "F14";
		case nc::KeySym::F15: return "F15";

		case nc::KeySym::NUM_LOCK: return "Num Lock";
		case nc::KeySym::CAPS_LOCK: return "Caps Lock";
		case nc::KeySym::SCROLL_LOCK: return "Scroll Lock";
		case nc::KeySym::RSHIFT: return "Right Shift";
		case nc::KeySym::LSHIFT: return "Left Shift";
		case nc::KeySym::RCTRL: return "Right Ctrl";
		case nc::KeySym::LCTRL: return "Left Ctrl";
		case nc::KeySym::RALT: return "Right Alt";
		case nc::KeySym::LALT: return "Left Alt";
		case nc::KeySym::RSUPER: return "Right Super";
		case nc::KeySym::LSUPER: return "Left Super";
		case nc::KeySym::PRINTSCREEN: return "PtrSc";
		case nc::KeySym::PAUSE: return "Pause";
		case nc::KeySym::MENU: return "Menu";

		case nc::KeySym::CLEAR: return "Clear";
		case nc::KeySym::EXCLAIM: return "!";
		case nc::KeySym::QUOTEDBL: return "\"";
		case nc::KeySym::HASH: return "#";
		case nc::KeySym::DOLLAR: return "$";
		case nc::KeySym::AMPERSAND: return "&";
		case nc::KeySym::LEFTPAREN: return "(";
		case nc::KeySym::RIGHTPAREN: return ")";
		case nc::KeySym::ASTERISK: return "*";
		case nc::KeySym::COLON: return ":";
		case nc::KeySym::LESS: return "<";
		case nc::KeySym::EQUALS: return "=";
		case nc::KeySym::GREATER: return ">";
		case nc::KeySym::QUESTION: return "?";
		case nc::KeySym::AT: return "@";
		case nc::KeySym::CARET: return "^";
		case nc::KeySym::UNDERSCORE: return "_";
		case nc::KeySym::MODE: return "Mode";
		case nc::KeySym::APPLICATION: return "Application";
		case nc::KeySym::HELP: return "Help";
		case nc::KeySym::SYSREQ: return "SysRq";
		case nc::KeySym::POWER: return "Power";
		case nc::KeySym::UNDO: return "Undo";

		case nc::KeySym::WORLD1: return "World1";
		case nc::KeySym::WORLD2: return "World2";

		case nc::KeySym::FUNCTION_KEY: return "Fn";

		default:
			return "Unknown";
	}

	return "Unknown";
}

const char *buttonToName(nc::ButtonName button)
{
	switch (button)
	{
		case nc::ButtonName::UNKNOWN: return "Unknown";

		case nc::ButtonName::A: return "A";
		case nc::ButtonName::B: return "B";
		case nc::ButtonName::X: return "X";
		case nc::ButtonName::Y: return "Y";
		case nc::ButtonName::BACK: return "Back";
		case nc::ButtonName::GUIDE: return "Guide";
		case nc::ButtonName::START: return "Start";
		case nc::ButtonName::LSTICK: return "Left Stick";
		case nc::ButtonName::RSTICK: return "Right Stick";
		case nc::ButtonName::LBUMPER: return "Left Bumper";
		case nc::ButtonName::RBUMPER: return "Right Bumper";
		case nc::ButtonName::DPAD_UP: return "DPad Up";
		case nc::ButtonName::DPAD_DOWN: return "Dpad Down";
		case nc::ButtonName::DPAD_LEFT: return "DPad Left";
		case nc::ButtonName::DPAD_RIGHT: return "DPad Right";
		case nc::ButtonName::MISC1: return "Misc1";
		case nc::ButtonName::PADDLE1: return "Paddle1";
		case nc::ButtonName::PADDLE2: return "Paddle2";
		case nc::ButtonName::PADDLE3: return "Paddle3";
		case nc::ButtonName::PADDLE4: return "Paddle4";

		default:
			return "Unknown";
	}

	return "Unknown";
}

const char *axisToName(nc::AxisName axis)
{
	switch (axis)
	{
		case nc::AxisName::UNKNOWN: return "Unknown";

		case nc::AxisName::LX: return "Left X";
		case nc::AxisName::LY: return "Left Y";
		case nc::AxisName::RX: return "Right X";
		case nc::AxisName::RY: return "Right Y";
		case nc::AxisName::LTRIGGER: return "Left Trigger";
		case nc::AxisName::RTRIGGER: return "Right Trigger";

		default:
			return "Unknown";
	}

	return "Unknown";
}
