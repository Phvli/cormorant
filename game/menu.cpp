#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "menu.h"

#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>

#include "../core/engine.h"

#include "../core/engines/input.h"
#include "../core/util/string.h"
#include "../math/vec2.h"
#include "../math/util.h"
#include "../gfx/3d/program.h"
#include "../gfx/font.h"

using namespace game;

static gfx::Program
    *_shader = NULL;

static GLuint
    _vbo;

Menu::Menu()
{
    this->container = new Menu::Container();
    this->selection = this->container;

    this->container->width  = 1.0f;
    this->container->height = 1.0f;
}

Menu::~Menu()
{
    delete this->container;
    // for (std::vector<Menu::Option *>::iterator i = this->options.begin();
    //     i != this->options.end(); ++i)
    // {
    //     delete *i;
    // }
}

bool
Menu::run(Menu *parent)
{
    if (input.keyboard.was_pressed(SDL_SCANCODE_UP)
        && this->selection->target.up != NULL)
    {
        this->selection = this->selection->target.up;
    }

    if (input.keyboard.was_pressed(SDL_SCANCODE_DOWN)
        && this->selection->target.down != NULL)
    {
        this->selection = this->selection->target.down;
    }

    if (input.keyboard.was_pressed(SDL_SCANCODE_LEFT)
        && this->selection->target.left != NULL)
    {
        this->selection = this->selection->target.left;
    }

    if (input.keyboard.was_pressed(SDL_SCANCODE_RIGHT)
        && this->selection->target.right != NULL)
    {
        this->selection = this->selection->target.right;
    }

    // int change = 0;

    // if (input.keyboard.was_pressed(SDL_SCANCODE_UP))
    // {
    //     change--;
    // }
    // if (input.keyboard.was_pressed(SDL_SCANCODE_DOWN))
    // {
    //     change++;
    // }
    // if (change)
    // {
    //     this->selection = (this->selection + change + (int)this->options.size())
    //         % this->options.size();
    // }

    // if (input.keyboard.was_pressed(SDL_SCANCODE_RETURN))
    // {
    //     return false;
    // }
    // if (input.keyboard.was_pressed(SDL_SCANCODE_ESCAPE))
    // {
    //     return false;
    // }

    return true;
}

void
Menu::render(void)
const
{
    _shader = gfx::Program::get(
        "../../fonts/shaders/background_vertex",
        "../../fonts/shaders/background_fragment");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glGenBuffers(1, &_vbo);
    this->container->font->prepare();
    this->container->width  = 1.0f;
    this->container->height = 1.0f;
    this->container->render(this->selection
        , -1.0f, 1.0f, 2.0f, 2.0f
        // , -0.0f, 0.0f, 1.0f, 1.0f
        // , -0.8f, 0.8f, 1.6f, 1.6f
        );
    this->container->font->prepare(false);
    glDeleteBuffers(1, &_vbo);

    // int n = 0;
    // for (std::vector<Menu::Option *>::const_iterator i = this->options.begin();
    //     i != this->options.end(); ++i)
    // {
    //     (*i)->render(this->selection == n++);
    // }

}

bool
Menu::load(const char *filename)
{
    core::Config data;
    data.load(filename);

    return this->load(&data);
}

bool
Menu::load(const core::Config *data)
{
    // this->container->compose();
    
    // (*(core::Config *)data).save("../test.txt");
    // // (*(core::Config *)data)["main_menu"].save("../test.txt");
    // this->options.push_back(new Menu::Option("Herp", 0xfff0e050));
    // this->options.push_back(new Menu::Option("Derp", 0xfff0e050));
    // this->options.push_back(new Menu::Option("kebab", 0xffe06040));
    
    // float y = .5f;
    // for (std::vector<Menu::Option *>::iterator i = this->options.begin();
    //     i != this->options.end(); ++i)
    // {
    //     (*i)->left = -.5f;
    //     (*i)->top  = y -= .15f;
    // }

    return true;
}

