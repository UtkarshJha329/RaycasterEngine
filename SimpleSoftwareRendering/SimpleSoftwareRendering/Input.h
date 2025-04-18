#pragma once

#include <vector>

/*
#define GLFW_KEY_A                  65
#define GLFW_KEY_B                  66
#define GLFW_KEY_C                  67
#define GLFW_KEY_D                  68
#define GLFW_KEY_E                  69
#define GLFW_KEY_F                  70
#define GLFW_KEY_G                  71
#define GLFW_KEY_H                  72
#define GLFW_KEY_I                  73
#define GLFW_KEY_J                  74
#define GLFW_KEY_K                  75
#define GLFW_KEY_L                  76
#define GLFW_KEY_M                  77
#define GLFW_KEY_N                  78
#define GLFW_KEY_O                  79
#define GLFW_KEY_P                  80
#define GLFW_KEY_Q                  81
#define GLFW_KEY_R                  82
#define GLFW_KEY_S                  83
#define GLFW_KEY_T                  84
#define GLFW_KEY_U                  85
#define GLFW_KEY_V                  86
#define GLFW_KEY_W                  87
#define GLFW_KEY_X                  88
#define GLFW_KEY_Y                  89
#define GLFW_KEY_Z                  90
*/

enum KeyCode {
	
	// Keyboard buttons.
	KEY_W					= 87,
	KEY_S					= 83,
	KEY_D					= 68,
	KEY_A					= 65,
	KEY_LEFT_SHIFT			= 340,
	KEY_LEFT_CTRL			= 341,
	KEY_SPACE				= 32,
	KEY_P					= 80,
	KEY_I					= 73,
	KEY_O					= 79,
	// Mouse buttons
	MOUSE_BUTTON_LEFT		= 0,
	MOUSE_BUTTON_RIGHT		= 1
};

enum KeyAction {
	PRESSED_OR_HELD,
	RELEASED
};

// Turn into map.
int KeyIndex(KeyCode keyCode) {

	if (keyCode == MOUSE_BUTTON_LEFT) {
		return 0;
	}
	if (keyCode == MOUSE_BUTTON_RIGHT) {
		return 1;
	}
	if (keyCode == KEY_W) {
		return 2;
	}
	if (keyCode == KEY_S) {
		return 3;
	}
	if (keyCode == KEY_D) {
		return 4;
	}
	if (keyCode == KEY_A) {
		return 5;
	}
	if (keyCode == KEY_LEFT_SHIFT) {
		return 6;
	}
	if (keyCode == KEY_LEFT_CTRL) {
		return 7;
	}
	if (keyCode == KEY_SPACE) {
		return 8;
	}
	if (keyCode == KEY_P) {
		return 9;
	}
	if (keyCode == KEY_I) {
		return 10;
	}
	if (keyCode == KEY_O) {
		return 11;
	}
}

constexpr int numKeys = 12;

std::vector<bool> keyPressedInThisFrame(numKeys);
std::vector<bool> keyHeld(numKeys);
std::vector<bool> keyReleasedInThisFrame(numKeys);

double mouseX, mouseXFromPreviousFrame = 0.0;
double mouseY, mouseYFromPreviousFrame = 0.0;

bool GetKeyPressedInThisFrame(KeyCode keyCode) {
	return keyPressedInThisFrame[KeyIndex(keyCode)];
}

bool GetKeyHeld(KeyCode keyCode) {
	return keyHeld[KeyIndex(keyCode)];
}

bool GetKeyReleasedInThisFrame(KeyCode keyCode) {
	return keyReleasedInThisFrame[KeyIndex(keyCode)];
}

void SetKeyPressedInThisFrame(KeyCode keyCode, bool value) {
	keyPressedInThisFrame[KeyIndex(keyCode)] = value;
}

void SetKeyHeld(KeyCode keyCode, bool value) {
	keyHeld[KeyIndex(keyCode)] = value;
}

void SetKeyReleasedInThisFrame(KeyCode keyCode, bool value) {
	keyReleasedInThisFrame[KeyIndex(keyCode)] = value;
}

void SetKeyBasedOnState(KeyCode keyCode, KeyAction action) {

	if (action == PRESSED_OR_HELD)
	{
		if (GetKeyPressedInThisFrame(keyCode) || GetKeyHeld(keyCode))
		{
			SetKeyHeld(keyCode, true);
			SetKeyPressedInThisFrame(keyCode, false);
		}
		else if (!GetKeyPressedInThisFrame(keyCode))
		{
			SetKeyPressedInThisFrame(keyCode, true);
		}
	}
	else if (action == RELEASED) {
		if (GetKeyHeld(keyCode) || GetKeyPressedInThisFrame(keyCode)) {
			SetKeyPressedInThisFrame(keyCode, false);
			SetKeyHeld(keyCode, false);
			SetKeyReleasedInThisFrame(keyCode, true);
		}
	}
}

void ResetKeysReleased() {
	keyReleasedInThisFrame[KeyIndex(KEY_W)] = false;
	keyReleasedInThisFrame[KeyIndex(KEY_S)] = false;
	keyReleasedInThisFrame[KeyIndex(KEY_D)] = false;
	keyReleasedInThisFrame[KeyIndex(KEY_A)] = false;
	keyReleasedInThisFrame[KeyIndex(KEY_LEFT_SHIFT)] = false;
	keyReleasedInThisFrame[KeyIndex(KEY_LEFT_CTRL)] = false;
	keyReleasedInThisFrame[KeyIndex(KEY_SPACE)] = false;
	keyReleasedInThisFrame[KeyIndex(KEY_P)] = false;
	keyReleasedInThisFrame[KeyIndex(KEY_I)] = false;
	keyReleasedInThisFrame[KeyIndex(KEY_O)] = false;

	keyReleasedInThisFrame[KeyIndex(MOUSE_BUTTON_LEFT)] = false;
	keyReleasedInThisFrame[KeyIndex(MOUSE_BUTTON_RIGHT)] = false;
}