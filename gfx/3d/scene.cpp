#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "scene.h"


#include <algorithm> // min, max, find
#include <cmath>     // pow

#include "../../core/util/string.h"

#include "model.h"
#include "mesh.h"
#include "material.h"
#include "program.h"
#include "../../math/vec3.h"
#include "../../math/util.h"

#include "../../core/engine.h"
#include "../../game/entity.h"
#include "../../game/terrain.h"

using namespace gfx;


#define _CLOUDS        7
#define _CLOUD_SIZE    50
#define _CLOUD_SPRITES 4
#include "../../gfx/3d/scenery/clouds.h"

static Mesh *_cloud_mesh[_CLOUD_SPRITES];

typedef
    struct {
        float dist;
        float opacity;
        math::Vec3 pos;
        math::Mat4 mv;
        math::Mat4 local;
    }
    _CloudSprite;

static struct
{
    math::Vec3 pos;
    int size;
    _CloudSprite sprite[_CLOUD_SPRITES][_CLOUD_SIZE];
} _cloud[_CLOUDS];

static void
_move_cloud(int c, const math::Vec3 &pos)
{
    float size = math::rnd(.75, 2.0);
    int cumulusness = math::rnd(15, 50);

    float
        width  = math::rnd(1500.0f, 2500.0f) * size,
        height = math::rnd(7.5f, 10.0f) * cumulusness * size,
        snaking = math::rnd(math::DOUBLE_PI);
    
    _cloud[c].pos = pos;
    if (_cloud[c].pos.y == 0.0f)
    {
        _cloud[c].pos.y = math::rnd(2000, 3000);
    }
    
    math::Vec3 p = math::Vec3(0.0f);

    _cloud[c].size = math::clamp(size * _CLOUD_SIZE, 0, _CLOUD_SIZE);
    for (int s = 0; s < _CLOUD_SPRITES; ++s)
    {
        for (int n = 0; n < _cloud[c].size; ++n)
        {
            float
                a = math::rnd(math::DOUBLE_PI),
                r = math::transition::ease_out(1.0f, width, math::rnd());
            
            _cloud[c].sprite[s][n].pos
                = p + math::Vec3(
                    r * cos(a),
                    math::vary(height * (r / width)),
                    r * sin(a)
                );

            _cloud[c].sprite[s][n].local
                = math::Mat4::translation(_cloud[c].sprite[s][n].pos);

            _cloud[c].sprite[s][n].mv
                = math::Mat4(_cloud[c].sprite[s][n].local)
                    .translate(_cloud[c].pos);

            if ((n % cumulusness) == 0)
            {
                float step = math::rnd(.2f, 1.0f) * width;
                snaking += math::vary(.2f);
                p += math::Vec3(
                    step * cos(snaking), 0.0f,
                    step * sin(snaking));
            }
        }

    }
}

static void
_init_sky(void)
{
    Material *cloud_material = Material::add("clouds");
    cloud_material->shader   = Program::get("cloud", "cloud");
    
    cloud_material->color_map
        = Texture::get("video/textures/effects/cloud.png",
            Texture::NEAREST | Texture::NO_MIPMAP);
    
    cloud_material->compose();

    for (int s = 0; s < _CLOUD_SPRITES; ++s)
    {
        _cloud_mesh[s] = &(*primitive::quad())
            .scale(1500, 750, 1.0)
            .scale_uv(1.0f / _CLOUD_SPRITES, 1.0f)
            .translate_uv((float)s / _CLOUD_SPRITES, 0.0f);
        _cloud_mesh[s]->material = cloud_material;
        _cloud_mesh[s]->compose();
    }
    
    for (int c = 0; c < _CLOUDS; ++c)
    {
        _move_cloud(c, math::Vec3(
            math::vary(25000),
            0.0f,
            math::vary(25000)));
    }
}

#include <vector>
#include <algorithm>
static bool
_sort_clouds(_CloudSprite *a, _CloudSprite *b)
{
    return a->dist > b->dist;
}

