#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#define MIN_TEXTURE_SIZE 32
#define MAX_TEXTURE_SIZE 2048

#include "texture.h"

#include <cmath>     // pow, log
#include <algorithm> // max

#include "../../core/util/string.h"
#include "../../core/util/cache.h"
#include "../../core/util/file.h"
#include "../../core/engine.h"
#include "../../math/util.h"

using namespace gfx;

static core::Cache<Texture> global_cache;

Texture::Texture() :
    w(w_mutable),
    h(h_mutable),
    ready(ready_mutable),
    translucent(translucent_mutable)
{
    this->id                  = 0;
    this->flags               = 0;
    this->framebuffer_id      = 0;
    this->depthbuffer_id      = 0;
    this->w_mutable           = 0;
    this->h_mutable           = 0;
    this->translucent_mutable = false;
}

Texture::~Texture()
{
    glDeleteTextures(1, &this->id);
    glDeleteFramebuffers(1, &this->framebuffer_id);
    glDeleteRenderbuffers(1, &this->depthbuffer_id);
    
    global_cache.drop(this, false);
}

void
Texture::bind(void)
const
{
    glBindTexture(this->type, this->id);
}

void
Texture::unbind(void)
const
{
    glBindTexture(this->type, 0);
}

void
Texture::target(bool render_to_texture)
{
    if (render_to_texture)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_id);
        glViewport(0, 0, this->w, this->h);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, this->id);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(screen.x, screen.y, screen.w, screen.h);
    }
}

Texture *
Texture::get(const char *filename, Texture::Flags flags, int index)
{
    Texture *result;

    char *normalized = core::str::cat(DATA_DIRECTORY, filename);
    core::str::set(normalized, core::str::normalize_path(normalized));
    
    char *key  = core::str::format("%s::%x", normalized, flags);
    
    if (global_cache.contains(key))
    {
        result = global_cache[key];
    }
    else
    {
        GLenum type = (flags & Texture::CUBEMAP)
            ? GL_TEXTURE_CUBE_MAP
            : GL_TEXTURE_2D;
        
        int   files;
        bool  success;
        char *path = NULL, *dir, *ext;
        gfx::Sprite *sprite[6];
        switch (type)
        {
            case GL_TEXTURE_2D:
                core::engine.log("Loading %s", normalized);
                core::str::set(path, core::str::dup(normalized));
                sprite[0] = new gfx::Sprite();
                success   = sprite[0]->load(path);
                files     = 1;

                break;

            
            case GL_TEXTURE_CUBE_MAP:
                const char *cubemap_sides[] = {
                    "right",  // GL_TEXTURE_CUBE_MAP_POSITIVE_X
                    "left",   // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
                    "bottom", // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
                    "top",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
                    "back",   // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
                    "front"   // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
                };

                core::str::get_path_components(normalized,
                    &path, &dir, &ext);
                
                core::str::set(dir, core::str::cat(path, dir));

                core::engine.log("Loading %s/*.%s", dir, ext);
                for (files = 0, success = true; success && files < 6; ++files)
                {
                    core::str::set(path, core::str::format("%s/%s.%s",
                        dir, cubemap_sides[files], ext));
                    
                    sprite[files] = new gfx::Sprite();
                    success &= sprite[files]->load(path);
                }
                delete[] dir;
                delete[] ext;

                break;
        }
        
        if (success)
        {
            result = global_cache.store(key, Texture::load(
                (const gfx::Sprite **)sprite, flags, index));
            core::engine.log("<(OK)");
        }
        else
        {
            core::engine.log("(!) Failed to load %s", path);
            result = global_cache.store(key, NULL);
        }
        
        delete[] path;
        for (int i = 0; i < files; ++i)
        {
            delete sprite[i];
        }
    }

    delete[] normalized;
    delete[] key;
    return result;
}

Texture *
Texture::framebuffer(const char *name, int w, int h, bool alpha, unsigned int depth_bits)
{
    if (global_cache.contains(name))
    {
        return global_cache[name];
    }

    core::engine.log("New render buffer \"%s\" (%i x %i)", name, w, h);

    Texture *texture   = new Texture();
    texture->type      = GL_TEXTURE_2D;
    texture->w_mutable = w;
    texture->h_mutable = h;
    texture->flags     = Texture::FRAMEBUFFER
        | (Texture::DEPTHBUFFER * (depth_bits > 0));

    glGenFramebuffers(1, &texture->framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, texture->framebuffer_id);
    
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    GLint format = (alpha)
        ? GL_RGBA
        : GL_RGB;

    texture->translucent_mutable = alpha;
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
        format, GL_UNSIGNED_BYTE, 0);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, texture->id, 0);

    if (texture->flags & Texture::DEPTHBUFFER)
    {
        glGenRenderbuffers(1, &texture->depthbuffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, texture->depthbuffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, texture->depthbuffer_id);
        // glGenTextures(1, &texture->depthbuffer_id);
        // glBindTexture(GL_TEXTURE_2D, texture->depthbuffer_id);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // switch (depth_bits)
        // {
        //     case 16: format = GL_DEPTH_COMPONENT16; break;
        //     case 32: format = GL_DEPTH_COMPONENT32; break;
        //     default: format = GL_DEPTH_COMPONENT24;
        // }
        
        // texture->translucent_mutable = false;
        // glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
        //         GL_DEPTH_COMPONENT, GL_FLOAT, 0);

        // glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        //     GL_TEXTURE_2D, texture->depthbuffer_id, 0);

        // glDrawBuffers(1, &texture->depthbuffer_id);
    }
    
    texture = global_cache.store(name, texture);
    
    core::engine.log("<(OK)");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

   return texture;
}

