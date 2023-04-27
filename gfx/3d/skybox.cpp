#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "skybox.h"


#include "../../core/engine.h"
#include "../../core/util/string.h"

#include "model.h"
#include "mesh.h"
#include "material.h"
#include "program.h"

using namespace gfx;

Skybox::Skybox()
{
    this->mesh = NULL;
}

Skybox::Skybox(const char *filename)
{
    this->mesh = NULL;
    this->load(filename);
}

Skybox::~Skybox()
{
    delete this->mesh;
}

bool
Skybox::load(const char *filename)
{
    char *path = core::str::cat("video/skybox/", filename);

    this->mesh = gfx::primitive::cube(false);
    this->mesh->flip().compose();
    this->mesh->material            = new gfx::Material();
    this->mesh->material->color_map = gfx::Texture::get(path, gfx::Texture::CUBEMAP);
    this->mesh->material->shader    = gfx::Program::get("skybox", "skybox");
    this->mesh->material->compose();
    
    delete[] path;
    
    return true;
}

void
Skybox::render(void)
{
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    Mesh     *mesh     = this->mesh;
    Material *material = mesh->material;
    Program  *shader   = material->shader;
    material->use();

    // Bind
    glBindBuffer(GL_ARRAY_BUFFER, mesh->attr.pos);
    glEnableVertexAttribArray(shader->attr.v);
    glVertexAttribPointer(shader->attr.v, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(GLfloat), NULL);
    
    math::Mat4 transformation = screen.scene->matrix.camera;
    transformation
        .reset_translation()
        .to(shader->unif.modelview);

    // Render
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->attr.index);
    glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, NULL);

    // Unbind
    glDisableVertexAttribArray(shader->attr.v);
    glUseProgram(0);
    
    glDepthMask(GL_TRUE);
}
