#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#define CUTSCENE_DIRECTORY "../data/game/cutscenes/"

#include "cutscene.h"
#include <cstddef>

#include "../gfx/font.h"
#include "../core/util/config.h"
#include "../core/util/file.h"
#include "../core/util/string.h"
#include "../core/engine.h"


// shader and vertex buffer object for text rendering
gfx::Program *_img_shader = NULL;
gfx::Font    *_img_font   = NULL;
GLuint _img_texture;

static GLuint
_img_vbo;

using namespace game;

static void
_setup_image(gfx::Sprite *sprite);

Cutscene::Cutscene()
{
    this->name  = NULL;
    this->music = NULL;
}

Cutscene::Cutscene(const char *name)
{
    this->name  = NULL;
    this->music = NULL;
    this->load(name);
}

Cutscene::~Cutscene()
{
    delete[] this->name;
    delete[] this->music;

    for (std::vector<Cutscene::Page *>::iterator page = this->pages.begin();
        page != this->pages.end(); ++page)
    {
        delete *page;
    }
}

Cutscene::Page::Page()
{
    this->image = NULL;
    this->width = 0.0f;
}

Cutscene::Page::~Page()
{
    delete this->image;
    
    for (std::vector<char *>::iterator line = this->lines.begin();
        line != this->lines.end(); ++line)
    {
        delete *line;
    }
}

void
Cutscene::Page::render(int reveal_threshold)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float
        x = -this->width / 2.0f,
        y = this->lines.size() * this->line_spacing / 2.0f;
    
    if (this->image)
    {
        float h =
            (float)(screen.w / screen.h)
            / (float)(this->image->w / this->image->h);
        
        y -= h / 2.0f;
        
        GLfloat img[16] = {
            -.5f, y + h,    0.0f, 0.0f,
             .5f, y + h,    1.0f, 0.0f,
            -.5f, y,        0.0f, 1.0f,
             .5f, y,        1.0f, 1.0f
        };

        y -= this->line_spacing * 2.0f;

        glUseProgram(_img_shader->id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _img_texture);
        glUniform1i(_img_shader->unif.color_map, 0);

        glBindBuffer(GL_ARRAY_BUFFER, _img_vbo);
        glEnableVertexAttribArray(_img_shader->attr.t);

        glVertexAttribPointer(_img_shader->attr.t, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(GLfloat), img, GL_STATIC_DRAW);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableVertexAttribArray(_img_shader->attr.t);

        _img_font->prepare();
    }

    for (std::vector<char *>::iterator i = this->lines.begin();
        i != this->lines.end(); ++i)
    {
        char *s = core::str::dupn(*i, reveal_threshold);
        reveal_threshold -= core::str::len(*i);

        _img_font
            ->at(x, y)
            ->write(s);
        
        y -= this->line_spacing;
    }

    SDL_GL_SwapWindow(screen.window);
}

#include <cstdio>
bool
Cutscene::load(const char *name)
{
    char *path = core::str::format(CUTSCENE_DIRECTORY "%s.cfg", name);
    core::str::set(path, core::str::normalize_path(path));

    core::engine.log("Loading %s", path);
    core::str::set(this->name, core::str::dup(name));

    core::Config file;
    file.load(path);
    delete path;
    
    this->text_speed      = file["text_speed"].real(.4f);
    this->text_delay      = file["text_delay"].real(25.0f);
    this->page_turn_delay = file["page_turn_delay"].integer(3500);
    core::str::set(this->music, core::str::dup(file["music"].string("")));
    this->pages.clear();
    
    core::engine.log("speed %f, delay %f, turnd %i", this->text_speed, this->text_delay, this->page_turn_delay);
    core::engine.log("music %s", this->music);

    for (core::Config::Value::iterator i = file["pages"].begin();
        i != file["pages"].end(); ++i)
    {
        Cutscene::Page *page = new Cutscene::Page();
        this->pages.push_back(page);
        
        core::Config &lines = *i->second;

        page->image        = new gfx::Sprite();
        page->line_spacing = file["row_spacing"].real(.06f);

        path = core::str::cat(CUTSCENE_DIRECTORY, lines["img"].string(""));
        core::str::set(path, core::str::normalize_path(path));
        if (!page->image->load(path))
        {
            delete page->image;
            page->image = NULL;
        }
        delete path;
        delete &lines["img"];

        for (core::Config::Value::iterator line = lines.begin();
            line != lines.end(); ++line)
        {
            page->lines.push_back(core::str::dup(line->second->string("")));
        }
    }
    
    if (this->pages.size())
    {
        core::engine.log("Cutscene %s loaded", this->name);
    }
    else
    {
        core::engine.log("(!) Failed to load cutscene data");
    }
    
    return false;
}

void
Cutscene::run(void)
{
    core::engine.log_header(this->name);
    core::engine.deactivate_module("game");
    core::engine.log("Playing %i-page cutscene", this->pages.size());

    if (core::str::len(this->music))
    {
        core::engine.play_music(this->music);
    }

    _img_font = gfx::Font::get();
    _img_font->prepare()
        ->set_size(22)
        ->set_color(0xd0d0d0)
        ->add_effect(gfx::Font::OUTLINE, 0x000000);

    for (std::vector<Cutscene::Page *>::iterator page = this->pages.begin();
        page != this->pages.end(); ++page)
    {
        if ((*page)->image != NULL)
        {
            _setup_image((*page)->image);
        }

        int letter_count = 0;
        for (std::vector<char *>::iterator i = (*page)->lines.begin();
            i != (*page)->lines.end(); ++i)
        {
            (*page)->width = std::max((*page)->width, _img_font->get_width(*i));
            letter_count += core::str::len(*i);
        }
        
        unsigned long int page_skip = core::engine.ticks + 1;
        for (
            float reveal_threshold = -this->text_delay * this->text_speed;
            core::engine.ticks < page_skip
                && core::engine.run()
                && !input[core::Input::QUIT];
            reveal_threshold += this->text_speed)
        {
            (*page)->render(reveal_threshold);

            if (reveal_threshold <= letter_count)
            {
                page_skip = core::engine.ticks + this->page_turn_delay;
            }
            if (input.keyboard.presses.size())
            {
                if (reveal_threshold > letter_count)
                {
                    break;
                }
                reveal_threshold = letter_count;
            }
        }
        
        if (input.keyboard[SDL_SCANCODE_ESCAPE] || input[core::Input::QUIT])
        {
            break;
        }
    }
    
    delete _img_font;
    _img_font = NULL;

    core::engine.log_header("Cutscene finished");
    core::engine.activate_module("game");
}

static void
_setup_image(gfx::Sprite *sprite)
{
    if (_img_shader == NULL)
    {
        glGenBuffers(1, &_img_vbo);
        _img_shader = gfx::Program::get(
            "../../fonts/shaders/vertex",
            "textured");
    }
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &_img_texture);
    glBindTexture(GL_TEXTURE_2D, _img_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    unsigned char *data = new unsigned char[sprite->w * sprite->h * 4];
    const gfx::Component *src = (gfx::Component *)sprite->data;
    for (int i = 0; i < sprite->w * sprite->h * 4; src += 4)
    {
        data[i++] = src[2];
        data[i++] = src[1];
        data[i++] = src[0];
        data[i++] = src[3];
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite->w, sprite->h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);

    delete[] data;

    glBindTexture(GL_TEXTURE_2D, 0);
}
