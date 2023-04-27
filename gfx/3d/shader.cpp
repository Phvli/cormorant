#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#define SHADER_PATH "video/shaders/"
#include "shader.h"

#include "../../core/engine.h"
#include "../../core/util/file.h"
#include "../../core/util/string.h"
#include "../../core/util/cache.h"
#include "../../core/util/config.h"

using namespace gfx;

static core::Cache<Shader> global_cache;

Shader::Shader() :
    id(id_mutable),
    type(type_mutable),
    ready(ready_mutable),
    filename(filename_mutable)
{
    this->id_mutable       = -1;
    this->ready_mutable    = false;
    this->filename_mutable = NULL;
    this->log              = NULL;
}

Shader::~Shader()
{
    this->unload();

    global_cache.drop(this, false);
}

void
Shader::unload(void)
{
    if (this->ready)
    {
        core::engine.log("<S%i", this->id);
        glDeleteShader(this->id);
    }
    
    delete[] this->log;
    delete[] this->filename_mutable;
    
    this->id_mutable       = -1;
    this->ready_mutable    = false;
    this->filename_mutable = NULL;
    this->log              = NULL;
}

Shader *
Shader::get_vertex(const char *filename)
{
    const char *s = core::str::cat("vertex/", filename);
    Shader *result = Shader::get(s);
    delete[] s;
    
    return result;
}

Shader *
Shader::get_fragment(const char *filename)
{
    const char *s = core::str::cat("fragment/", filename);
    Shader *result = Shader::get(s);
    delete[] s;
    
    return result;
}

Shader *
Shader::get(const char *filename)
{
    if (!global_cache.contains(filename))
    {
        global_cache.store(
            filename,
            Shader::load(filename));
    }

    return global_cache[filename];
}

static char *
_compose_glsl(const char *directory, const char *file)
{
    char *filename = core::str::strip_duplicate(core::str::dup(file), ' ');
    core::Config *shader_filenames = core::str::split(filename);
    core::str::set(filename, NULL);

    core::Config shader_globals;
    core::Config shader_functions;
    core::Config shader_statements;
    
    // Load shader files (filenames separated by spaces)
    char *source = core::str::empty();
    for (core::Config::Value::iterator i = shader_filenames->begin();
        i != shader_filenames->end(); ++i)
    {
        core::str::set(filename,
            core::str::cat(directory, i->second->string()));

        // Add file extension if omitted
        if (core::str::icmp(".glsl", core::str::last(filename, 5)))
        {
            core::str::set(filename, core::str::cat(filename, ".glsl"));
        }

        // Load shader file
        core::File glsl(filename);

        if (glsl.exists())
        {
            char *main = core::str::get_filename(filename);
            shader_functions[core::Config::NEXT] = main;
            
            char *s = core::str::strip_duplicate(
                core::str::dup(glsl.get_contents()),
                ' ');

            core::str::set(main, core::str::cat("void _", main));
            core::str::set(s, core::str::replace(s, "void main", main));
            
            core::str::set(source, core::str::cat(source, s));
            delete[] s;
            delete[] main;
        }
        else
        {
            core::engine.log("(!) File not found: %s", glsl.filename);
        }
    }
    
    char *version = NULL;
    
    // List keywords that cause statements to be included only once (NULL-terminated)
    static const char *global_keywords[] = {
        "#define ",
        "#version ",
        "#pragma ",
        "attribute ",
        "centroid ",
        "const ",
        "flat ",
        "in ",
        "layout (",
        "layout(",
        "out ",
        "smooth ",
        "uniform ",
        "varying ",
    };

    // Parse file (define globals only once, combine main functions)
    int indent = 0;
    bool comment = false;
    for (char *start = source, *s = source; *s != '\0'; ++s)
    {
        switch (*s)
        {
            case '/':
                if (s[1] == '/' && !comment)
                {
                    for (s++; *s != '\n' && *s != '\0'; s++);

                    if (!indent)
                    {
                        start = s;
                    }
                    s--;
                }
                else if (s[1] == '*')
                {
                    comment = true;
                    s++;
                }
                break;
                
            case '*':
                if (s[1] == '/')
                {
                    s++;
                    comment = false;

                    if (!indent)
                    {
                        start = s;
                    }
                }
                break;
            
            case '{':
                indent += !comment;
                break;
            
            case '#':
                if (comment)
                {
                    break;
                }

                for (s++; *s != '\n' && *s != '\0'; s++);
                if (*s == '\0')
                {
                    s--;
                    break;
                }
                    
                indent++;
                // Don't break, just fall through (goto is life <3)...

            case '}':
                indent -= !comment;
                // Don't break, just fall through (goto is life <3)...
            
            case ';':
                // Cache a new, complete statement
                if (comment == false && indent == 0)
                {
                    char *statement = core::str::substring(start, s);
                    core::str::set(statement, core::str::strip(statement, '\t'));
                    core::str::set(statement, core::str::ltrim(statement));
                    start = s + 1;
                    
                    if (core::str::ncmp(statement, "#version", 8) == 0)
                    {
                        if (version != NULL && core::str::cmp(version, statement))
                        {
                            core::engine.log("(!) Conflicting #version directives, omitting %s", statement);
                        }
                        else
                        {
                            core::str::set(version, statement);
                        }

                        break;
                    }
                    else if (core::str::ncmp(statement, "attribute", 9) == 0
                        || core::str::ncmp(statement, "varying", 7) == 0)
                    {
                        core::engine.log("(!) Deprecated GLSL \"%s\"", statement);
                    }
                    
                    const char **keyword;
                    
                    for (keyword = global_keywords; *keyword != NULL
                        && core::str::ncmp(*keyword, statement,
                            strlen(*keyword));
                        ++keyword);

                    if (*keyword != NULL)
                    {
                        // Add to globals (index to keep globals unique)
                        shader_globals[statement] = statement;
                    }
                    else
                    {
                        // Add as a new statement
                        // (these are usually full function blocks)
                        shader_statements[core::Config::NEXT]
                            = statement;
                    }
                    
                    delete[] statement;
                }
                
                break;
        }
    }
    
    // Include cached globals
    core::str::set(source, core::str::join(&shader_globals, '\n'));
    core::str::set(source, core::str::cat(source, '\n'));

    // Add version statement
    if (version == NULL)
    {
        version = core::str::dup("#version 330 core");
    }
    core::str::set(version, core::str::cat(version, "\n"));
    core::str::set(source, core::str::cat(version, source));
    delete[] version;

    // Include all statements
    char *s = core::str::join(&shader_statements, '\n');
    core::str::set(source, core::str::cat(source, s));
    delete[] s;

    // Build a new main function (calls old shader functions in succession)
    core::str::set(source, core::str::cat(source, "\nvoid main(void) {\n"));
    for (core::Config::Value::const_iterator i = shader_functions.begin();
        i != shader_functions.end(); ++i)
    {
        s = core::str::format("    _%s();\n", i->second->string());
        core::str::set(source, core::str::cat(source, s));
        delete[] s;
    }
    core::str::set(source, core::str::cat(source, '}'));
    
    // core::engine.log("PARSED:\n----------\n%s\n----------\n", source);

    delete[] filename;
    delete shader_filenames;
    return source;
}


