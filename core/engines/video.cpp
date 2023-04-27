#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "video.h"

#include "input.h"
#include "../engine.h"
#include "../../version.h"
#include "../../gfx/3d/program.h"
#include "../../gfx/3d/material.h"
#include "../../gfx/3d/model.h"
#include "../../gfx/3d/mesh.h"
#include "../../math/util.h"

core::Graphics screen;

using namespace core;

void
VideoEngine::update_tick(void)
{
}

void
VideoEngine::update_frame(void)
{
    if (input[Input::FULLSCREEN_TOGGLE])
    {
        screen.set_fullscreen(!screen.fullscreen);
    }
    
    screen.show();
}

void
VideoEngine::initialize_module()
{
   if (!SDL_WasInit(SDL_INIT_VIDEO))
    {
        SDL_Init(SDL_INIT_VIDEO);
    }

    screen.resize(
        this->config["video"]["resolution"]["x"].integer(1280),
        this->config["video"]["resolution"]["y"].integer(720),
        this->config["video"]["fullscreen"].boolean(true)
    );
}

VideoEngine::VideoEngine()
{
    this->register_module(this);
}

VideoEngine::~VideoEngine()
{
    this->log(this->get_module_name());

#ifdef USE_OPENGL
    glUseProgram(0);
    
    this->log("Closing scene");
    delete screen.scene;
    screen.scene = NULL;

    this->log("Unloading textures");
    gfx::Texture::flush_cache();
    
    this->log("Unloading materials");
    gfx::Material::flush_cache();
    
    this->log("Unloading models");
    gfx::Model::flush_cache();
    
    this->log("Unloading shaders");
    gfx::Shader::flush_cache();
    
    this->log("Unloading programs");
    gfx::Program::flush_cache();

    this->log("Video resources unloaded");

    if (screen.gl_context != NULL)
    {
        SDL_GL_DeleteContext(screen.gl_context);
        screen.gl_context = NULL;
    }
#else
    if (screen.texture != NULL)
    {
        SDL_DestroyTexture(screen.texture);
        screen.texture = NULL;
    }
    
    if (screen.renderer != NULL)
    {
        SDL_DestroyRenderer(screen.renderer);
        screen.renderer = NULL;
    }
    
    if (screen.window != NULL)
    {
        SDL_DestroyWindow(screen.window);
        screen.window = NULL;
    }
#endif
}

Graphics::Graphics()
#ifdef USE_OPENGL
:
    x(mutable_x),
    y(mutable_y)
#endif
{
    this->fullscreen   = false;
    this->detail_level = 0;
    
    this->window       = NULL;

#ifdef USE_OPENGL
    this->mutable_x    = 0;
    this->mutable_y    = 0;
    this->gl_context   = NULL;
#else
    this->renderer     = NULL;
    this->texture      = NULL;
#endif

    this->scene        = NULL;
}

Graphics::~Graphics()
{
    delete this->scene;
}

void
Graphics::show()
{
#ifdef USE_OPENGL
    glViewport(this->x, this->y, this->w, this->h);
    SDL_GL_SwapWindow(this->window);
#else
    SDL_UpdateTexture(this->texture, NULL, this->data, this->w * Sprite::BYTES_PER_PIXEL);
    SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);
    SDL_RenderPresent(this->renderer);
#endif
}

