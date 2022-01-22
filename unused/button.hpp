#pragma once

#include <string>
#include <functional>

#include <allegro5/allegro.h>

#include "vec.hpp"

enum BTN_STATE
{
    DEFAULT,
    HOVERED,
    PRESSED
};

class Button
{
public:
    Button() { }
    Button(vec2i position, vec2i size, std::string label, ALLEGRO_COLOR border_cl, ALLEGRO_COLOR background_cl, ALLEGRO_COLOR text_cl, ALLEGRO_FONT* font) :
        m_state(BTN_STATE::DEFAULT), m_position(position), m_size(size), m_label(label), m_border_cl(border_cl), m_background_cl(background_cl), m_text_cl(text_cl), m_font(font)
    { }
    ~Button() { }

    void operator()()
    {
        m_fn();
    }

    void setCallback(std::function<void(void)> fn)
    {
        m_fn = fn;
    }
    void setState(int state) {m_state = state;}

    bool isMouseHovering(vec2i mouse_position)
    {
        return mouse_position.isInBounds(m_position, m_position + m_size);
    }

    void draw()
    {
        vec2i offset{0, 0};

        switch (m_state)
        {
            case BTN_STATE::DEFAULT:
            break;
            case BTN_STATE::HOVERED:
            break;
            case BTN_STATE::PRESSED:
                offset = {1, 3};
            break;
            default:
            break;
        }

        vec2i tl = m_position + offset;
        vec2i br = m_position + m_size + offset;

        al_draw_filled_rectangle(tl.x, tl.y, br.x, br.y, m_background_cl);
        al_draw_rectangle(tl.x, tl.y, br.x, br.y, m_border_cl, 2);
        al_draw_text(m_font, m_text_cl, m_position.x + (m_size.x / 2) + offset.x, m_position.y + (m_size.y / 2) - (al_get_font_line_height(m_font) / 2) + offset.y, ALLEGRO_ALIGN_CENTER, m_label.c_str());
    }
private:
    int m_state;
    vec2i m_position;
    vec2i m_size;
    std::string m_label;
    ALLEGRO_COLOR m_border_cl;
    ALLEGRO_COLOR m_background_cl;
    ALLEGRO_COLOR m_text_cl;
    ALLEGRO_FONT* m_font;

    std::function<void(void)> m_fn;
};