#pragma once

#include "RpgInputTypes.h"



namespace RpgInputWindows
{
	constexpr WPARAM KEY_BUTTON_TO_VK[] =
	{
		0,				// NONE
		VK_LBUTTON,		// MOUSE_BUTTON_LEFT
		VK_RBUTTON,		// MOUSE_BUTTON_RIGHT
		VK_MBUTTON,		// MOUSE_BUTTON_MIDDLE
		VK_XBUTTON1,	// MOUSE_BUTTON_4,
		VK_XBUTTON2,	// MOUSE_BUTTON_5,
		VK_BACK,		// KEYBOARD_BACKSPACE
		VK_TAB,			// KEYBOARD_TAB
		VK_RETURN,		// KEYBOARD_ENTER
		VK_PAUSE,		// KEYBOARD_PAUSE
		VK_CAPITAL,		// KEYBOARD_CAPS_LOCK
		VK_ESCAPE,		// KEYBOARD_ESCAPE
		VK_SPACE,		// KEYBOARD_SPACEBAR
		VK_PRIOR,		// KEYBOARD_PAGE_UP
		VK_NEXT,		// KEYBOARD_PAGE_DOWN
		VK_END,			// KEYBOARD_END
		VK_HOME,		// KEYBOARD_HOME
		VK_LEFT,		// KEYBOARD_LEFT
		VK_UP,			// KEYBOARD_UP
		VK_RIGHT,		// KEYBOARD_RIGHT
		VK_DOWN,		// KEYBOARD_DOWN
		VK_PRINT,		// KEYBOARD_PRINT_SCREEN
		VK_INSERT,		// KEYBOARD_INSERT
		VK_DELETE,		// KEYBOARD_DELETE
		0x30,			// KEYBOARD_0
		0x31,			// KEYBOARD_1
		0x32,			// KEYBOARD_2
		0x33,			// KEYBOARD_3
		0x34,			// KEYBOARD_4
		0x35,			// KEYBOARD_5
		0x36,			// KEYBOARD_6
		0x37,			// KEYBOARD_7
		0x38,			// KEYBOARD_8
		0x39,			// KEYBOARD_9
		0x41,			// KEYBOARD_A
		0x42,			// KEYBOARD_B
		0x43,			// KEYBOARD_C
		0x44,			// KEYBOARD_D
		0x45,			// KEYBOARD_E
		0x46,			// KEYBOARD_F
		0x47,			// KEYBOARD_G
		0x48,			// KEYBOARD_H
		0x49,			// KEYBOARD_I
		0x4A,			// KEYBOARD_J
		0x4B,			// KEYBOARD_K
		0x4C,			// KEYBOARD_L
		0x4D,			// KEYBOARD_M
		0x4E,			// KEYBOARD_N
		0x4F,			// KEYBOARD_O
		0x50,			// KEYBOARD_P
		0x51,			// KEYBOARD_Q
		0x52,			// KEYBOARD_R
		0x53,			// KEYBOARD_S
		0x54,			// KEYBOARD_T
		0x55,			// KEYBOARD_U
		0x56,			// KEYBOARD_V
		0x57,			// KEYBOARD_W
		0x58,			// KEYBOARD_X
		0x59,			// KEYBOARD_Y
		0x5A,			// KEYBOARD_Z
		VK_NUMPAD0,		// KEYBOARD_NUMPAD_0
		VK_NUMPAD1,		// KEYBOARD_NUMPAD_1
		VK_NUMPAD2,		// KEYBOARD_NUMPAD_2
		VK_NUMPAD3,		// KEYBOARD_NUMPAD_3
		VK_NUMPAD4,		// KEYBOARD_NUMPAD_4
		VK_NUMPAD5,		// KEYBOARD_NUMPAD_5
		VK_NUMPAD6,		// KEYBOARD_NUMPAD_6
		VK_NUMPAD7,		// KEYBOARD_NUMPAD_7
		VK_NUMPAD8,		// KEYBOARD_NUMPAD_8
		VK_NUMPAD9,		// KEYBOARD_NUMPAD_9
		VK_MULTIPLY,	// KEYBOARD_NUMPAD_MUL
		VK_ADD,			// KEYBOARD_NUMPAD_ADD
		VK_SUBTRACT,	// KEYBOARD_NUMPAD_SUB
		VK_DECIMAL,		// KEYBOARD_NUMPAD_DOT
		VK_DIVIDE,		// KEYBOARD_NUMPAD_DIV
		VK_F1,			// KEYBOARD_F1
		VK_F2,			// KEYBOARD_F2
		VK_F3,			// KEYBOARD_F3
		VK_F4,			// KEYBOARD_F4
		VK_F5,			// KEYBOARD_F5
		VK_F6,			// KEYBOARD_F6
		VK_F7,			// KEYBOARD_F7
		VK_F8,			// KEYBOARD_F8
		VK_F9,			// KEYBOARD_F9
		VK_F10,			// KEYBOARD_F10
		VK_F11,			// KEYBOARD_F11
		VK_F12,			// KEYBOARD_F12
		VK_NUMLOCK,		// KEYBOARD_NUM_LOCK
		VK_SCROLL,		// KEYBOARD_SCROLL_LOCK
		VK_LSHIFT,		// KEYBOARD_SHIFT_LEFT
		VK_RSHIFT,		// KEYBOARD_SHIFT_RIGHT
		VK_LCONTROL,	// KEYBOARD_CTRL_LEFT
		VK_RCONTROL,	// KEYBOARD_CTRL_RIGHT
		VK_LMENU,		// KEYBOARD_ALT_LEFT
		VK_RMENU,		// KEYBOARD_ALT_RIGHT
		VK_OEM_1,		// KEYBOARD_SEMICOLON
		VK_OEM_PLUS,	// KEYBOARD_EQUALS
		VK_OEM_COMMA,	// KEYBOARD_COMMA
		VK_OEM_MINUS,	// KEYBOARD_MINUS
		VK_OEM_PERIOD,	// KEYBOARD_PERIOD
		VK_OEM_2,		// KEYBOARD_SLASH
		VK_OEM_3,		// KEYBOARD_TILDE
		VK_OEM_4,		// KEYBOARD_BRACKET_LEFT
		VK_OEM_5,		// KEYBOARD_SEPARATOR
		VK_OEM_6,		// KEYBOARD_BRACKET_RIGHT
		VK_OEM_7,		// KEYBOARD_QUOTE
	};
	static_assert(sizeof(KEY_BUTTON_TO_VK) / sizeof(WPARAM) == RpgInputKey::MAX_COUNT, "KEY_BUTTON_TO_VK invalid mappings!");