void
Graphics::resize(int w, int h, bool fullscreen)
{
    SDL_Window    *win;

#ifdef USE_OPENGL
    Uint32 window_flags = SDL_WINDOW_OPENGL;
    SDL_GLContext con   = NULL;
#else
    Uint32 window_flags = 0;
    SDL_Renderer *ren   = NULL;
    SDL_Texture  *tex   = NULL;
#endif

    if (w <= 0 || h <= 0)
    {
        engine.log("(!) Failed to set video mode: invalid parameters or graphics module uninitialized");
        return;
    }
    
    engine.log("Video mode: %i x %i (%i bpp)", w, h, Sprite::BYTES_PER_PIXEL);
    
    try
    {
#ifdef USE_OPENGL
        
        // Set double buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // Set FSAA
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,
            engine.config["video"]["fsaa"].integer(4));

        // Create OpenGL window
        win = SDL_CreateWindow(APP_TITLE,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            w, h,
            window_flags | (SDL_WINDOW_FULLSCREEN * fullscreen));

#else
        // Create normal window
        win = SDL_CreateWindow(APP_TITLE,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            w * !fullscreen, h * !fullscreen,
            window_flags | (SDL_WINDOW_FULLSCREEN_DESKTOP * fullscreen));
#endif
        if (win == NULL)
        {
            engine.log("(!) Window creation failed: %s", SDL_GetError());
            throw 666;
        }
    
#ifdef USE_OPENGL

        // Create OpenGL context
        con = SDL_GL_CreateContext(win);
        if (con == NULL)
        {
            engine.log("(!) OpenGL context creation failed: %s", SDL_GetError());
            throw 666;
        }

        // OpenGL 3.2
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        // SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 0);

        int major, minor;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
        engine.log("OpenGL %i.%i", major, minor);

        if (major > 3 || (major == 3 && minor >= 2))
        {
            engine.log("<supported");
        }
        else
        {
            engine.log("<not supported");
            throw 666;
        }

#   ifndef __APPLE__
        // Init GLEW
        glewExperimental = GL_TRUE;
        GLenum glew_init = glewInit();
        if (glew_init != GLEW_OK)
        {
            engine.log("(!) GLEW initialization failed: %s", glewGetErrorString(glew_init));
            throw 666;
        }
#   endif

        // Set v-sync
        SDL_GL_SetSwapInterval(0);
        if (engine.config["video"]["vsync"].boolean(true))
        {
            SDL_GL_SetSwapInterval(1);
        }

        // Get translation coordinates for fullscreen mode
        SDL_GL_GetDrawableSize(win, &this->mutable_x, &this->mutable_y);
        this->mutable_x = (this->x - w) / 2;
        this->mutable_y = (this->y - h) / 2;

        delete this->scene;
        this->gl_context = con;
        
        this->scene = new gfx::Scene();
        this->scene->resize(w, h);

        glCullFace(GL_BACK);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#else // initialize non-opengl version:

        ren = SDL_CreateRenderer(win, -1, 0);
        if (ren == NULL)
        {
            throw 666;
        }
        
        if (this->detail_level)
        {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        }
        else
        {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
        }
        
        tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, w, h);
        if (tex == NULL)
        {
            throw 666;
        }
        
        SDL_DestroyTexture(this->texture);
        SDL_DestroyRenderer(this->renderer);
        
        SDL_RenderSetLogicalSize(ren, w, h);

        delete this->scene;

        this->renderer = ren;
        this->texture  = tex;

#endif // no opengl

        SDL_DestroyWindow(this->window);
        this->window = win;

        Sprite::resize(w, h);

        this->fullscreen = (SDL_GetWindowFlags(this->window)
            & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP))
            ? true : false;

        this->clear();
        this->show();

        core::engine.log("Video mode set");
    }
    catch (int)
    {
        engine.log("(!) Failed to set video mode: %s", SDL_GetError());

#ifdef USE_OPENGL
        SDL_GL_DeleteContext(con);
#else
        SDL_DestroyTexture(tex);
        SDL_DestroyRenderer(ren);
#endif

        SDL_DestroyWindow(win);
    }
}

void
Graphics::set_fullscreen(bool fullscreen)
{
    if (fullscreen != this->fullscreen)
    {
        engine.log((fullscreen) ? "Windowed mode" : "Fullscreen mode");
        if (SDL_SetWindowFullscreen(this->window,
            SDL_WINDOW_FULLSCREEN_DESKTOP * fullscreen) == 0)
        {
            this->fullscreen = fullscreen;
            engine.log("<(OK)");
        }
    }
}