Shader *
Shader::load(const char *path)
{
    Shader *shader = new Shader();
    core::str::set(shader->filename_mutable, core::str::cat(SHADER_PATH, path));

    // Determine shader type from its directory
    char *directory = core::str::get_directory(shader->filename);
    char *file      = core::str::get_filename(shader->filename);

    if (core::str::ifind(directory, "vertex/") != NULL)
    {
        shader->type_mutable = GL_VERTEX_SHADER;
        core::engine.log("New vertex shader \"%s\"", file);
    }
    else if (core::str::ifind(directory, "fragment/") != NULL)
    {
        shader->type_mutable = GL_FRAGMENT_SHADER;
        core::engine.log("New fragment shader \"%s\"", file);
    }
    else
    {
        core::engine.log("<'%s'", path);
        core::engine.log("(!) Invalid shader path: %i", shader->filename);

        delete[] directory;
        delete[] file;
        delete shader;

        return NULL;
    }
    shader->id_mutable = glCreateShader(shader->type);

    // Load and compile shader
    const char *s[1];
    s[0] = _compose_glsl(directory, file);

    delete[] directory;
    delete[] file;

    glShaderSource(shader->id, 1, s, NULL);
    glCompileShader(shader->id);
    delete s[0];

    GLint success = GL_FALSE;
    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &success);
    
    shader->ready_mutable = (success == GL_TRUE);
    if (shader->ready)
    {
        core::engine.log("<known as S%i", shader->id);
    }
    else
    {
        core::engine.log("<(S%i FAILED)", shader->id);
        core::engine.log("(!) Shader failed to compile:");
        core::engine.log_header(shader->get_log());
        delete shader;
        
        throw 666;
        return NULL;
    }

    return shader;
}

const char *
Shader::get_log(void)
{
    delete[] this->log;
    
    int len;
    if (glIsShader(this->id))
    {
        glGetShaderiv(this->id, GL_INFO_LOG_LENGTH, &len);
        this->log = new char[len];
        glGetShaderInfoLog(this->id, len, &len, this->log);
    }
    else if (glIsProgram(this->id))
    {
        this->log = core::str::format(
            "Name P%i is a program, not a shader",
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
Shader::flush_cache(void)
{
    global_cache.flush();
}
