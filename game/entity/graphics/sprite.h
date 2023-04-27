#ifndef _GAME_ENTITY_GRAPHICS_SPRITE_H
#define _GAME_ENTITY_GRAPHICS_SPRITE_H

#include "../component.h"
#include "../../../gfx/3d/material.h"

namespace game
{
    class SpriteGraphics:
        virtual public GraphicsComponent
    {
    public:
        float
            opacity,
            fading_speed,
            angular_speed;
        
        SpriteGraphics();
        SpriteGraphics(const SpriteGraphics &graphics);
        SpriteGraphics(const char *texture, int animation_frames = 1, float size = 1.0f, float fading_speed = .01f, float angular_speed = .001f);
        virtual ~SpriteGraphics();

        virtual void
        render(void);
    
        virtual void
        update(void);
    
        virtual SpriteGraphics *
        clone(void);

    protected:
        gfx::Material *material;
        float size;
        int animation_frames;
    };
}

#endif
