#ifndef _GFX_FONT_H
#define _GFX_FONT_H

#include <GL/gl.h>

// #include "3d/texture.h"
#include "sprite.h"

namespace gfx
{
    class Font;
    class Font
    {
    public:
        static Font *
        get(const char *name = NULL, float size = 8.0f, gfx::Color stroke = 0xffffffff, gfx::Color background = 0x00000000, gfx::Color effect = 0xff808080);
        // Returns a font instance. Loads font bitmap if not loaded yet.
        // Caller must delete returned instance.

        float
            x, y,
            size;
            
        struct
        {
            gfx::Color
                stroke,
                background,
                effect;
        } color;

        unsigned long int
            effects;
        
        static const unsigned long int
            BACKGROUND = (0x00000001 << 0),
            BOLD       = (0x00000001 << 1),
            ITALIC     = (0x00000001 << 2),
            UNDERLINE  = (0x00000001 << 3),
            MONOSPACE  = (0x00000001 << 4),
            SHADOW     = (0x00000001 << 5),
            OUTLINE    = (0x00000001 << 6),

            NONE       =  0x00000000;


        typedef
            struct
            {
                float
                    width,
                    start,
                    end;
            }
            Character;

        Font();
        Font(const char *name, float size = 8.0f, gfx::Color stroke = 0xffffffff, gfx::Color background = 0x00000000, gfx::Color effect = 0xff808080);

        Font *
        prepare(bool setup_font_rendering = true);
        // Sets OpenGL in such a mode that rendering text on screen is possible
        // Disables depth testing and enables transparency
        // Restores previous state if setup_font_rendering is false

        inline Font *
        at(float x) { this->x = x; return this; }
        // Moves cursor at (x, y) pixels

        inline Font *
        at(float x, float y) { this->x = x; this->y = y; return this; }
        // Moves cursor at (x, y) pixels

        inline Font *
        at(float x, float y, gfx::Color stroke) { this->x = x; this->y = y; this->color.stroke = stroke | (0xff000000 * ((stroke & 0xff000000) == 0)); return this; }
        // Moves cursor at (x, y) pixels and sets font color

        inline Font *
        at(float x, float y, gfx::Color stroke, gfx::Color background) { this->x = x; this->y = y; this->color.stroke = stroke | (0xff000000 * ((stroke & 0xff000000) == 0)); this->color.background = background | (0xff000000 * ((background & 0xff000000) == 0)); return this; }
        // Moves cursor at (x, y) pixels and sets font color

        inline Font *
        at(float x, float y, gfx::Color stroke, gfx::Color background, gfx::Color effect) { this->x = x; this->y = y; this->color.stroke = stroke | (0xff000000 * ((stroke & 0xff000000) == 0)); this->color.background = background | (0xff000000 * ((background & 0xff000000) == 0)); this->color.effect = effect | (0xff000000 * ((effect & 0xff000000) == 0)); return this; }
        // Moves cursor at (x, y) pixels and sets font color

        // Convenience functions for setting properties:
        // (alpha automatically set to 0xff if omitted)
        inline Font *set_color(gfx::Color stroke) { this->color.stroke = stroke | (0xff000000 * ((stroke & 0xff000000) == 0)); return this; }
        inline Font *set_color(gfx::Color stroke, gfx::Color background) { this->color.stroke = stroke | (0xff000000 * ((stroke & 0xff000000) == 0)); this->color.background = background | (0xff000000 * ((background & 0xff000000) == 0)); return this; }
        inline Font *set_color(gfx::Color stroke, gfx::Color background, gfx::Color effect) { this->color.stroke = stroke | (0xff000000 * ((stroke & 0xff000000) == 0)); this->color.background = background | (0xff000000 * ((background & 0xff000000) == 0)); this->color.effect = effect | (0xff000000 * ((effect & 0xff000000) == 0)); return this; }
        inline Font *set_background(gfx::Color background) { this->color.background = background | (0xff000000 * ((background & 0xff000000) == 0)); this->effects |= Font::BACKGROUND; return this; }
        inline Font *set_effect_color(gfx::Color effect) { this->color.effect = effect | (0xff000000 * ((effect & 0xff000000) == 0)); return this; }
        inline Font *set_size(float size) { this->size = size; return this; }
        inline Font *add_effect(unsigned long int effect) { this->effects |= effect; return this; }
        inline Font *add_effect(unsigned long int effect, gfx::Color color) { this->effects |= effect; this->color.effect = color | (0xff000000 * ((color & 0xff000000) == 0)); return this; }
        inline Font *remove_effect(unsigned long int effect) { this->effects &= ~effect; return this; }
        inline Font *clear_effects(unsigned long int effects = 0) { this->effects = effects; return this; }

        Font *
        write(const char *format, ...);
        // Draws formatted text at current cursor location and color

        Font *
        center(const char *format, ...);
        // Draws formatted text centered at current cursor location

        Font *
        align_left(const char *format, ...);
        // Same as write

        Font *
        align_middle(const char *format, ...);
        // Draws formatted text centered to bottom at current cursor location

        Font *
        align_right(const char *format, ...);
        // Draws formatted text ending at current cursor location

        float
        get_width(const char *s) const;

        float
        get_height(const char *s) const;

        const Character &
        operator[](char c) const;
        
    protected:
        // Texture *texture;
        GLuint
            texture;

        bool
        load(const char *name);

        Character
            *character;
    };
}

#endif