Menu::Option::Option()
{
    this->text    = NULL;
    this->color   = 0xffc0c0c0;
    this->x       = 0.0f;
    this->y       = 0.0f;
    this->enabled = true;
    
    this->value   = NULL;
    this->submenu = NULL;
}

Menu::Option::Option(const char *text, gfx::Color color, float x, float y)
{
    this->text    = core::str::dup(text);
    this->color   = color;
    this->x       = x;
    this->y       = y;
    this->enabled = true;
    
    this->value   = NULL;
    this->submenu = NULL;
}

Menu::Option::~Option()
{
    delete[] this->text;

    delete this->value;
    delete this->submenu;

    for (std::vector<Menu::Action *>::iterator i = this->actions.begin();
        i != this->actions.end(); ++i)
    {
        delete *i;
    }
}

void
Menu::Option::render(bool selected)
const
{
    gfx::Font *font = gfx::Font::get()
        ->prepare()
        ->add_effect(gfx::Font::ITALIC)
        ->set_size(32)
        ->at(this->x, this->y);

    if (selected)
    {
        font->set_color(0xffffffff)
            ->add_effect(gfx::Font::OUTLINE);
        
        char buf[2] = "x";
        float x = this->x;
        for (int i = 0; (buf[0] = this->text[i]) != '\0'; ++i)
        {
            float f = sin(core::engine.ticks / 256.0f - (float)i);
            font
                ->set_size(32.0f + f)
                ->set_effect_color(
                gfx::blend::ratio(
                    this->color,
                    0x00000000,
                    sin(core::engine.ticks / 256.0f - (float)i * .3f)
                        * .3f
                        + .3f
                ))
            ->at(x)
            ->write(buf);

            x += font->set_size(32)->get_width(buf);
        }
        
        delete font;
        return;
    }
    else if (this->enabled)
    {
        font->set_color(this->color);
    }
    else
    {
        font->set_color(0xff404040);
    }

    delete font
        ->write(this->text);
}

Menu::Action::Action()
{
    this->command = NULL;
    this->param   = NULL;
}


Menu::Action::~Action()
{
    delete[] this->command;

    for (char **p = this->param; p != NULL; ++p)
    {
        delete[] *p;
    }

    delete[] this->param;
}

Menu::Container::Container()
{
    this->init();
}

Menu::Container::Container(float size, bool horizontal)
{
    this->init();
    
    this->vertical = !horizontal;
    this->width    = size;
    this->height   = size;
}


Menu::Container::Container(const char *caption, float size, bool horizontal)
{
    this->init();
    
    if (caption != NULL)
    {
        core::str::set(this->caption, core::str::dup(caption));
    }
    
    this->vertical = !horizontal;
    this->width    = size;
    this->height   = size;
}

void
Menu::Container::init(void)
{
    this->parent       = NULL;
    this->caption      = NULL;
    this->value        = NULL;
    
    this->width        = 0.0f;
    this->height       = 0.0f;
    this->margin       = 0.0f;
    this->padding      = 0.0f;
    this->background   = 0x80202020;
    this->border       = 0xff10a020;

    this->target.left  = NULL;
    this->target.right = NULL;
    this->target.up    = NULL;
    this->target.down  = NULL;
    this->enabled      = true;
    
    this->center.x     = 666.0f;
    this->center.y     = 666.0f;

    this->vertical     = true;
    this->align        = CENTER;
    this->font         = gfx::Font::get()
    //     ->set_size(32);

    // this->font         = gfx::Font::get("small")
    //     ->add_effect(gfx::Font::MONOSPACE)
        ->set_size(14);

    this->margin       = .01f;
    this->padding      = .00f;
}

Menu::Container::~Container()
{
    delete[] this->caption;
    delete[] this->value;
    delete   this->font;

    for (std::vector<Menu::Container *>::iterator i = this->children.begin();
        i != this->children.end(); ++i)
    {
        delete *i;
    }
}

