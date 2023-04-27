#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "chunk.h"

#include "../../math/vec3.h"
#include "../../core/engine.h"
#include "../../core/util/string.h"
#include "../../math/util.h"

#include <cstdlib> // rand
#include <cmath>   // sqrt

using namespace game;

static int
    _refcount = 0;

static struct
{
    gfx::Program *shader  = NULL;
    GLuint pos;
} _billboard;

TerrainChunk::TerrainChunk()
{
    this->init();
}

TerrainChunk::TerrainChunk(int x, int z)
{
    this->init();
    this->generate(x, z);
}

void
TerrainChunk::init(void)
{
    this->mesh  = NULL;

    for (int lod = 0; lod < TerrainChunk::VEGETATION_LOD; ++lod)
    {
        this->trees[lod] = NULL;
    }
    
    _refcount++;
    if (_refcount == 1)
    {
        _billboard.shader  = gfx::Program::get("billboard", "billboard");

        GLfloat
            pos[8] = {
                -0.5f, +1.0f,
                -0.5f,  0.0f,
                +0.5f, +1.0f,
                +0.5f,  0.0f
            };

        glGenBuffers(1, &_billboard.pos);
        glBindBuffer(GL_ARRAY_BUFFER, _billboard.pos);
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &pos, GL_STATIC_DRAW);
    }
}

TerrainChunk::~TerrainChunk()
{
    for (Props::iterator prop = this->props.begin();
        prop != this->props.end(); ++prop)
    {
        delete *prop;
    }
    
    delete this->mesh;

    for (int lod = 0; lod < TerrainChunk::VEGETATION_LOD; ++lod)
    {
        delete this->trees[lod];
    }
    
    if (--_refcount == 0)
    {
        glDeleteBuffers(1, &_billboard.pos);
        // glDeleteBuffers(2, _billboard.attr);
    }
}

TerrainChunk::Prop::Prop()
{
}

