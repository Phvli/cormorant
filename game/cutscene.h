#ifndef _GAME_CUTSCENE_H
#define _GAME_CUTSCENE_H

#include <vector>

#include "../gfx/sprite.h"

namespace game
{
    class Cutscene
    {
    public:
        class Page
        {
        public:
            std::vector<char *>
                lines;

            float
                line_spacing;
            
            float width;
            
            gfx::Sprite
                *image;
            
            Page();
            ~Page();
            
            void
            render(int reveal_threshold);
        };
        
        Cutscene();
        Cutscene(const char *name);
        ~Cutscene();
        
        bool
        load(const char *name);
        
        void
        run(void);
    
    protected:
        char
            *name,
            *music;

        float
            text_speed,
            text_delay;
        
        int
            page_turn_delay;

        std::vector<Page *>
            pages;
    };
}

#endif