Menu::Container *
Menu::Container::add(Menu::Container *container)
{
    container->parent = this;
    
    if (this->vertical)
    {
        container->width = 0.0f;
    }
    else
    {
        container->height = 0.0f;
    }
    
    if (std::find(this->children.begin(), this->children.end(), container)
        == this->children.end())
    {
        this->children.push_back(container);
    }
    
    return this;
}

void
Menu::Container::compose(void)
{
    static std::vector<Menu::Container *> global_list;
    
    if (this->parent == NULL)
    {
        global_list.clear();
    }
    
    // If selectable, add as a selection target
    // if (this->children.empty())
    {
        global_list.push_back(this);
    }
    
    for (std::vector<Menu::Container *>::iterator i = this->children.begin();
        i != this->children.end(); ++i)
    {
        (*i)->compose();
    }
    
    this->left          = 0.0f;
    this->top           = 0.0f;

    int unspecified     = 0;
    float largest       = 0.0f;
    float pending_space = 1.0f;
    
    // Count the elements that do not have their dimension specified
    for (std::vector<Menu::Container *>::iterator i = this->children.begin();
        i != this->children.end(); ++i)
    {
        if (this->vertical)
        {
            pending_space -= (*i)->height;
            unspecified   += ((*i)->height == 0.0f);
            largest        = math::max(largest, (*i)->width);
        }
        else
        {
            pending_space -= (*i)->width;
            unspecified   += ((*i)->width == 0.0f);
            largest        = math::max(largest, (*i)->height);
        }
    }
    
    if (largest == 0.0f)
    {
        largest = 1.0f;
    }
    
    if (unspecified && pending_space > 0.0f)
    {
        pending_space /= (float)unspecified;
    }
    else
    {
        pending_space = 0.0f;
    }
    
    
    float offset = 0.0f;

    // Locate child elements and make them fill all empty space
    for (std::vector<Menu::Container *>::iterator i = this->children.begin();
        i != this->children.end(); ++i)
    {
        if (this->vertical)
        {
            (*i)->left    = 0.0f;
            (*i)->top     = offset;
            (*i)->width   = largest;
            (*i)->height += pending_space * ((*i)->height == 0.0f);
            offset       -= (*i)->height;
        }
        else
        {
            (*i)->left    = offset;
            (*i)->top     = 0.0f;
            (*i)->width  += pending_space * ((*i)->width == 0.0f);
            (*i)->height  = largest;
            offset       += (*i)->width;
        }
    }

    if (this->parent == NULL)
    {
        for (std::vector<Menu::Container *>::iterator
            c = global_list.begin(); c != global_list.end(); ++c)
        {
            Menu::Container *p;
            std::vector<Menu::Container *> path;
            for (p = *c; p != NULL; p = p->parent)
            {
                path.push_back(p);
            }

            float x = 0.0f, y = 0.0f, w = 1.0f, h = 1.0f;

            while (!path.empty())
            {
                p = path.back();
                path.pop_back();

                float s = p->margin + p->padding;

                x += w * p->left   + s;
                y += h * p->top    - s;
                w  = w * p->width  - s * 2.0f;
                h  = h * p->height - s * 2.0f;
            }
            (*c)->center.x = x + w / 2.0f;
            (*c)->center.y = y - h / 2.0f;
        }
            
        for (std::vector<Menu::Container *>::iterator
            c = global_list.begin(); c != global_list.end(); ++c)
        {
            struct direction
            {
                Menu::Container **target;
                float dist;
                math::Vec2 vec;
            } dir[4];
            
            dir[0].vec    = math::Vec2(-1.0f,  0.0f);
            dir[0].target = &(*c)->target.left;
            
            dir[1].vec    = math::Vec2( 1.0f,  0.0f);
            dir[1].target = &(*c)->target.right;
            
            dir[2].vec    = math::Vec2( 0.0f,  1.0f);
            dir[2].target = &(*c)->target.up;

            dir[3].vec    = math::Vec2( 0.0f, -1.0f);
            dir[3].target = &(*c)->target.down;
            
            for (int d = 0; d < 4; ++d)
            {
                dir[d].dist = 99999.9f;
            }
            
            for (std::vector<Menu::Container *>::iterator
                i = global_list.begin(); i != global_list.end(); ++i)
            {
                if (*i == *c || !(*i)->is_selectable())
                {
                    continue;
                }
                
                math::Vec2 vec = math::Vec2(
                    (*i)->center.x - (*c)->center.x,
                    (*i)->center.y - (*c)->center.y
                );
                
                float
                    dist = vec.length_sq();

                for (int d = 0; d < 4; ++d)
                {
                    vec.normalize();
                    float a = (acos(vec.dot(dir[d].vec)));
                    dist *= 1.0f + a * 2.0f;

                    if (a < 1.0f && dist < dir[d].dist)
                    {
                        dir[d].dist    = dist;
                        *dir[d].target = *i;
                    }
                }
            }
        }
    }
}