void
TerrainChunk::generate(int x, int z)
{
    delete this->mesh;
    this->mesh = new gfx::Mesh();
    
    int subdiv = core::engine.config["video"]["detail"]["terrain"].integer(5);
    int cells  = subdiv * subdiv;
    this->mesh->vertices.reserve(cells * 4);
    this->mesh->indices.reserve(cells * 6);

    struct
    {
       float x, z;
    } offset[4] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

    float scale = (float)TerrainChunk::SIZE / subdiv;
    for (int i = 0; i < 4; ++i)
    {
        offset[i].x = x + scale * offset[i].x;
        offset[i].z = z + scale * offset[i].z;
    }

    int base = 0;
    for (int local_z = 0; local_z < subdiv; ++local_z)
    {
        for (int local_x = 0; local_x < subdiv; ++local_x)
        {
            for (int i = 0; i < 4; ++i)
            {
                int v = this->mesh->vertices.size();

                float
                    x_src = scale * local_x + offset[i].x,
                    z_src = scale * local_z + offset[i].z;
                    
                math::Vec3
                    A = math::Vec3(0.0f,  0.0f,  0.0f),
                    B = math::Vec3(scale, 0.0f,  0.0f),
                    C = math::Vec3(0.0f,  0.0f, -scale);
                    
                game::TerrainNode node = game::terrain.at(x_src, z_src);

                A.y = node.height;
                B.y = game::terrain.at(x_src + B.x, z_src + B.z).height;
                C.y = game::terrain.at(x_src + C.x, z_src + C.z).height;
                
                this->mesh->add(
                    // Vertex position
                    math::Vec3(
                        x_src,
                        node.height,
                        // node.height + (1.0f - node.vegetation)
                        //     * math::perlin_noise(x_src, z_src, .5, 2),
                        z_src),
                    
                    // Face normal
                    (B - A).cross(C - A).normalize()
                );

                // Texture weights
                this->mesh->vertices[v].uv_weight = math::Vec4(
                    node.texture[0],
                    node.texture[1],
                    node.texture[2],
                    node.texture[3]
                );
            }

            this->mesh->add(base, base + 2, base + 1);
            this->mesh->add(base, base + 3, base + 2);
            base += 4;
        }
    }
    
    for (gfx::Mesh::Vertices::iterator v = this->mesh->vertices.begin();
        v != this->mesh->vertices.end(); ++v)
    {
        // Scale ground texture
        v->uv = math::Vec2(v->pos.x, v->pos.z) * .005f;
    }

    for (int prop_count = core::engine.config["video"]["detail"]["cities"].integer(50); prop_count > 0; --prop_count)
    {
        math::Vec3 pos(x + rand() % TerrainChunk::SIZE, 0.0f,
            z + rand() % TerrainChunk::SIZE);

        game::TerrainNode node = game::terrain.at(pos);
        pos.y = node.height;
        
        gfx::Model *model[TerrainChunk::LOD_LEVELS];
        bool exists = false;
        
        if (node.height < 6.0f)
        {
            continue;
        }
        else if (node.vegetation < .1f)
        {
            
            if (node.vegetation || node.height > 80.0f || math::probability(.3f))
            {
                if (math::probability(.8f + .0003f * node.height))
                {
                    continue;
                }
                exists = game::terrain.get_prop(model, Terrain::HOUSE);
            }
            else
            {
                exists = game::terrain.get_prop(model,
                    (math::probability(.05f))
                        ? Terrain::HIGHRISE
                        : Terrain::TOWNHOUSE);
            }
            
            pos.y -= Terrain::BUILDING_STEM * 3.2f;
        }
        // else if (node.vegetation > .5f)
        // {
        //     exists = game::terrain.get_prop(model, Terrain::TREE);
        // }
        else
        {
            continue;
            // Terrain::PropType type;
            // do
            // {
            //     type = static_cast<Terrain::PropType>(
            //         rand() % Terrain::PROP_TYPE_COUNT);

            // } while (type == Terrain::HOUSE || type == Terrain::TOWNHOUSE || type == Terrain::HIGHRISE);

            // exists = game::terrain.get_prop(model, type);
        }
        
        if (!exists)
        {
            continue;
        }
        
        TerrainChunk::Prop *prop = new TerrainChunk::Prop();
        for (int lod = 0; lod < TerrainChunk::LOD_LEVELS; ++lod)
        {
            prop->model[lod] = model[lod];
        }

        math::Vec3 size = prop->model[0]->size();
        float radius = sqrt(size.x * size.x + size.z * size.z);
        
        prop->transformation = math::Mat4::identity()
            .rotY(math::rnd(math::PI))
            .translate(pos.x, pos.y, pos.z);
        
        prop->scaled = math::Mat4::scaling(radius, size.y, radius)
            .translate(pos.x, pos.y, pos.z);
        
        this->props.push_back(prop);
    }

    this->mesh->clip();
    this->mesh->compose();
    
    this->generate_forest(x, z);
}

