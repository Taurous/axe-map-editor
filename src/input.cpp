#include "input.hpp"

#include <iostream> // For std::cerr

InputHandler::InputHandler() : m_flags(INPUT::MOD::NONE), m_char_pressed(0)
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
	m_char_pressed = 0;

	m_prev_mouse_state = m_cur_mouse_state;
	al_get_mouse_state(&m_cur_mouse_state);

	m_prev_key_state = m_cur_key_state;
	al_get_keyboard_state(&m_cur_key_state);

	if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
	{
		// Get Modifiers Pressed
		switch (ev.keyboard.keycode)
		{
		case ALLEGRO_KEY_LSHIFT:
		case ALLEGRO_KEY_RSHIFT:
			m_flags.set(INPUT::MOD::SHIFT);
			m_flags.reset(INPUT::MOD::NONE);
			break;
		case ALLEGRO_KEY_ALTGR:
		case ALLEGRO_KEY_ALT:
			m_flags.set(INPUT::MOD::ALT);
			m_flags.reset(INPUT::MOD::NONE);
			break;
		case ALLEGRO_KEY_LCTRL:
		case ALLEGRO_KEY_RCTRL:
			m_flags.set(INPUT::MOD::CTRL);
			m_flags.reset(INPUT::MOD::NONE);
			break;
		default:
			break;
		};
	}

	// Clear Modifiers Released
	else if (ev.type == ALLEGRO_EVENT_KEY_UP)
	{
		switch (ev.keyboard.keycode)
		{
		case ALLEGRO_KEY_LSHIFT:
		case ALLEGRO_KEY_RSHIFT:
			m_flags.reset(INPUT::MOD::SHIFT);
			break;
		case ALLEGRO_KEY_ALTGR:
		case ALLEGRO_KEY_ALT:
			m_flags.reset(INPUT::MOD::ALT);
			break;
		case ALLEGRO_KEY_LCTRL:
		case ALLEGRO_KEY_RCTRL:
			m_flags.reset(INPUT::MOD::CTRL);
			break;
		default:
			break;
		};

		if (m_flags.none()) m_flags.set(INPUT::MOD::NONE);
	}

	// Catch unicode key pressed
	else if (ev.type == ALLEGRO_EVENT_KEY_CHAR)
	{
		m_char_pressed = ev.keyboard.unichar;
	}
}

char InputHandler::getChar() const
{
	return m_char_pressed;
}

bool InputHandler::isKeyPressed(const int key, const int mod) const
{
	if (al_key_down(&m_cur_key_state, key) && !al_key_down(&m_prev_key_state, key))
	{
		if (mod == INPUT::MOD::ANY || m_flags[mod]) return true;
	}

	return false;
}
bool InputHandler::isKeyReleased(const int key, const int mod) const
{
	if (al_key_down(&m_prev_key_state, key) && !al_key_down(&m_cur_key_state, key))
	{
		if (mod == INPUT::MOD::ANY || m_flags[mod]) return true;
	}

	return false;
}
bool InputHandler::isKeyDown(const int key, const int mod) const
{
	if (al_key_down(&m_cur_key_state, key))
	{
		if (mod == INPUT::MOD::ANY || m_flags[mod]) return true;
	}

	return false;
}

bool InputHandler::isMousePressed(const int button, const int mod) const
{
	if (al_mouse_button_down(&m_cur_mouse_state, button) && !al_mouse_button_down(&m_prev_mouse_state, button))
	{
		if (mod == INPUT::MOD::ANY || m_flags[mod]) return true;
	}

	return false;
}
bool InputHandler::isMouseReleased(const int button, const int mod) const
{
	if (al_mouse_button_down(&m_prev_mouse_state, button) && !al_mouse_button_down(&m_cur_mouse_state, button))
	{
		if (mod == INPUT::MOD::ANY || m_flags[mod]) return true;
	}

	return false;
}
bool InputHandler::isMouseDown(const int button, const int mod) const
{
	if (al_mouse_button_down(&m_cur_mouse_state, button))
	{
		if (mod == INPUT::MOD::ANY || m_flags[mod]) return true;
	}

	return false;
}

bool InputHandler::isMouseWheelDown(const int mod) const
{
	if (m_cur_mouse_state.z < m_prev_mouse_state.z && (mod == INPUT::MOD::ANY || m_flags[mod]))
	{
		return true;
	}
	else return false;
}
bool InputHandler::isMouseWheelUp(const int mod) const
{
	if (m_cur_mouse_state.z > m_prev_mouse_state.z && (mod == INPUT::MOD::ANY || m_flags[mod]))
	{
		return true;
	}
	else return false;
}

vec2f InputHandler::getMousePos(void) const
{
	return vec2f(m_cur_mouse_state.x, m_cur_mouse_state.y);
}
bool InputHandler::isMouseInWindow(void) const
{
	return (m_cur_mouse_state.display == al_get_current_display());
}

bool InputHandler::isModifierDown(const int mod)
{
	return m_flags.test(mod);
}