bool
Menu::Container::is_selectable(void)
const
{
    return this->children.empty();
}

void
Menu::Container::render(const Container *selection, float x, float y, float w, float h)
const
{
    x += w * this->left   + this->margin;
    y += h * this->top    - this->margin;
    w  = w * this->width  - this->margin * 2.0f;
    h  = h * this->height - this->margin * 2.0f;
    
    GLfloat rect[8] = {
        x,     y,
        x + w, y,
        x + w, y - h,
        x,     y - h
    };
    
    glUseProgram(_shader->id);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glEnableVertexAttribArray(_shader->attr.t);
    glVertexAttribPointer(_shader->attr.t, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &rect, GL_STATIC_DRAW);

    // Render background
    if ((this->background >> 24) & 0xff)
    {
        gfx::Color color = this->background;
        if (selection == this)
        {
            color = (color & 0xff000000)
                | (this->font->color.stroke & 0x00ffffff);
        }
        
        glUniform4f(_shader->unif.diffuse_color,
            (float)((color >> 16) & 0xff) / 256.0f,
            (float)((color >>  8) & 0xff) / 256.0f,
            (float)((color      ) & 0xff) / 256.0f,
            (float)((color >> 24) & 0xff) / 256.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // Render border
    if ((this->border >> 24) & 0xff)
    {
        gfx::Color color = this->border;
        
        if (selection == this)
        {
            color = gfx::blend::ratio(
                color,
                0xffffffff,
                sin(core::engine.ticks / 64.0f) * .25f + .75f);
        }
        
        glUniform4f(_shader->unif.diffuse_color,
            (float)((color >> 16) & 0xff) / 256.0f,
            (float)((color >>  8) & 0xff) / 256.0f,
            (float)((color      ) & 0xff) / 256.0f,
            (float)((color >> 24) & 0xff) / 256.0f);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }

    glDisableVertexAttribArray(_shader->attr.t);
    
    x += this->padding;
    y -= this->padding;
    w -= this->padding * 2.0f;
    h -= this->padding * 2.0f;
    
    // Render caption
    if (this->caption != NULL)
    {
        gfx::Color color = this->font->color.stroke;
        
        if (this == selection)
        {
            this->font->set_color(this->background);
        }
        
        if (this->align & LEFT)
        {
            this->font->x = x;
        }
        else if (this->align & RIGHT)
        {
            this->font->x = x + w - this->font->get_width(this->caption);
        }
        else if (this->align & CENTER_X)
        {
            this->font->x = x + (w - this->font->get_width(this->caption))
                / 2.0f;
        }

        if (this->align & TOP)
        {
            this->font->y = y;
        }
        else if (this->align & BOTTOM)
        {
            this->font->y = y - h + this->font->get_height(this->caption);
        }
        else if (this->align & CENTER_Y)
        {
            this->font->y = y - (h - this->font->get_height(this->caption))
                / 2.0f;
        }
        
        this->font
            ->write(this->caption)
            ->set_color(color);
    }

    // Render children
    for (std::vector<Menu::Container *>::const_iterator
        i = this->children.begin();
        i != this->children.end(); ++i)
    {
        (*i)->render(selection, x, y, w, h);
    }
}
