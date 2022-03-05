#include "input.hpp"

#include <iostream> // For std::cerr

int getModifier(int keycode)
{
	switch (keycode)
	{
		case ALLEGRO_KEY_LSHIFT: // Fall through
		case ALLEGRO_KEY_RSHIFT:
			return ALLEGRO_KEYMOD_SHIFT;
		break;
		case ALLEGRO_KEY_LCTRL:
		case ALLEGRO_KEY_RCTRL:
			return ALLEGRO_KEYMOD_CTRL;
		break;
		case ALLEGRO_KEY_ALT:
			return ALLEGRO_KEYMOD_ALT;
		default:
		break;
	}

	return 0;
}

InputHandler::InputHandler() : modifiers(0)
{
	if (!al_is_system_installed())
	{
		std::cerr << "input - Allegro has not been installed!" << std::endl;
		abort();
	}

	al_install_keyboard();
	al_install_mouse();

	memset(keys_pressed, false, sizeof(bool) * CUSTOM_ALLEGRO_KEY_MAX);
	memset(keys_held, false, sizeof(bool) * CUSTOM_ALLEGRO_KEY_MAX);
	memset(keys_released, false, sizeof(bool) * CUSTOM_ALLEGRO_KEY_MAX);
}

InputHandler::~InputHandler()
{
	al_uninstall_keyboard();
	al_uninstall_mouse();
}

void InputHandler::getInput(const ALLEGRO_EVENT &ev)
{
	memset(keys_pressed, false, sizeof(bool) * CUSTOM_ALLEGRO_KEY_MAX);
	memset(keys_released, false, sizeof(bool) * CUSTOM_ALLEGRO_KEY_MAX);

	mouse_wheel_down = false;
	mouse_wheel_up = false;

	switch (ev.type)
	{
		case ALLEGRO_EVENT_MOUSE_AXES:
			if (ev.mouse.dz > 0)
			{
				callKeybind(MOUSE::WHEELUP, true);
				mouse_wheel_up = true;
			}
			else if (ev.mouse.dz < 0)
			{
				callKeybind(MOUSE::WHEELDOWN, true);
				mouse_wheel_down = true;
			}

			mouse_position = { ev.mouse.x, ev.mouse.y };
		break;

		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			callKeybind(ALLEGRO_KEY_MAX + ev.mouse.button, true);
			keys_pressed[ALLEGRO_KEY_MAX + ev.mouse.button] = true;
			keys_held[ALLEGRO_KEY_MAX + ev.mouse.button] = true;
		break;

		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			callKeybind(ALLEGRO_KEY_MAX + ev.mouse.button, false);
			keys_released[ALLEGRO_KEY_MAX + ev.mouse.button] = true;
			keys_held[ALLEGRO_KEY_MAX + ev.mouse.button] = false;
		break;

		case ALLEGRO_EVENT_KEY_DOWN:
			// Catch modifier keys
			modifiers |= getModifier(ev.keyboard.keycode);
			callKeybind(ev.keyboard.keycode, true);
			keys_pressed[ev.keyboard.keycode] = true;
			keys_held[ev.keyboard.keycode] = true;
			std::cout << "Shift Pressed: " << (ev.keyboard.modifiers & ALLEGRO_KEYMOD_SHIFT ? "true" : "false") <<  "\n"
				<< "\tKeymodifiers Bitfield " << ev.keyboard.modifiers << "\n";
		break;

		case ALLEGRO_EVENT_KEY_UP:
			modifiers &= ~getModifier(ev.keyboard.keycode);
			callKeybind(ev.keyboard.keycode, false);
			keys_released[ev.keyboard.keycode] = true;
			keys_held[ev.keyboard.keycode] = false;
		break;

		default:
		break;
	};
}

bool InputHandler::isKeyPressed(const int key, const int mod) const
{
	return keys_pressed[key] && (mod == -1 || mod & modifiers);
}
bool InputHandler::isKeyReleased(const int key, const int mod) const
{
	return keys_released[key] && (mod == -1 || mod & modifiers);
}
bool InputHandler::isKeyDown(const int key, const int mod) const
{
	return keys_held[key] && (mod == -1 || mod & modifiers);
}

bool InputHandler::isMousePressed(int button, const int mod) const
{
	return isKeyPressed(button, mod);
}
bool InputHandler::isMouseReleased(int button, const int mod) const
{
	return isKeyReleased(button, mod);
}
bool InputHandler::isMouseDown(int button, const int mod) const
{
	return isKeyDown(button, mod);
}

bool InputHandler::isMouseWheelDown(const int mod) const
{
	return mouse_wheel_down && (mod == -1 || mod & modifiers);
}
bool InputHandler::isMouseWheelUp(const int mod) const
{
	return mouse_wheel_up && (mod == -1 || mod & modifiers);
}

vec2i InputHandler::getMousePos(void) const
{
	return mouse_position;
}

bool InputHandler::isModifierDown(const int mod)
{
	return mod & modifiers;
}

void InputHandler::setKeybind(int key, std::function<void(void)> callback, bool pressed)
{
	if (pressed) keybinds_p[key] = callback;
	else keybinds_r[key] = callback;
}

void InputHandler::clearKeybind(int key)
{
	//TODO Untested
	auto it = keybinds_p.find(key);
	if (it != keybinds_p.end())
	{
		keybinds_p.erase(it);
		return;
	}

	it = keybinds_r.find(key);

	if (it != keybinds_r.end()) keybinds_r.erase(it);
}

void InputHandler::callKeybind(int key, bool pressed)
{
	if (pressed && keybinds_p.find(key) != keybinds_p.end())
	{
		keybinds_p[key]();
		return;
	}

	if (keybinds_r.find(key) != keybinds_r.end()) keybinds_r[key]();
}