static void
_render_sky(void)
{
    float
        view_range  = 2.0f * core::engine.config
            ["video"]["detail"]["view_range"]
            .real(6000.0f),
        view_range2 = view_range * view_range;
    
    typedef
        std::vector<_CloudSprite *>
        RenderBuffer;

    RenderBuffer renderbuffer[_CLOUD_SPRITES];

    for (int c = 0; c < _CLOUDS; ++c)
    {
        float dist = _cloud[c].pos.dist_sq(screen.scene->camera.pos);

        if (dist > view_range2)
        {
            if (dist > view_range2 * 1.1f)
            {
                // reposition faraway clouds
                float a = math::rnd(math::DOUBLE_PI);
                _move_cloud(c, math::Vec3(
                    screen.scene->camera.x + cos(a) * view_range,
                    0.0f,
                    screen.scene->camera.z + sin(a) * view_range));

                c--; // try again
            }
            
            continue;
        }

        float
            lod   = math::transition::smooth(1.0f, 0.0f, dist / view_range2),
            quads = _cloud[c].size * math::min(1.0f, lod),
            fog   = math::clamp(
                    1.0f - .5f
                        * screen.scene->fog.w * sqrt(dist),
                    0.0, 1.0f);

        for (int q = 0; q < quads; ++q)
        {
            float opacity = 0.2f * lod
                * math::clamp((quads - q) / 2.0f, 0.0f, 1.0f)
                * fog;

                ;

            for (int s = 0; s < _CLOUD_SPRITES; ++s)
            {
                _cloud[c].sprite[s][q].opacity = opacity;

                _cloud[c].sprite[s][q].dist = _cloud[c].sprite[s][q].pos
                    .dist_sq(screen.scene->camera.pos);
                
                renderbuffer[s].push_back(&_cloud[c].sprite[s][q]);
            }
        }
    }

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    
    math::Mat4 base_rot
        = math::Mat4::rotationY(screen.scene->camera.rot.vec3().y);

    for (int s = 0; s < _CLOUD_SPRITES; ++s)
    {
        Mesh *mesh = _cloud_mesh[s];
        mesh->material->use();
        Program *shader = mesh->material->shader;

        glEnableVertexAttribArray(shader->attr.v);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->attr.pos);
        glVertexAttribPointer(shader->attr.v, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glEnableVertexAttribArray(shader->attr.t);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->attr.uv);
        glVertexAttribPointer(shader->attr.t, 2, GL_FLOAT, GL_FALSE, 0, NULL);

        std::sort(renderbuffer[s].begin(), renderbuffer[s].end(),
            _sort_clouds);

        for (RenderBuffer::const_iterator c = renderbuffer[s].begin();
            c != renderbuffer[s].end(); ++c)
        {
            // whiteness
            glUniform1f(shader->unif.ambient_color,
                .7f + 0.0007f * (*c)->pos.y);
            
            glUniform1f(shader->unif.alpha, (*c)->opacity);
            
            (base_rot * (*c)->mv * screen.scene->matrix.camera)
            .to(shader->unif.modelview);
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        // Unbind
        glDisableVertexAttribArray(shader->attr.v);
        glDisableVertexAttribArray(shader->attr.t);
    }
    glUseProgram(0);
    glDepthMask(GL_TRUE);
}


