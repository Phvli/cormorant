#ifndef _CORE_ENGINES_VIDEO_H
#define _CORE_ENGINES_VIDEO_H

#include <SDL2/SDL.h>

#include "core.h"
#include "../../gfx/sprite.h"
#include "../../gfx/3d/scene.h"

namespace core
{
    class VideoEngine:
        virtual public EngineCore,
        public EngineInterface
    {

    public:
        VideoEngine();
        ~VideoEngine();

        virtual const char *
        get_module_name(void) { return "video"; };

        virtual void
        initialize_module(void);

        virtual void
        update_tick(void);

        virtual void
        update_frame(void);
    };

    class Graphics:
        public gfx::Sprite
    {
    public:
        // (!) FIXME: use const references for READ-ONLIES
        bool fullscreen;        /* READ-ONLY, use set_fullscreen() instead */
        int detail_level;       /* READ-ONLY */
        
        SDL_Window    *window;    /* READ-ONLY */

#ifdef USE_OPENGL
        const int      &x, &y;
        SDL_GLContext gl_context; /* READ-ONLY */
        gfx::Scene    *scene;
#else
        SDL_Renderer  *renderer;  /* READ-ONLY */
        SDL_Texture   *texture;   /* READ-ONLY */
#endif

        Graphics();
        ~Graphics();

        void
        quit();

        void
        show();

        void inline
        resize(int w, int h) { this->resize(w, h, this->fullscreen); };

        void
        resize(int w, int h, bool fullscreen);

        void
        set_fullscreen(bool fullscreen = true);

    protected:
#ifdef USE_OPENGL
        int mutable_x, mutable_y;
#endif
    };
}


extern core::Graphics screen;

#endif
