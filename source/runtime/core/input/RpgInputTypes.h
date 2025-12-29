#pragma once

#include "core/RpgString.h"


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogInput)



namespace RpgInputKey
{
	enum EButton : uint8_t
	{
		NONE = 0,

		MOUSE_LEFT,
		MOUSE_MIDDLE,
		MOUSE_RIGHT,
		MOUSE_4,
		MOUSE_5,

		KEYBOARD_BACKSPACE,
		KEYBOARD_TAB,
		KEYBOARD_ENTER,
		KEYBOARD_PAUSE,
		KEYBOARD_CAPS_LOCK,
		KEYBOARD_ESCAPE,
		KEYBOARD_SPACEBAR,
		KEYBOARD_PAGE_UP,
		KEYBOARD_PAGE_DOWN,
		KEYBOARD_END,
		KEYBOARD_HOME,
		KEYBOARD_LEFT,
		KEYBOARD_UP,
		KEYBOARD_RIGHT,
		KEYBOARD_DOWN,
		KEYBOARD_PRINT_SCREEN,
		KEYBOARD_INSERT,
		KEYBOARD_DELETE,
		KEYBOARD_0,
		KEYBOARD_1,
		KEYBOARD_2,
		KEYBOARD_3,
		KEYBOARD_4,
		KEYBOARD_5,
		KEYBOARD_6,
		KEYBOARD_7,
		KEYBOARD_8,
		KEYBOARD_9,
		KEYBOARD_A,
		KEYBOARD_B,
		KEYBOARD_C,
		KEYBOARD_D,
		KEYBOARD_E,
		KEYBOARD_F,
		KEYBOARD_G,
		KEYBOARD_H,
		KEYBOARD_I,
		KEYBOARD_J,
		KEYBOARD_K,
		KEYBOARD_L,
		KEYBOARD_M,
		KEYBOARD_N,
		KEYBOARD_O,
		KEYBOARD_P,
		KEYBOARD_Q,
		KEYBOARD_R,
		KEYBOARD_S,
		KEYBOARD_T,
		KEYBOARD_U,
		KEYBOARD_V,
		KEYBOARD_W,
		KEYBOARD_X,
		KEYBOARD_Y,
		KEYBOARD_Z,
		KEYBOARD_NUMPAD_0,
		KEYBOARD_NUMPAD_1,
		KEYBOARD_NUMPAD_2,
		KEYBOARD_NUMPAD_3,
		KEYBOARD_NUMPAD_4,
		KEYBOARD_NUMPAD_5,
		KEYBOARD_NUMPAD_6,
		KEYBOARD_NUMPAD_7,
		KEYBOARD_NUMPAD_8,
		KEYBOARD_NUMPAD_9,
		KEYBOARD_NUMPAD_MUL,
		KEYBOARD_NUMPAD_ADD,
		KEYBOARD_NUMPAD_SUB,
		KEYBOARD_NUMPAD_DOT,
		KEYBOARD_NUMPAD_DIV,
		KEYBOARD_F1,
		KEYBOARD_F2,
		KEYBOARD_F3,
		KEYBOARD_F4,
		KEYBOARD_F5,
		KEYBOARD_F6,
		KEYBOARD_F7,
		KEYBOARD_F8,
		KEYBOARD_F9,
		KEYBOARD_F10,
		KEYBOARD_F11,
		KEYBOARD_F12,
		KEYBOARD_NUM_LOCK,
		KEYBOARD_SCROLL_LOCK,
		KEYBOARD_SHIFT_LEFT,
		KEYBOARD_SHIFT_RIGHT,
		KEYBOARD_CTRL_LEFT,
		KEYBOARD_CTRL_RIGHT,
		KEYBOARD_ALT_LEFT,
		KEYBOARD_ALT_RIGHT,
		KEYBOARD_SEMICOLON,
		KEYBOARD_EQUALS,
		KEYBOARD_COMMA,
		KEYBOARD_MINUS,
		KEYBOARD_PERIOD,
		KEYBOARD_SLASH,
		KEYBOARD_TILDE,
		KEYBOARD_BRACKET_LEFT,
		KEYBOARD_BACKSLASH,
		KEYBOARD_BRACKET_RIGHT,
		KEYBOARD_QUOTE,

		MAX_COUNT
	};


	constexpr const char* KEY_BUTTON_NAMES[MAX_COUNT] =
	{
		"NONE",
		"Mouse Left", "Mouse Middle", "Mouse Right", "Mouse 4", "Mouse 5",
		"Backspace", "Tab", "Enter", "Pause", "Caps Lock", "Escape", "Spacebar", "Page Up", "Page Down", "End", "Home",
		"Left", "Up", "Right", "Down", "Print Screen", "Insert", "Delete",
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", 
		"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
		"Numpad 0", "Numpad 1", "Numpad 2", "Numpad 3", "Numpad 4", "Numpad 5", "Numpad 6", "Numpad 7", "Numpad 8", "Numpad 9", "Numpad *", "Numpad +", "Numpad -", "Numpad .", "Numpad /",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
		"Num Lock", "Scroll Lock", "Left Shift", "Right Shift", "Left Ctrl", "Right Ctrl", "Alt Left", "Alt Right", 
		";", "=", ",", ".", "/", "~", "[", "\\", "]", "'"
	};

};



enum class RpgInputButtonState : uint8_t
{
	NONE,
	PRESSED,
	DOWN,
	RELEASED
};



enum class RpgInputContext : uint8_t
{
	NONE = 0,
	GAME,
	MENU
};