	constexpr RpgInputKey::EButton SC_TO_KEY_BUTTON[] =
	{
		RpgInputKey::NONE,
		RpgInputKey::KEYBOARD_ESCAPE,	
		RpgInputKey::KEYBOARD_1,
		RpgInputKey::KEYBOARD_2,
		RpgInputKey::KEYBOARD_3,
		RpgInputKey::KEYBOARD_4,
		RpgInputKey::KEYBOARD_5,
		RpgInputKey::KEYBOARD_6,
		RpgInputKey::KEYBOARD_7,
		RpgInputKey::KEYBOARD_8,
		RpgInputKey::KEYBOARD_9,
		RpgInputKey::KEYBOARD_0,
		RpgInputKey::KEYBOARD_MINUS,
		RpgInputKey::KEYBOARD_EQUALS,
		RpgInputKey::KEYBOARD_BACKSPACE,
		RpgInputKey::KEYBOARD_TAB,
		RpgInputKey::KEYBOARD_Q,
		RpgInputKey::KEYBOARD_W,
		RpgInputKey::KEYBOARD_E,
		RpgInputKey::KEYBOARD_R,
		RpgInputKey::KEYBOARD_T,
		RpgInputKey::KEYBOARD_Y,
		RpgInputKey::KEYBOARD_U,
		RpgInputKey::KEYBOARD_I,
		RpgInputKey::KEYBOARD_O,
		RpgInputKey::KEYBOARD_P,
		RpgInputKey::KEYBOARD_BRACKET_LEFT,
		RpgInputKey::KEYBOARD_BRACKET_RIGHT,
		RpgInputKey::KEYBOARD_ENTER,
		RpgInputKey::KEYBOARD_CTRL_LEFT,
		RpgInputKey::KEYBOARD_A,
		RpgInputKey::KEYBOARD_S,
		RpgInputKey::KEYBOARD_D,
		RpgInputKey::KEYBOARD_F,
		RpgInputKey::KEYBOARD_G,
		RpgInputKey::KEYBOARD_H,
		RpgInputKey::KEYBOARD_J,
		RpgInputKey::KEYBOARD_K,
		RpgInputKey::KEYBOARD_L,
		RpgInputKey::KEYBOARD_SEMICOLON,
		RpgInputKey::KEYBOARD_QUOTE,
		RpgInputKey::KEYBOARD_TILDE,
		RpgInputKey::KEYBOARD_SHIFT_LEFT,
		RpgInputKey::KEYBOARD_BACKSLASH,
		RpgInputKey::KEYBOARD_Z,
		RpgInputKey::KEYBOARD_X,
		RpgInputKey::KEYBOARD_C,
		RpgInputKey::KEYBOARD_V,
		RpgInputKey::KEYBOARD_B,
		RpgInputKey::KEYBOARD_N,
		RpgInputKey::KEYBOARD_M,
		RpgInputKey::KEYBOARD_COMMA,
		RpgInputKey::KEYBOARD_PERIOD,
		RpgInputKey::KEYBOARD_SLASH,
		RpgInputKey::KEYBOARD_SHIFT_RIGHT,
		RpgInputKey::KEYBOARD_NUMPAD_MUL,
		RpgInputKey::KEYBOARD_ALT_LEFT,
		RpgInputKey::KEYBOARD_SPACEBAR,
		RpgInputKey::KEYBOARD_CAPS_LOCK,
		RpgInputKey::KEYBOARD_F1,
		RpgInputKey::KEYBOARD_F2,
		RpgInputKey::KEYBOARD_F3,
		RpgInputKey::KEYBOARD_F4,
		RpgInputKey::KEYBOARD_F5,
		RpgInputKey::KEYBOARD_F6,
		RpgInputKey::KEYBOARD_F7,
		RpgInputKey::KEYBOARD_F8,
		RpgInputKey::KEYBOARD_F9,
		RpgInputKey::KEYBOARD_F10,
		RpgInputKey::KEYBOARD_NUM_LOCK,
		RpgInputKey::KEYBOARD_SCROLL_LOCK,
		RpgInputKey::KEYBOARD_NUMPAD_7,
		RpgInputKey::KEYBOARD_NUMPAD_8,
		RpgInputKey::KEYBOARD_NUMPAD_9,
		RpgInputKey::KEYBOARD_NUMPAD_SUB,
		RpgInputKey::KEYBOARD_NUMPAD_4,
		RpgInputKey::KEYBOARD_NUMPAD_5,
		RpgInputKey::KEYBOARD_NUMPAD_6,
		RpgInputKey::KEYBOARD_NUMPAD_ADD,
		RpgInputKey::KEYBOARD_NUMPAD_1,
		RpgInputKey::KEYBOARD_NUMPAD_2,
		RpgInputKey::KEYBOARD_NUMPAD_3,
		RpgInputKey::KEYBOARD_NUMPAD_0,
		RpgInputKey::KEYBOARD_NUMPAD_DOT,

		RpgInputKey::NONE,
		RpgInputKey::NONE,
		RpgInputKey::NONE,

		RpgInputKey::KEYBOARD_F11,
		RpgInputKey::KEYBOARD_F12
	};
	constexpr int SC_TO_KEY_BUTTON_COUNT = sizeof(SC_TO_KEY_BUTTON) / sizeof(uint8_t);



