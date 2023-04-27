#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#define FONT_DIRECTORY "../data/video/fonts/"

#define LETTER_SPACING  0.0002f
#define SPACE_WIDTH     0.0007f
#define MONOSPACE_WIDTH 0.0018f
#define ITALIC_OFFSET   0.0005f
#define BOLD_STRENGTH   0.2f

#include "font.h"

#include <cstdio>
#include <cstdarg>


#include "../core/engine.h"
#include "../core/util/string.h"
#include "../core/util/file.h"
#include "../core/util/cache.h"
#include "../math/util.h"
#include "../math/vec4.h"
#include "3d/program.h"

#include <map>

using namespace gfx;

// shader and vertex buffer objects for text rendering
static bool _loaded = false;
static Program
    *_normal_shader     = NULL,
    *_effect_shader     = NULL,
    *_background_shader = NULL;

static GLuint
_font_vbo;

// global cache for loaded font templates
static core::Cache<Font> global_cache;

// global character offset buffer
static Font::Character
    _char_buffer[1024];

static int
    _char_count = 0;
    
// global mapping table for ASCII code to character index
static unsigned char _char_map[0xff + 1] = {
    // (!) TODO: FIXME: find ascii values for these:
    // '€' = 95;
    // 'Å' = 96; 'å' = 99;
    // 'Ä' = 97; 'ä' = 100;
    // 'Ö' = 98; 'ö' = 101;
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
    32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
    48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// stash for OpenGL attributes for pushing and popping state on Font::prepare()
class GLCapStash
{
public:
    typedef
        struct
        {
            bool was_enabled;
            int  stack_depth;
        }
        GLCap;
    
    typedef
        std::map<GLenum, GLCap>
        GLCapList;
    
    void
    set(GLenum cap, bool new_state, bool push_state)
    {
        if (this->stash.find(cap) == this->stash.end())
        {
            this->stash[cap].was_enabled = (glIsEnabled(cap) == GL_TRUE);
            this->stash[cap].stack_depth = 0;
        }
        
        GLCap *c = &this->stash[cap];
        
        if (push_state)
        {
            // set OpenGL state
            if (c->stack_depth++ == 0)
            {
                c->was_enabled = (glIsEnabled(cap) == GL_TRUE);
            }

            if (new_state)
            {
                glEnable(cap);
            }
            else
            {
                glDisable(cap);
            }
        }
        else
        {
            // resume old state
            if (--c->stack_depth <= 0)
            {
                c->stack_depth = 0;

                if (c->was_enabled)
                {
                    glEnable(cap);
                }
                else
                {
                    glDisable(cap);
                }
            }
        }
    }

protected:
    GLCapList
        stash;
};

static GLCapStash _gl_cap;

Font::Font()
{
    this->character = NULL;
    this->size      = 8.0f;
    this->effects   = Font::NONE;
    this->at(0.0f, 0.0f, 0xffffffff, 0x00000000);
}

Font::Font(const char *name, float size, Color stroke, Color background, Color effect)
{
    Font *font_template = Font::get(name, size, stroke, background, effect);
    if (font_template)
    {
        *this = *font_template;
        delete font_template;
    }
    else
    {
        this->character = NULL;
        this->size      = size;
        this->effects   = Font::NONE;
        this->at(0.0f, 0.0f, stroke, background, effect);
    }
}

Font *
Font::prepare(bool setup_font_rendering)
{
    _gl_cap.set(GL_DEPTH_TEST, false, setup_font_rendering);
    _gl_cap.set(GL_BLEND,      true, setup_font_rendering);
    _gl_cap.set(GL_CULL_FACE,  false, setup_font_rendering);

    if (setup_font_rendering)
    {
        glGenBuffers(1, &_font_vbo);
    }
    else
    {
        glDeleteBuffers(1, &_font_vbo);
    }
    
    return this;
}


const Font::Character &
Font::operator[](char c)
const
{
    return this->character[(int)_char_map[(int)c]];
}

Font *
Font::write(const char *format, ...)
{
    static char
        buffer[2048];

    static GLfloat
        vertex_data[12000];

    char *s = buffer, ascii;
    float
        x = this->x,
        y = this->y;

    float w = this->size * 2.0f / screen.scene->w;
    float h = this->size * 2.0f / screen.scene->h;
    float max_x = x;
    
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    // cut overly long texts
    buffer[300] = '\0';

    int data_index = 0;
    while ((ascii = *(s++)))
    {
        if (ascii == '\n')
        {
            max_x = math::max(max_x, x);

            y -= h;
            x  = this->x;
        }
        else if (ascii == ' ')
        {
            x += this->size * ((this->effects & Font::MONOSPACE)
                ? MONOSPACE_WIDTH
                : SPACE_WIDTH);
        }
        else
        {
            Character c = this->character[(int)_char_map[(int)ascii]];
            
            GLfloat *data = vertex_data + data_index;
            
            float orig_x = x;
            if (this->effects & Font::MONOSPACE)
            {
                x += (this->size * MONOSPACE_WIDTH - w * c.width);
            }
            
            // Bottom
            data[ 9] = data[13] = data[21] = y - h;
            data[11] = data[15] = data[23] = 0.0f;
            
            // Left
            data[ 0] = data[ 8] = data[12] = x - .003f;
            data[ 2] = data[10] = data[14] = c.start - .0008f;
            
            // Top
            data[ 1] = data[ 5] = data[17] = y;
            data[ 3] = data[ 7] = data[19] = 1.0f;
            
            // Right
            x += w * c.width;
            data[ 4] = data[16] = data[20] = x + .003f;
            data[ 6] = data[18] = data[22] = c.end + .0008f;

            x += this->size * LETTER_SPACING;

            if (this->effects & Font::BOLD)
            {
                float widening = (c.end - c.start) * this->size * BOLD_STRENGTH;
                x        += widening;
                data[4]  += widening;
                data[16] += widening;
                data[20] += widening;
            }

            if (this->effects & Font::ITALIC)
            {
                float offset = this->size * ITALIC_OFFSET;
                data[0]  += offset;
                data[4]  += offset;
                data[16] += offset;
            }

            if (this->effects & Font::MONOSPACE)
            {
                x = orig_x + this->size * MONOSPACE_WIDTH;
            }

            data_index += 24;
        }
    }
    max_x = math::max(max_x, x);
    
    // Render VBO if one was built
    if (data_index)
    {
        Program *shader;
        
        // Render background first
        if (this->effects & Font::BACKGROUND)
        {
            shader = _background_shader;
            glUseProgram(shader->id);
            
            GLfloat rect[8] = {
                this->x, this->y,
                max_x,   this->y,
                this->x, y - h,
                max_x,   y - h
            };
            
            math::Vec4(this->color.background).to(shader->unif.diffuse_color);

            glBindBuffer(GL_ARRAY_BUFFER, _font_vbo);
            glEnableVertexAttribArray(shader->attr.t);

            glVertexAttribPointer(shader->attr.t, 2, GL_FLOAT, GL_FALSE, 0, NULL);
            glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &rect, GL_STATIC_DRAW);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glDisableVertexAttribArray(shader->attr.t);
        }
        
        // Draw letters
        shader = (this->effects & ~(Font::BACKGROUND | Font::MONOSPACE))
            ? _effect_shader
            : _normal_shader;

        glUseProgram(shader->id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->texture);
        glUniform1i(shader->unif.color_map, 0);

        glUniform4i(shader->unif.decal_map,
            (this->effects & Font::BOLD),
            (this->effects & Font::UNDERLINE),
            (this->effects & Font::OUTLINE),
            (this->effects & Font::SHADOW)
        );

        math::Vec4(this->color.stroke).to(shader->unif.diffuse_color);
        math::Vec4(this->color.effect).to(shader->unif.specular_color);

        glEnableVertexAttribArray(shader->attr.t);
        glBindBuffer(GL_ARRAY_BUFFER, _font_vbo);

        glVertexAttribPointer(shader->attr.t, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        glBufferData(GL_ARRAY_BUFFER, data_index * sizeof(GLfloat), vertex_data, GL_STATIC_DRAW);

        glDrawArrays(GL_TRIANGLES, 0, data_index / 4);
        glDisableVertexAttribArray(shader->attr.t);
    }

    return this;
}

Font *
Font::center(const char *format, ...)
{
    static char
        buffer[2048];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    float
        w = this->get_width(buffer) / 2.0f,
        h = this->get_height(buffer) / 2.0f;
    
    this->x -= w;
    this->y += h;

    this->write(buffer);

    this->x += w;
    this->y -= h;
    
    return this;
}

Font *
Font::align_left(const char *format, ...)
{
    static char
        buffer[2048];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    this->write(buffer);
    
    return this;
}

Font *
Font::align_middle(const char *format, ...)
{
    static char
        buffer[2048];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    float w = this->get_width(buffer) / 2.0f;
    
    this->x -= w;

    this->write(buffer);

    this->x += w;
    
    return this;
}

Font *
Font::align_right(const char *format, ...)
{
    static char
        buffer[2048];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    float w = this->get_width(buffer);
    
    this->x -= w;

    this->write(buffer);

    this->x += w;
    
    return this;
}

float
Font::get_width(const char *s)
const
{
    if (this->effects & Font::MONOSPACE)
    {
        return core::str::len(s) * this->size * MONOSPACE_WIDTH;
    }
    
    char  ascii;
    float width = 0.0f, cur_w = 0.0f;
    float w = this->size * 2.0f / screen.scene->w;
    
    while ((ascii = *(s++)))
    {
        if (ascii == '\n')
        {
            width = std::max(width, cur_w);
            cur_w = 0.0f;
        }
        else if (ascii == ' ')
        {
            cur_w += this->size * SPACE_WIDTH;
        }
        else
        {
            Character c = this->character[(int)_char_map[(int)ascii]];
            cur_w += w * c.width + this->size * LETTER_SPACING;

            if (this->effects & Font::BOLD)
            {
                cur_w += (c.end - c.start) * this->size * BOLD_STRENGTH;
            }
        }
    }

    return std::max(cur_w, width);
}

float
Font::get_height(const char *s)
const
{
    char  c;
    float height = this->size * 2.0f / screen.scene->h;
    
    while ((c = *(s++)))
    {
        height += this->size * (c == '\n');
    }

    return height;
}

Font *
Font::get(const char *name, float size, Color stroke, Color background, Color effect)
{
    if (name == NULL)
    {
        name = "normal";
    }
    
    char *path = core::str::format(FONT_DIRECTORY "%s.png", name);
    core::str::set(path, core::str::normalize_path(path));

    Font *font = new Font();

    if (global_cache.contains(path))
    {
        *font = *global_cache[path];
    }
    else
    {
        Font *base_font = new Font();
        core::engine.log("Loading %s font", name);
        base_font->load(path);
        
        *font = *global_cache.store(path, base_font);
    }

    delete[] path;

    font->size             = size;
    font->color.stroke     = stroke;
    font->color.background = background;
    font->color.effect     = effect;
    return font;
}

#define VERT "../../fonts/shaders/vertex"
#define FRAG "../../fonts/shaders/"
bool
Font::load(const char *path)
{
    Sprite *sprite = new Sprite();
    if (!sprite->load(path))
    {
        core::engine.log("(!) Failed to load %s", path);
        delete sprite;
        return false;
    }
    
    if (!_loaded)
    {
        core::engine.log("Initializing font shaders");

        _normal_shader     = Program::get(VERT, FRAG "normal");
        _effect_shader     = Program::get(VERT, FRAG "effect");
        _background_shader = Program::get(FRAG "background_vertex",
            FRAG "background_fragment");

        _loaded = true;

        core::engine.log("Font shaders loaded");
    }
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &this->texture);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    this->character = _char_buffer + _char_count;
    
    // Rebuild bitmap upside down for OpenGL
    unsigned char *data   = new unsigned char[sprite->w * sprite->h * 4];
    bool empty_column     = true, previous_empty;
    float char_start      = 0.0f;
    float char_end        = 0.0f;
    bool quote_gap_passed = false;
    int i = 0;
    
    for (int x = 0; x < sprite->w; ++x)
    {
        const Component *src
            = (Component *)sprite->data + (x + sprite->w * sprite->h) * 4;
        
        previous_empty = empty_column;
        empty_column   = true;
        for (int y = 0; y < sprite->h; ++y)
        {
            src -= sprite->w * 4;

// (!) FIXME: remove upper line?
            i = ((sprite->h - y - 1) * sprite->w + x) * 4;
            i = (y * sprite->w + x) * 4;
            data[i++] = src[2];
            data[i++] = src[1];
            data[i++] = src[0];
            data[i++] = src[3];
            
            empty_column &= ((src[0] | src[1] | src[2]) == 0x00);
            
        }

        // Detect character gaps & advance to the next one
        if (!empty_column)
        {
            float pos = (float)x / sprite->w;
            if (previous_empty)
            {
                // Allow one space within quotation mark (")
                if (!quote_gap_passed
                    && &_char_buffer[_char_count] == &this->character[2])
                {
                    quote_gap_passed = true;
                }
                else
                {
                    
                    _char_buffer[_char_count].start = char_start;
                    _char_buffer[_char_count].end   = char_end;
                    _char_buffer[_char_count].width = char_end - char_start;
                    
                    char_start = pos;
                    _char_count++;
                }
            }

            char_end = pos + 1.0f / sprite->w;
        }
    }

    int characters_loaded = _char_buffer + _char_count - this->character;
    for (int i = 0; i < characters_loaded; ++i)
    {
        this->character[i].width *= (float)characters_loaded;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite->w, sprite->h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);

    delete[] data;

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}