void
TerrainChunk::generate_forest(int x, int z)
{
    for (int lod = 0; lod < TerrainChunk::VEGETATION_LOD; ++lod)
    {
        delete this->trees[lod];
        this->trees[lod] = NULL;
        
        int
            trees = (lod + 1) * core::engine.config
                ["video"]["detail"]["vegetation"]
                .integer(128),
            total = 0;
        
        GLuint
            *index = new GLuint[trees * 12], *i = index;

        GLfloat
            *pos   = new GLfloat[trees * 24], *p = pos,
            *uv    = new GLfloat[trees * 16], *t = uv;
        
        while (trees-- > 0)
        {
            math::Vec3 v(x + rand() % TerrainChunk::SIZE, 0.0f,
                z + rand() % TerrainChunk::SIZE);
            
            float
                height = math::rnd(10, 20),
                radius = height / 2.0f;

            game::TerrainNode node = game::terrain.at(v);
            v.y = node.height;
            
            if (v.y < 10.0f || math::probability(1.0f - node.vegetation))
            {
                continue;
            }
            
            radius *= 1.0f + node.vegetation;
            
            float
                left_edge = .25f * (int)math::rnd(4),
                right_edge = left_edge + .25f;
            
            float
                a = math::rnd(math::HALF_PI),
                w = cos(a) * radius,
                h = sin(a) * radius;
            
            // 0
            *(p++) = v.x - w; *(p++) = v.y; *(p++) = v.z - h;
            *(t++) = left_edge; *(t++) = 0.0f;
            
            // 1
            *(p++) = v.x + w; *(p++) = v.y; *(p++) = v.z + h;
            *(t++) = right_edge; *(t++) = 0.0f;
            
            // 2
            *(p++) = v.x + w; *(p++) = v.y + height; *(p++) = v.z + h;
            *(t++) = right_edge; *(t++) = 1.0f;
            
            // 3
            *(p++) = v.x - w; *(p++) = v.y + height; *(p++) = v.z - h;
            *(t++) = left_edge; *(t++) = 1.0f;
            
            a += math::HALF_PI;
            w = cos(a) * radius;
            h = sin(a) * radius;

            // 4
            *(p++) = v.x - w; *(p++) = v.y; *(p++) = v.z - h;
            *(t++) = right_edge; *(t++) = 0.0f;

            // 5
            *(p++) = v.x + w; *(p++) = v.y; *(p++) = v.z + h;
            *(t++) = left_edge; *(t++) = 0.0f;
            
            // 6
            *(p++) = v.x + w; *(p++) = v.y + height; *(p++) = v.z + h;
            *(t++) = left_edge; *(t++) = 1.0f;
            
            // 7
            *(p++) = v.x - w; *(p++) = v.y + height; *(p++) = v.z - h;
            *(t++) = right_edge; *(t++) = 1.0f;
            
            int base = total * 8;
            *(i++) = base + 0; *(i++) = base + 1; *(i++) = base + 2;
            *(i++) = base + 0; *(i++) = base + 2; *(i++) = base + 3;
            *(i++) = base + 4; *(i++) = base + 5; *(i++) = base + 6;
            *(i++) = base + 4; *(i++) = base + 6; *(i++) = base + 7;
            
            total++;
        }

        if (total)
        {
            gfx::Mesh *mesh = new gfx::Mesh();
            
            const char *texture = (math::probability(.5f))
                ? "video/textures/scenery/trees/deciduous.png"
                : "video/textures/scenery/trees/coniferous.png";

            mesh->material            = gfx::Material::add(texture);
            mesh->material->shader    = gfx::Program::get("forest", "forest");
            mesh->material->color_map = gfx::Texture::get(texture);

            glGenBuffers(1, &mesh->attr.index);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->attr.index);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * total * sizeof(GLuint),
                index, GL_STATIC_DRAW);

            glGenBuffers(1, &mesh->attr.pos);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->attr.pos);
            glBufferData(GL_ARRAY_BUFFER, 24 * total * sizeof(GLfloat),
                pos, GL_STATIC_DRAW);

            glGenBuffers(1, &mesh->attr.uv);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->attr.uv);
            glBufferData(GL_ARRAY_BUFFER, 16 * total * sizeof(GLfloat),
                uv, GL_STATIC_DRAW);
            
            this->trees[lod]      = mesh;
            this->tree_count[lod] = total * 4;
        }
        
        delete[] index;
        delete[] pos;
        delete[] uv;
    }
}