	constexpr uint8_t SC_EXT[] =
	{
		0x1C,	// KEYBOARD_NUMPAD_ENTER
		0x1D,	// KEYBOARD_CTRL_RIGHT
		0x35,	// KEYBOARD_NUMPAD_DIV
		0x37,	// KEYBOARD_PRINT_SCREEN
		0x38,	// KEYBOARD_ALT_RIGHT
		0x47,	// KEYBOARD_HOME
		0x48,	// KEYBOARD_UP
		0x49,	// KEYBOARD_PAGE_UP
		0x4B,	// KEYBOARD_LEFT
		0x4D,	// KEYBOARD_RIGHT
		0x4F,	// KEYBOARD_END
		0x50,	// KEYBOARD_DOWN
		0x51,	// KEYBOARD_PAGE_DOWN
		0x52,	// KEYBOARD_INSERT
		0x53,	// KEYBOARD_DELETE
		0x57,	// KEYBOARD_F11
		0x58,	// KEYBOARD_F12
	};
	constexpr int SC_EXT_COUNT = sizeof(SC_EXT) / sizeof(uint8_t);


	constexpr RpgInputKey::EButton SC_EXT_INDEX_TO_KEY_BUTTON[] =
	{
		RpgInputKey::KEYBOARD_ENTER,
		RpgInputKey::KEYBOARD_CTRL_RIGHT,
		RpgInputKey::KEYBOARD_NUMPAD_DIV,
		RpgInputKey::KEYBOARD_PRINT_SCREEN,
		RpgInputKey::KEYBOARD_ALT_RIGHT,
		RpgInputKey::KEYBOARD_HOME,
		RpgInputKey::KEYBOARD_UP,
		RpgInputKey::KEYBOARD_PAGE_UP,
		RpgInputKey::KEYBOARD_LEFT,
		RpgInputKey::KEYBOARD_RIGHT,
		RpgInputKey::KEYBOARD_END,
		RpgInputKey::KEYBOARD_DOWN,
		RpgInputKey::KEYBOARD_PAGE_DOWN,
		RpgInputKey::KEYBOARD_INSERT,
		RpgInputKey::KEYBOARD_DELETE,
		RpgInputKey::KEYBOARD_F11,
		RpgInputKey::KEYBOARD_F12
	};
	constexpr int SC_EXT_INDEX_TO_KEY_BUTTON_COUNT = sizeof(SC_EXT_INDEX_TO_KEY_BUTTON) / sizeof(RpgInputKey::EButton);


