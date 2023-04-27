#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "program.h"
#include "../../core/engine.h"
#include "../../core/util/string.h"
#include "../../core/util/cache.h"

using namespace gfx;

static core::Cache<Program> global_cache;

Program::Program() :
    id(id_mutable),
    ready(ready_mutable)
{
    this->ready_mutable   = false;
    this->log             = NULL;
    this->vertex_shader   = NULL;
    this->fragment_shader = NULL;
}

Program::~Program()
{
    this->unload();

    global_cache.drop(this, false);
}

Program *
Program::get(const char *vertex, const char *fragment)
{
    Program *program;
    char *key = core::str::cat(vertex, fragment);

    if (!global_cache.contains(key))
    {
        program = new Program();

        program->vertex_shader   = Shader::get_vertex(vertex);
        program->fragment_shader = Shader::get_fragment(fragment);

        program->init();
        if (!program->ready)
        {
            delete program;
            program = NULL;
        }

        global_cache.store(
            key,
            program);
    }
    else
    {
        program = global_cache[key];
    }
    
    delete[] key;
    return program;
}

void
Program::unload(void)
{
    delete[] this->log;
    this->log = NULL;

    if (this->ready)
    {
        core::engine.log("<P%i", this->id);
        glDeleteProgram(this->id);

        this->ready_mutable = false;
    }

    this->vertex_shader   = NULL;
    this->fragment_shader = NULL;
}

void
Program::init(void)
{
    if (this->ready
        || this->vertex_shader == NULL || this->fragment_shader == NULL
        || !(this->vertex_shader->ready && this->fragment_shader->ready)
    )
    {
        return;
    }

    this->ready_mutable = false;
    this->id_mutable    = glCreateProgram();
    
    // Attach and link shaders
    core::engine.log("Linking shaders S%i and S%i to P%i",
        this->vertex_shader->id,
        this->fragment_shader->id,
        this->id);

    glAttachShader(this->id, this->vertex_shader->id);
    glAttachShader(this->id, this->fragment_shader->id);
    glLinkProgram(this->id);
    glDetachShader(this->id, this->vertex_shader->id);
    glDetachShader(this->id, this->fragment_shader->id);

    // Check linker status
    GLint linker_status = GL_TRUE;
    glGetProgramiv(this->id, GL_LINK_STATUS, &linker_status);

    if (linker_status != GL_TRUE)
    {
        core::engine.log("<(FAILED)", this->id);
        core::engine.log("(!) GLSL linker failed:");
        core::engine.log_header(this->get_log());
        
        throw 666;
        return;
    }
    
    // Attributes
    this->attr.v
        = glGetAttribLocation(this->id, "world_pos");
    
    this->attr.n
        = glGetAttribLocation(this->id, "world_normal");
    
    this->attr.t
        = glGetAttribLocation(this->id, "texture_pos");
    
    this->attr.t_weight
        = glGetAttribLocation(this->id, "texture_weight");
    
    // Uniforms
    this->unif.elapsed_time
        = glGetUniformLocation(this->id, "elapsed_time");

    this->unif.projection
        = glGetUniformLocation(this->id, "transformation_matrix");
    
    this->unif.modelview
        = glGetUniformLocation(this->id, "modelview_matrix");
    
    this->unif.normal
        = glGetUniformLocation(this->id, "normal_matrix");
    
    this->unif.ambient_color
        = glGetUniformLocation(this->id, "ambient_color");

    this->unif.diffuse_color
        = glGetUniformLocation(this->id, "diffuse_color");

    this->unif.specular_color
        = glGetUniformLocation(this->id, "specular_color");

    this->unif.specular_exponent
        = glGetUniformLocation(this->id, "specular_exponent");

    this->unif.refractive_index
        = glGetUniformLocation(this->id, "refractive_index");

    this->unif.alpha
        = glGetUniformLocation(this->id, "alpha");

    this->unif.ambient_fog
        = glGetUniformLocation(this->id, "ambient_fog");

    this->unif.light_position
        = glGetUniformLocation(this->id, "light_position");

    this->unif.camera_pos
        = glGetUniformLocation(this->id, "camera_pos");

    // Textures
    this->unif.color_map
        = glGetUniformLocation(this->id, "color_map");

    this->unif.normal_map
        = glGetUniformLocation(this->id, "normal_map");

    this->unif.bump_map
        = glGetUniformLocation(this->id, "bump_map");

    this->unif.specular_map
        = glGetUniformLocation(this->id, "specular_map");

    this->unif.decal_map
        = glGetUniformLocation(this->id, "decal_map");

    this->unif.ambient_occlusion_map
        = glGetUniformLocation(this->id, "ambient_occlusion_map");


    core::engine.log("<(OK)", this->id);
    this->ready_mutable = true;
}

void
Program::use(void)
{
    static GLuint current_program = -1;
    
    if (true || current_program != this->id)
    {
        glUseProgram(this->id);
        current_program = this->id;

        screen.scene->matrix.projection.to(this->unif.projection);

        math::Mat4 M = screen.scene->matrix.mv;
        M.reset_translation();
        math::Vec4 sun = screen.scene->sunlight * M;
        sun.to(this->unif.light_position);
        screen.scene->fog.to(this->unif.ambient_fog);
        screen.scene->camera.pos.to(this->unif.camera_pos);

        glUniform1f(this->unif.elapsed_time, core::engine.ticks);
    }
}

const char *
Program::get_log(void)
{
    delete[] this->log;
    
    int len;
    if (glIsProgram(this->id))
    {
        glGetProgramiv(this->id, GL_INFO_LOG_LENGTH, &len);
        this->log = new char[len];
        glGetProgramInfoLog(this->id, len, &len, this->log);
    }
    else if (glIsShader(this->id))
    {
        this->log = core::str::format(
            "Name S%i is a shader, not a program",
            this->id);
    }
    else
    {
        this->log = core::str::format(
            "Name #%i is neither a shader nor a program",
            this->id);
    }
    
    return this->log;
}

void
Program::flush_cache(void)
{
    global_cache.flush();
}