#include <cstdlib>
Scene::Scene():
    frame(frame_mutable),
    w(w_mutable),
    h(h_mutable),
    zoom(zoom_mutable),
    fov(fov_mutable),
    aspect_ratio(aspect_ratio_mutable),
    clip_near(clip_near_mutable),
    clip_far(clip_far_mutable),
    perspective(perspective_mutable)
{
    this->player = NULL;
    // this->entities.reserve(2048);

    this->frame_mutable       = 0;
    this->w_mutable           = 128;
    this->h_mutable           = 128;
    this->fov_mutable         = 60.0f;
    this->zoom_mutable        = 1.0f;
    this->perspective_mutable = true;

    this->clip_near_mutable = 0;
    this->clip(0.2f, 9000.0f);

    this->matrix.mv     = math::Mat4::identity();
    this->matrix.camera = math::Mat4::identity();
    this->matrix.normal = math::Mat4::identity();
    
    this->sunlight  = math::Vec4(
        math::Vec3(0.1f, 5.0f, 1.0f).normalize(),
        1.0f);
    
    this->skybox = NULL; // new Skybox("beach.jpg");

    float
        ocean_scale   = 64.0f,
        ocean_horizon = core::engine.config
            ["video"]["detail"]["view_range"]
            .real(6000.0f) - game::TerrainChunk::SIZE / 2.0f;

    this->waves = primitive::grid(1.5f * ocean_horizon / ocean_scale);

    // (!) FIXME: lol this is not the proper way to do a circular mesh, code in a better one
    for (Mesh::Vertices::iterator v = this->waves->vertices.begin();
        v != this->waves->vertices.end(); ++v)
    {
        if (v->pos.length() > ocean_horizon / ocean_scale)
        {
            v->pos.y--;
        }
    }
    this->waves->clip();
    
    this->waves->scale(ocean_scale, false);
    this->waves->translate(0.0f, -2.0f, 0.0f);
    this->waves->scale_uv(ocean_scale, ocean_scale);
    this->waves->compose();
    this->waves->material         = Material::add("waves");
    this->waves->material->shader = Program::get("waves", "waves fog");
    this->waves->material->compose();
    
    this->ground_plane = NULL;

    this->fog = math::Vec4(
        0.78f, 0.95f, 1.00f,
        0.00005f
    );
    
    _init_sky();
}

Scene::~Scene()
{
    for (EntityContainer::iterator i = this->entities.begin();
        i != this->entities.end(); ++i)
    {
        delete *i;
    }
    
    delete this->skybox;
    delete this->waves;

    delete this->ground_plane;

    for (int i = 0; i < _CLOUD_SPRITES; ++i)
    {
        delete _cloud_mesh[i];
    }
}


typedef
    struct
    {
        game::Entity *entity;
        long          dist;
    }
    _PostRender;

static bool
_sort_post_render(_PostRender a, _PostRender b)
{
    return a.dist > b.dist;
}

