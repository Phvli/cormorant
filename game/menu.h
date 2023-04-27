#ifndef _GAME_MENU_H
#define _GAME_MENU_H

#include <vector>

#include "../gfx/sprite.h"
#include "../gfx/font.h"
#include "../core/util/config.h"

namespace game
{
    class Option;

    class Menu
    {
    public:
        static const unsigned int
            LEFT     = (0x0001 << 0),
            RIGHT    = (0x0001 << 1),
            TOP      = (0x0001 << 2),
            BOTTOM   = (0x0001 << 3),
            CENTER_X = (0x0001 << 4),
            CENTER_Y = (0x0001 << 5),
            CENTER   = (CENTER_X | CENTER_Y);

        class Action
        {
        public:
            Action();
            ~Action();

            char *command;
            char **param;

            void
            execute(void);
        };

        class Option
        {
        public:
            Option();
            Option(const char *text, gfx::Color color = 0xffffffff, float x = 0.0f, float y = 0.0f);
            ~Option();

            void
            render(bool selected = false) const;

            bool
            select(void);

            char
                *text;

            float
                x, y;

            gfx::Color
                color;
            
            bool
                enabled;

            std::vector<Action *>
                actions;

            core::Config *value;
            Menu         *submenu;
        };
        
        class Container;
        class Container
        {
        public:
            Container
                *parent;

            struct
            {
                float x;
                float y;
            } center;

            float
                left,
                top,
                width,
                height,
                margin,
                padding;

            char
                *caption,
                *value;
            
            bool
                vertical,
                enabled,
                selectable;
            
            unsigned int
                align;

            gfx::Color
                background,
                border;
            
            gfx::Font
                *font;
                
            struct
            {
                Container
                    *up,
                    *down,
                    *left,
                    *right;
            } target;

            std::vector<Container *>
                children;

            std::vector<Option *>
                options;

            Container();
            Container(float size, bool horizontal = false);
            Container(const char *caption, float size = 0.0f, bool horizontal = false);
            ~Container();
            
            Container *
            add(Container *container);
            // Adds container as a child, returns this.
            
            Container *
            set_horizontal(bool horizontal = true) { this->vertical = !horizontal; return this; }

            Container *
            set_vertical(bool vertical = true) { this->vertical = vertical; return this; }

            void
            compose(void);
            // Calculates final child positions
            
            bool
            is_selectable(void) const;
            // Returns true if this can receive focus (others are layout containers)
            
            void
            render(const Container *selection = NULL, float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f) const;
            // render(const Container *selection = NULL, float x = -1.0f, float y = 1.0f, float w = 1.0f, float h = 1.0f) const;
            // render(const Container *selection = NULL, float x = -1.0f, float y = 1.0f, float w = 2.0f, float h = 2.0f) const;
            
        private:
            void
            init(void);
        };
        
        Menu();
        ~Menu();

        bool
        run(Menu *parent = NULL);

        void
        render(void) const;

        bool
        load(const char *filename);
    
        bool
        load(const core::Config *data);
    
        Container *
        add(Container *container) { return this->container->add(container); }

        Container
            *selection,
            *container;
    };
}


#endif
