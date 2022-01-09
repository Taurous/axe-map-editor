#pragma once

#include <functional> //for std::function & std::bind
#include <map>

#include <allegro5/allegro.h>

#include "vec.hpp"

enum MOUSE
{
	LEFT = ALLEGRO_KEY_MAX + 1,
	RIGHT,
	MIDDLE,
	BUTTON4,
	BUTTON5,
	WHEELUP,
	WHEELDOWN
};

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

	bool isKeyPressed(const int key, const int mod = -1) const;
	bool isKeyReleased(const int key, const int mod = -1) const;
	bool isKeyDown(const int key, const int mod = -1) const;

	bool isMousePressed(int button, const int mod = -1) const;
	bool isMouseReleased(int button, const int mod = -1) const;
	bool isMouseDown(int button, const int mod = -1) const;
	bool isMouseWheelDown(const int mod = -1) const;
	bool isMouseWheelUp(const int mod = -1) const;

	bool isModifierDown(const int mod);

	vec2f getMousePos() const;
	bool isMouseInWindow() const;

	void setKeybind(int key, std::function<void(void)> callback, bool pressed = true);
	void clearKeybind(int key);
	void callKeybind(int key, bool pressed);
	
private:
	int mods;
	
	ALLEGRO_KEYBOARD_STATE m_prev_key_state;
	ALLEGRO_KEYBOARD_STATE m_cur_key_state;

	ALLEGRO_MOUSE_STATE m_prev_mouse_state;
	ALLEGRO_MOUSE_STATE m_cur_mouse_state;

	std::map<int, std::function<void(void)> > keybinds_r;
	std::map<int, std::function<void(void)> > keybinds_p;
};
