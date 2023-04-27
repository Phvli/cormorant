#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "sprite.h"

#include "../../../core/util/string.h"
#include "../../../math/util.h"
#include "../../../core/engine.h"
// #include "../../../gfx/3d/scene.h"
#include "../../../gfx/3d/mesh.h"
#include "../../../gfx/3d/program.h"
#include "../../../gfx/3d/texture.h"
#include "../../../gfx/3d/primitives.h"

using namespace game;

static gfx::Mesh
    *_sprite_mesh = NULL;


SpriteGraphics *
SpriteGraphics::clone(void)
{
    return new SpriteGraphics(*this);
}

SpriteGraphics::SpriteGraphics()
{
    this->angular_speed    = math::vary(.001f);
    this->animation_frames = 1;
}

SpriteGraphics::SpriteGraphics(const SpriteGraphics &graphics)
{
    this->visible          = true;
    this->opacity          = 1.0f;
    this->size             = graphics.size;
    this->fading_speed     = graphics.fading_speed;
    this->angular_speed    = math::vary(graphics.angular_speed);
    this->animation_frames = graphics.animation_frames;
    this->material         = graphics.material;
}

SpriteGraphics::SpriteGraphics(const char *texture, int animation_frames, float size, float fading_speed, float angular_speed)
{
    char *s = core::str::cat("video/textures/effects/", texture);
    
    this->visible          = true;
    this->opacity          = 1.0f;
    this->size             = size;
    this->angular_speed    = math::vary(angular_speed);
    this->fading_speed     = fading_speed;
    this->animation_frames = animation_frames;

    if ((this->material = gfx::Material::get(s)) == NULL)
    {
        if (_sprite_mesh == NULL)
        {
            _sprite_mesh = gfx::primitive::quad();
            _sprite_mesh->compose();
        }
        
        this->material            = gfx::Material::add(s);
        this->material->shader    = gfx::Program::get("sprite", "sprite");
        this->material->color_map = gfx::Texture::get(s);
        
        this->material->compose();
    }
    
    delete[] s;
}

SpriteGraphics::~SpriteGraphics()
{
}

void
SpriteGraphics::update(void)
{
    if ((this->opacity -= this->fading_speed) <= 0.0f)
    {
        this->entity->destroy();
    }
}

void
SpriteGraphics::render(void)
{
    if (this->visible)
    {
        gfx::Program *shader = this->material->shader;
        this->material->use();

        glEnableVertexAttribArray(shader->attr.v);
        glBindBuffer(GL_ARRAY_BUFFER, _sprite_mesh->attr.pos);
        glVertexAttribPointer(shader->attr.v, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glEnableVertexAttribArray(shader->attr.t);
        glBindBuffer(GL_ARRAY_BUFFER, _sprite_mesh->attr.uv);
        glVertexAttribPointer(shader->attr.t, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        
        glUniform2i(shader->unif.decal_map,
            (1.0f - math::clamp(this->opacity, 0.0f, 1.0f))
            * this->animation_frames * this->animation_frames,
            this->animation_frames);
        
        glUniform1f(shader->unif.alpha, math::clamp(this->opacity, 0.0f, 1.0f));
        
        (math::Mat4::scaling(this->size)
        .rotZ(this->angular_speed * core::engine.ticks)
        .rotY(screen.scene->camera.rot.vec3().y)
        .translate(this->entity->pos)
        * screen.scene->matrix.camera
        ).to(shader->unif.modelview);
        
        screen.scene->matrix.projection.to(shader->unif.projection);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Unbind
        glDisableVertexAttribArray(shader->attr.v);
        glDisableVertexAttribArray(shader->attr.t);
    }
}
