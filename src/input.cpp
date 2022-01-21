#include "input.hpp"

#include <iostream> // For std::cerr

InputHandler::InputHandler() : mods(0)
{
	if (!al_is_system_installed())
	{
		std::cerr << "input - Allegro has not been installed!" << std::endl;
		abort();
	}

	al_install_keyboard();
	al_install_mouse();

	al_get_mouse_state(&m_prev_mouse_state);
	m_cur_mouse_state = m_prev_mouse_state;

	al_get_keyboard_state(&m_prev_key_state);
	m_cur_key_state = m_prev_key_state;
}

InputHandler::~InputHandler()
{
	al_uninstall_keyboard();
	al_uninstall_mouse();
}

void InputHandler::getInput(const ALLEGRO_EVENT &ev)
{
	m_prev_mouse_state = m_cur_mouse_state;
	al_get_mouse_state(&m_cur_mouse_state);

	m_prev_key_state = m_cur_key_state;
	al_get_keyboard_state(&m_cur_key_state);

	switch (ev.type)
	{
		case ALLEGRO_EVENT_MOUSE_AXES:
			if (ev.mouse.dz > 0) callKeybind(MOUSE::WHEELUP, true);
			else if (ev.mouse.dz < 0) callKeybind(MOUSE::WHEELDOWN, true);
		break;

		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			callKeybind(ALLEGRO_KEY_MAX + ev.mouse.button, true);
		break;

		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			callKeybind(ALLEGRO_KEY_MAX + ev.mouse.button, false);
		break;

		case ALLEGRO_EVENT_KEY_DOWN:
			mods = ev.keyboard.modifiers;
			callKeybind(ev.keyboard.keycode, true);
		break;

		case ALLEGRO_EVENT_KEY_UP:
			mods = ev.keyboard.modifiers;
			callKeybind(ev.keyboard.keycode, false);
		break;

		default:
		break;
	};
}

bool InputHandler::isKeyPressed(const int key, const int mod) const
{
	if (al_key_down(&m_cur_key_state, key) && !al_key_down(&m_prev_key_state, key))
	{
		if (mod == -1 || mod & mods) return true;
	}

	return false;
}
bool InputHandler::isKeyReleased(const int key, const int mod) const
{
	if (al_key_down(&m_prev_key_state, key) && !al_key_down(&m_cur_key_state, key))
	{
		if (mod == -1 || mod & mods) return true;
	}

	return false;
}
bool InputHandler::isKeyDown(const int key, const int mod) const
{
	if (al_key_down(&m_cur_key_state, key))
	{
		if (mod == -1 || mod & mods) return true;
	}

	return false;
}

bool InputHandler::isMousePressed(int button, const int mod) const
{
	button -= ALLEGRO_KEY_MAX;
	if (al_mouse_button_down(&m_cur_mouse_state, button) && !al_mouse_button_down(&m_prev_mouse_state, button))
	{
		if (mod == -1 || mod & mods) return true;
	}

	return false;
}
bool InputHandler::isMouseReleased(int button, const int mod) const
{
	button -= ALLEGRO_KEY_MAX;
	if (al_mouse_button_down(&m_prev_mouse_state, button) && !al_mouse_button_down(&m_cur_mouse_state, button))
	{
		if (mod == -1 || mod & mods) return true;
	}

	return false;
}
bool InputHandler::isMouseDown(int button, const int mod) const
{
	button -= ALLEGRO_KEY_MAX;
	if (al_mouse_button_down(&m_cur_mouse_state, button))
	{
		if (mod == -1 || mod & mods) return true;
	}

	return false;
}

bool InputHandler::isMouseWheelDown(const int mod) const
{
	if (m_cur_mouse_state.z < m_prev_mouse_state.z && (mod == -1 || mod & mods))
	{
		return true;
	}
	else return false;
}
bool InputHandler::isMouseWheelUp(const int mod) const
{
	if (m_cur_mouse_state.z > m_prev_mouse_state.z && (mod == -1 || mod & mods))
	{
		return true;
	}
	else return false;
}

vec2i InputHandler::getMousePos(void) const
{
	return vec2i{m_cur_mouse_state.x, m_cur_mouse_state.y};
}
bool InputHandler::isMouseInWindow(void) const
{
	return (m_cur_mouse_state.display == al_get_current_display()); // TODO: Test with multiple displays
}

bool InputHandler::isModifierDown(const int mod)
{
	return mod & mods;
}

void InputHandler::setKeybind(int key, std::function<void(void)> callback, bool pressed)
{
	if (pressed) keybinds_p[key] = callback;
	else keybinds_r[key] = callback;
}

void InputHandler::clearKeybind(int key)
{
	//Untested
	auto it = keybinds_p.find(key);
	if (it != keybinds_p.end())
	{
		keybinds_p.erase(it);
		return;
	}
	else
	{
		it = keybinds_r.find(key);
		if (it != keybinds_r.end())
		{
			keybinds_r.erase(it);
		}
	}
}

void InputHandler::callKeybind(int key, bool pressed)
{
	if (pressed)
	{
		if (keybinds_p.find(key) != keybinds_p.end())
		{
			keybinds_p[key]();
			//if (key < ALLEGRO_KEY_MAX) std::cout << "Keybind Pressed: " << al_keycode_to_name(key) << "\n";
		}
	}
	else
	{
		if (keybinds_r.find(key) != keybinds_r.end())
		{
			keybinds_r[key]();
			//if (key < ALLEGRO_KEY_MAX) std::cout << "Keybind Released: " << al_keycode_to_name(key) << "\n";
		}
	}
}