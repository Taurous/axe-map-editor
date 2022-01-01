#pragma once

#include <bitset>

#include <allegro5/allegro.h>

#include "vec.hpp"

namespace INPUT
{
	enum MOUSE
	{
		LEFT = 1,
		RIGHT = 2,
		MIDDLE = 3
	};

	enum MOD
	{
		NONE, // Requires that no modifiers be pressed
		SHIFT,
		CTRL,
		ALT,
		ANY // Ignores modifiers that are pressed (Allowing any modifier to be pressed)
	};
}

class InputHandler
{
public:
	InputHandler();

	InputHandler(const InputHandler &other) = delete;
	InputHandler &operator=(const InputHandler &other) = delete;

	InputHandler(InputHandler &&other) = delete;
	InputHandler &operator=(InputHandler &&other) = delete;

	~InputHandler();

	void getInput(const ALLEGRO_EVENT &ev);

	char getChar() const;

	bool isKeyPressed(const int key, const int mod = INPUT::MOD::ANY) const;
	bool isKeyReleased(const int key, const int mod = INPUT::MOD::ANY) const;
	bool isKeyDown(const int key, const int mod = INPUT::MOD::ANY) const;

	bool isMousePressed(const int button, const int mod = INPUT::MOD::ANY) const;
	bool isMouseReleased(const int button, const int mod = INPUT::MOD::ANY) const;
	bool isMouseDown(const int button, const int mod = INPUT::MOD::ANY) const;
	bool isMouseWheelDown(const int mod = INPUT::MOD::ANY) const;
	bool isMouseWheelUp(const int mod = INPUT::MOD::ANY) const;

	bool isModifierDown(const int mod);

	vec2f getMousePos() const;
	bool isMouseInWindow() const;
	
private:
	char m_char_pressed;
	std::bitset<4> m_flags;

	ALLEGRO_KEYBOARD_STATE m_prev_key_state;
	ALLEGRO_KEYBOARD_STATE m_cur_key_state;

	ALLEGRO_MOUSE_STATE m_prev_mouse_state;
	ALLEGRO_MOUSE_STATE m_cur_mouse_state;
};