void
TerrainChunk::render(float dist, float lod)
const
{
    // "Grow" hills, mountains and props in place
    math::Mat4
        y_scale
            = math::Mat4::scaling(
                1.0f,
                math::transition::ease_out(0.0f, 1.0f, lod * 2.0f),
                1.0f),
        mv  = screen.scene->matrix.mv
            = y_scale * screen.scene->matrix.camera;

    float
        vegetation_lod = math::clamp(lod * .9f + .11f, 0.0f, 1.0f)
            * TerrainChunk::VEGETATION_LOD,

        fog = math::clamp(
            pow(math::E, screen.scene->fog.w * dist) - 1.0f,
            0.0f, 1.0f);
    
    lod = math::clamp(lod * 3.2f - 2.0f, 0.0f, 1.0f);
    
    glEnable(GL_CULL_FACE);
    this->mesh->render();

    if (this->trees[0] != NULL)
    {
        glDisable(GL_CULL_FACE);
        gfx::Program *shader = this->trees[0]->material->shader;
        glUseProgram(shader->id);

        screen.scene->matrix.projection.to(shader->unif.projection);
        
        math::Vec4(
            screen.scene->fog.x,
            screen.scene->fog.y,
            screen.scene->fog.z,
            fog).to(shader->unif.ambient_color);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(shader->unif.color_map, 0);
        this->trees[0]->material->color_map->bind();

        glEnableVertexAttribArray(shader->attr.v);
        glEnableVertexAttribArray(shader->attr.t);

        for (int t = 0; t < vegetation_lod; ++t)
        {
            if (this->trees[t] == NULL)
            {
                continue;
            }
            
            // Grow tree LOD levels gradually in view
            (y_scale * math::Mat4::translation(
                0.0f,
                math::min(
                    0.0f,
                    20 * (vegetation_lod - t) - 20),
                0.0f)
            * screen.scene->matrix.camera)
            .to(shader->unif.modelview);

            (y_scale * math::Mat4::translation(
                0.0f,
                math::transition::smooth(-30.0f, 0.0f,
                    (vegetation_lod - t) / (.3f * TerrainChunk::VEGETATION_LOD)),
                0.0f)
            * screen.scene->matrix.camera)
            .to(shader->unif.modelview);

            // Render
            glBindBuffer(GL_ARRAY_BUFFER, this->trees[t]->attr.pos);
            glVertexAttribPointer(shader->attr.v, 3, GL_FLOAT, GL_FALSE, 0, NULL);

            glBindBuffer(GL_ARRAY_BUFFER, this->trees[t]->attr.uv);
            glVertexAttribPointer(shader->attr.t, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->trees[t]->attr.index);
            glDrawElements(GL_TRIANGLES, this->tree_count[t], GL_UNSIGNED_INT, NULL);
        }
            
        // Unbind
        glDisableVertexAttribArray(shader->attr.v);
        glDisableVertexAttribArray(shader->attr.t);
        glUseProgram(0);
    }
    
    const float
        LOW_TRESHOLD = .1f;

    if (lod > LOW_TRESHOLD)
    {
        glEnable(GL_CULL_FACE);
        lod = (lod - LOW_TRESHOLD) / (1.0f - LOW_TRESHOLD);

        int model = (lod == 1.0f)
            ? TerrainChunk::LOD_LEVELS - 1
            : lod * (TerrainChunk::LOD_LEVELS - 2);

        for (Props::const_iterator prop = this->props.begin();
            prop != this->props.end(); ++prop)
        {
            screen.scene->matrix.mv = (*prop)->transformation * mv;
            
            (*prop)->model[model]->render();
        }
    }
    else
    {
        glDisable(GL_CULL_FACE);
        glUseProgram(_billboard.shader->id);
        screen.scene->matrix.projection.to(_billboard.shader->unif.projection);

        math::Vec3(.8f, .79f, .77f)
            .mix(math::Vec3(screen.scene->fog), fog)
            .to(_billboard.shader->unif.ambient_color);

        glBindBuffer(GL_ARRAY_BUFFER, _billboard.pos);
        glEnableVertexAttribArray(_billboard.shader->attr.v);
        glVertexAttribPointer(_billboard.shader->attr.v, 2, GL_FLOAT, GL_FALSE, 0, NULL);

        math::Mat4 rot = math::Mat4::rotationY(math::HALF_PI
            + screen.scene->camera.physics->yaw());

        for (Props::const_iterator prop = this->props.begin();
            prop != this->props.end(); ++prop)
        {
            math::Mat4 billboard = rot * (*prop)->scaled * mv;

            billboard.to(_billboard.shader->unif.modelview);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glUseProgram(0);
        glDisableVertexAttribArray(_billboard.shader->attr.v);
    }
}