Texture *
Texture::load(const Sprite **sprite, Texture::Flags flags, int index)
{
    glActiveTexture(GL_TEXTURE0 + index);

    Texture *texture = new Texture();
    texture->flags   = flags;
    texture->type    = (flags & Texture::CUBEMAP)
        ? GL_TEXTURE_CUBE_MAP
        : GL_TEXTURE_2D;
    
    glGenTextures(1, &texture->id);
    glBindTexture(texture->type, texture->id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Minification filter
    if (texture->flags & Texture::NO_MIPMAP)
    {
        glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER,
            (texture->flags & Texture::NEAREST)
                ? GL_NEAREST
                : GL_LINEAR);
    }
    else
    {
        glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER,
            (texture->flags & Texture::NEAREST)
                ? GL_NEAREST_MIPMAP_NEAREST
                : GL_LINEAR_MIPMAP_LINEAR);
    }
    
    // Magnification filter
    glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER,
        (texture->flags & Texture::NEAREST)
            ? GL_NEAREST
            : GL_LINEAR);

    // Wrapping
    glTexParameteri(texture->type, GL_TEXTURE_WRAP_S,
        (texture->flags & (Texture::CLAMP_X | Texture::CUBEMAP))
            ? GL_CLAMP_TO_EDGE
            : GL_REPEAT);

    glTexParameteri(texture->type, GL_TEXTURE_WRAP_T,
        (texture->flags & (Texture::CLAMP_Y | Texture::CUBEMAP))
            ? GL_CLAMP_TO_EDGE
            : GL_REPEAT);

    if (texture->flags & Texture::CUBEMAP)
    {
        for (int i = 0; i < 6; ++i)
        {
            texture->attach(sprite[i],
                texture->flags, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        }
    }
    else
    {
        texture->attach(sprite[0], texture->flags, texture->type);
    }

    if (!(texture->flags & Texture::NO_MIPMAP))
    {
        glGenerateMipmap(texture->type);
    }

    glBindTexture(texture->type, 0);


    return texture;
}

bool
Texture::attach(const Sprite *sprite, Texture::Flags flags, GLenum target)
{
    int
        w = std::max(MIN_TEXTURE_SIZE,
            std::min(MAX_TEXTURE_SIZE,
            math::nearest_pow2(sprite->w))),
    
        h = std::max(MIN_TEXTURE_SIZE,
            std::min(MAX_TEXTURE_SIZE,
            math::nearest_pow2(sprite->h)));
    
    Sprite *scaled = NULL;
    // stretch to nearest power of two if needed
    if (sprite->w != w || sprite->h != h)
    {
        core::engine.log("<(%i x %i", sprite->w, sprite->h);
    // (!) FIXME: TODO: fix bilinear and use sprite->scale() instead
        scaled = sprite->scale_nearest(w, h);
        core::engine.log("<resized to %i x %i)", w, h);
        sprite = scaled;
    }

    // Rebuild bitmap upside down for OpenGL
    unsigned char *data = new unsigned char[w * h * 4];
    int i = 0;
    for (int y = h - 1; y >= 0; --y)
    {
        const gfx::Component *src = (flags & Texture::FLIP_Y)
            ? (gfx::Component *)sprite->data + (h - y - 1) * w * 4
            : (gfx::Component *)sprite->data + y * w * 4;

        int src_step;
        if ((flags & Texture::FLIP_X))
        {
            src_step = -4;
            src += (w - 1) * 4;
        }
        else
        {
            src_step = 4;
        }

        for (int x = 0; x < w; ++x)
        {
            data[i++] = src[2];
            data[i++] = src[1];
            data[i++] = src[0];
            data[i++] = src[3];
            
            this->translucent_mutable |= src[3] != 0xff;
            src += src_step;
        }
    }
    delete scaled;

    this->w_mutable = w;
    this->h_mutable = h;

    glTexImage2D(target, 0, GL_RGBA, sprite->w, sprite->h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);

    delete[] data;

    return true;
}

void
Texture::flush_cache(void)
{
    global_cache.flush();
}

void
Texture::blit(void)
const
{
    this->blit(-1.0f, -1.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f);
}

void
Texture::blit(float x, float y, float opacity, float rotation)
const
{
    this->blit(x, y, this->w / (float)screen.w, this->h / (float)screen.h, 0.0f, 0.0f, 1.0f, 1.0f, opacity, rotation);
}

void

Texture::blit(float x, float y, float w, float h, float x_src, float y_src, float w_src, float h_src, float opacity, float rotation)
const
{
    static Program *shader = NULL;
    static GLuint   vbo    = 0;

    if (!vbo)
    {
        core::engine.log("first blit (%f, %f) - (%f, %f)", x, y, x + w, y + h);
        shader = Program::get(
            "../../fonts/shaders/vertex",
            "textured");

        glGenBuffers(1, &vbo);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    
    GLfloat img[16] = {
        x,     y + h,    x_src,         y_src,
        x + w, y + h,    x_src + w_src, y_src,
        x,     y,        x_src,         y_src + h_src,
        x + w, y,        x_src + w_src, y_src + h_src
    };

    glUseProgram(shader->id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->id);
    glUniform1i(shader->unif.color_map, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(shader->attr.t);

    glVertexAttribPointer(shader->attr.t, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(GLfloat), img, GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(shader->attr.t);
}