	static_assert(SC_EXT_COUNT == SC_EXT_INDEX_TO_KEY_BUTTON_COUNT, "SC_EXT and SC_EXT_INDEX_TO_KEY_BUTTON invalid mappings!");



	inline RpgInputKey::EButton MapVirtualKeyToKeyButton(WPARAM wParam, LPARAM lParam) noexcept
	{
		const uint8_t scanCode = (lParam >> 16) & 0xFF;
		bool bExtended = (lParam >> 24) & 1;

		if (scanCode == 0x45)
		{
			bExtended = false;
		}


	#ifndef RPG_BUILD_SHIPPING
		WPARAM virtualKey = wParam;

		if (wParam == VK_SHIFT)
		{
			virtualKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
		}
		else if (wParam == VK_CONTROL)
		{
			virtualKey = bExtended ? VK_RCONTROL : VK_LCONTROL;
		}
		else if (wParam == VK_MENU)
		{
			virtualKey = bExtended ? VK_RMENU : VK_LMENU;
		}

		int keyButtonIndex = RPG_INDEX_INVALID;
		for (int i = 0; i < RpgInputKey::MAX_COUNT; ++i)
		{
			if (KEY_BUTTON_TO_VK[i] == virtualKey)
			{
				keyButtonIndex = i;
				break;
			}
		}

		RPG_Check(keyButtonIndex >= 0 && keyButtonIndex < RpgInputKey::MAX_COUNT);

		const RpgInputKey::EButton keyButtonFromVk = static_cast<RpgInputKey::EButton>(keyButtonIndex);
		
		RpgInputKey::EButton keyButtonFromSc = RpgInputKey::NONE;

		if (bExtended)
		{
			int index = RPG_INDEX_INVALID;

			for (int i = 0; i < SC_EXT_COUNT; ++i)
			{
				if (SC_EXT[i] == scanCode)
				{
					index = i;
					break;
				}
			}

			RPG_Check(index != RPG_INDEX_INVALID);
			keyButtonFromSc = SC_EXT_INDEX_TO_KEY_BUTTON[index];
		}
		else
		{
			RPG_Check(scanCode < SC_TO_KEY_BUTTON_COUNT);
			keyButtonFromSc = SC_TO_KEY_BUTTON[scanCode];
		}


		RPG_Check(keyButtonFromSc != RpgInputKey::NONE);
		RPG_Check(keyButtonFromVk == keyButtonFromSc);

	#else
		RpgInputKey::EButton keyButtonFromSc = RpgInputKey::NONE;

		if (scanCode == 0x45)
		{
			bExtended = false;
		}

		if (bExtended)
		{
			int index = RPG_INDEX_INVALID;
			
			for (int i = 0; i < SC_EXT_COUNT; ++i)
			{
				if (SC_EXT[i] == scanCode)
				{
					index = i;
					break;
				}
			}

			if (index != RPG_INDEX_INVALID)
			{
				keyButtonFromSc = SC_EXT_INDEX_TO_KEY_BUTTON[index];
			}
		}
		else if (scanCode >= 0 && scanCode < SC_TO_KEY_BUTTON_COUNT)
		{
			keyButtonFromSc = SC_TO_KEY_BUTTON[scanCode];
		}

	#endif // !RPG_BUILD_SHIPPING


		return keyButtonFromSc;
	}

};