void
Scene::render(void)
{
    if (!game::terrain.w)
    {
        return;
    }
    
    this->frame_mutable++;

    this->camera.rot.normalize();
    this->matrix.camera = math::Mat4::identity()
        .translate(-this->camera.pos)
        * -this->camera.rot;

    glClearColor(this->fog.x, this->fog.y, this->fog.z, 1.0f);
    if (this->skybox && this->camera.y >= 0)
    {
        this->skybox->render();
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    else
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Planar horizon
    this->render_ground_plane();

    this->clip(2.0f,
        2.0f * core::engine.config
            ["video"]["detail"]["view_range"]
            .real(6000.0f));

    // Terrain
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    game::terrain.render();

    // Entities
    glEnable(GL_CULL_FACE);
    
    static std::vector<_PostRender> post_render;
    

    post_render.clear();
    for (EntityContainer::iterator entity = this->entities.begin();
        entity != this->entities.end(); ++entity)
    {
        if ((*entity)->flags & game::Entity::TRANSLUCENT)
        {
            _PostRender p = {
                *entity,
                (long)(*entity)->pos.dist_sq(this->camera.pos)
            };
            post_render.push_back(p);
        }
        else
        {
            (*entity)->graphics->render();
        }
    }

    // Waves
    glDisable(GL_CULL_FACE);
    this->matrix.mv
        = math::Mat4::translation(this->camera.pos.x, 0.0f, this->camera.pos.z)
        * this->matrix.camera;
    this->waves->render();

    // Clouds
    _render_sky();

    // Translucent entities
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    std::sort(post_render.begin(), post_render.end(),
        _sort_post_render);

    // for (EntityContainer::iterator entity = post_render.begin();
    for (std::vector<_PostRender>::iterator entity = post_render.begin();
        entity != post_render.end(); ++entity)
    {
        entity->entity->graphics->render();
        // (*entity)->graphics->render();
    }
    glDepthMask(GL_TRUE);
}

void
Scene::clip(float near, float far)
{
    if ((near != this->clip_near) || (far != this->clip_far))
    {
        this->clip_near_mutable = near;
        this->clip_far_mutable = far;
        this->update_projection();
    }
}

void
Scene::update_projection(void)
{
    this->aspect_ratio_mutable = (float)this->w / (float)this->h;

    this->matrix.projection = (this->perspective)

        ? math::Mat4::perspective(
            (this->fov / this->zoom) * 3.141593f / 180.0f,
            this->aspect_ratio,
            this->clip_near,
            this->clip_far)

        // (!) FIXME: ortographic zoom
        : math::Mat4::ortho(
            0,
            this->aspect_ratio,
            1.0f,
            0.0f,
            this->clip_near,
            this->clip_far);
}

void
Scene::update(void)
{
    EntityContainer
        update_queue = this->entities,
        delete_queue;
    
    for (EntityContainer::iterator entity = update_queue.begin();
        entity != update_queue.end(); ++entity)
    {
        (*entity)->update();
        
        if ((*entity)->flags & game::Entity::DELETED)
        {
            delete_queue.push_back(*entity);
        }
    }

    for (EntityContainer::iterator entity = delete_queue.begin();
        entity != delete_queue.end(); ++entity)
    {
        this->remove(*entity, true);
    }
}

void
Scene::build_ground_plane(void)
{
    core::engine.log("Painting horizon");
    if (this->ground_plane != NULL)
    {
        delete this->ground_plane->material->color_map;
        delete this->ground_plane;
    }

    float
        x = game::Terrain::MAX_VISIBILITY
            / ((float)game::terrain.w * game::TerrainNode::SIZE),
        z = game::Terrain::MAX_VISIBILITY
            / ((float)game::terrain.h * game::TerrainNode::SIZE);

    this->ground_plane = &(*gfx::primitive::circle(64))
        .scale(game::Terrain::MAX_VISIBILITY / 2.0f, false)
        .transform_uv(math::Mat4::translation(-0.5f, -0.5f).scale(x, z));
    
    int
        size = core::engine.config
            ["video"]["detail"]["horizon"]
            .integer(512);
    
    this->ground_plane->compose();
    this->ground_plane->material         = gfx::Material::add("ground_plane");
    this->ground_plane->material->shader = gfx::Program::get("ground_plane", "textured fog");
    this->ground_plane->material->color_map
        = gfx::Texture::load(game::terrain.minimap(size, size),
            gfx::Texture::CLAMP | gfx::Texture::FLIP_Y);
    this->ground_plane->material->compose();
}

void
Scene::render_ground_plane(void)
{
    glDisable(GL_DEPTH_TEST);
    math::Vec3 camera_pos_backup = this->camera.pos;

    this->clip(.3f
        * core::engine.config
            ["video"]["detail"]["view_range"]
            .real(6000.0f),
        game::Terrain::MAX_VISIBILITY);

    this->matrix.mv
        = math::Mat4::translation(this->camera.pos.x, 0.0f, this->camera.pos.z)
        * this->matrix.camera;

    this->camera.pos.x /= (float)game::terrain.w * game::TerrainNode::SIZE;
    this->camera.pos.z /= (float)game::terrain.h * game::TerrainNode::SIZE;
    this->ground_plane->render();

    glClear(GL_DEPTH_BUFFER_BIT);
    this->camera.pos = camera_pos_backup;
}

game::Entity *
Scene::add(game::Entity *entity)
{
    this->entities.push_back(entity);
    return entity;
}

game::Entity *
Scene::remove(game::Entity *entity, bool destroy)
{
    EntityContainer::iterator i
        = std::find(this->entities.begin(), this->entities.end(), entity);

    if (i != this->entities.end())
    {
        this->entities.erase(i);
    }
    
    if (destroy)
    {
        delete entity;
        entity = NULL;
    }
    
    return entity;
}
