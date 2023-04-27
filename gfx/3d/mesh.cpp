#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "mesh.h"
#include "../../core/engine.h" // screen
#include "../../core/math.h"   // randomness

#include <cmath>     // abs, sqrt

using namespace gfx;

Mesh::Mesh()
{
    this->attr.index     = 0;
    this->attr.pos       = 0;
    this->attr.normal    = 0;
    this->attr.uv        = 0;
    this->attr.uv_weight = 0;
    
    this->name           = NULL;
    this->material       = NULL;
}

Mesh::~Mesh()
{
    delete[] this->name;
    
    glDeleteBuffers(1, &this->attr.index);
    glDeleteBuffers(1, &this->attr.pos);
    glDeleteBuffers(1, &this->attr.normal);
    glDeleteBuffers(1, &this->attr.uv);
    glDeleteBuffers(1, &this->attr.uv_weight);
}

void
Mesh::compose(void)
{
    if (this->attr.index)
    {
        glDeleteBuffers(1, &this->attr.index);
        glDeleteBuffers(1, &this->attr.pos);
        glDeleteBuffers(1, &this->attr.normal);
        glDeleteBuffers(1, &this->attr.uv);
        glDeleteBuffers(1, &this->attr.uv_weight);
    }
    
    glGenBuffers(1, &this->attr.index);
    glGenBuffers(1, &this->attr.pos);
    glGenBuffers(1, &this->attr.normal);
    glGenBuffers(1, &this->attr.uv);
    glGenBuffers(1, &this->attr.uv_weight);

    GLuint  *i_index     = (this->indices.empty())
        ? NULL
        : new GLuint[this->indices.size()];
    
    GLfloat *f_pos       = new GLfloat[3 * this->vertices.size()];
    GLfloat *f_normal    = new GLfloat[3 * this->vertices.size()];
    GLfloat *f_uv        = new GLfloat[2 * this->vertices.size()];
    GLfloat *f_uv_weight = new GLfloat[4 * this->vertices.size()];
    
    unsigned int i1, i2 = 0, i3 = 0, i4 = 0;
    
    this->faces.clear();

    if (i_index != NULL)
    {
        // Build and store vertex indices
        for (i1 = 0; i1 < this->indices.size(); ++i1)
        {
            i_index[i1] = this->indices[i1];
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->attr.index);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, i1 * sizeof(GLuint), i_index, GL_STATIC_DRAW);
        delete[] i_index;
    }

    // Convert vertex attributes to OpenGL-digested arrays
    for (Mesh::Vertices::const_iterator v = this->vertices.begin();
        v != this->vertices.end(); ++v)
    {
        f_pos[i3] = v->pos.x; f_normal[i3] = v->normal.x; i3++;
        f_pos[i3] = v->pos.y; f_normal[i3] = v->normal.y; i3++;
        f_pos[i3] = v->pos.z; f_normal[i3] = v->normal.z; i3++;
        
        f_uv[i2++] = v->uv.x;
        f_uv[i2++] = v->uv.y;

        f_uv_weight[i4++] = v->uv_weight.x;
        f_uv_weight[i4++] = v->uv_weight.y;
        f_uv_weight[i4++] = v->uv_weight.z;
        f_uv_weight[i4++] = v->uv_weight.w;
    }

    // Store vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, this->attr.pos);
    glBufferData(GL_ARRAY_BUFFER, i3 * sizeof(GLfloat), f_pos, GL_STATIC_DRAW);
    delete[] f_pos;
    
    // Store normals
    glBindBuffer(GL_ARRAY_BUFFER, this->attr.normal);
    glBufferData(GL_ARRAY_BUFFER, i3 * sizeof(GLfloat), f_normal, GL_STATIC_DRAW);
    delete[] f_normal;

    // Store texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, this->attr.uv);
    glBufferData(GL_ARRAY_BUFFER, i2 * sizeof(GLfloat), f_uv, GL_STATIC_DRAW);
    delete[] f_uv;

    // Store texture weights
    glBindBuffer(GL_ARRAY_BUFFER, this->attr.uv_weight);
    glBufferData(GL_ARRAY_BUFFER, i4 * sizeof(GLfloat), f_uv_weight, GL_STATIC_DRAW);
    delete[] f_uv_weight;
}

void
Mesh::render(void)
const
{
    Material *material = this->material;

    material->use();

    Program *shader = material->shader;

    // Bind
    glEnableVertexAttribArray(shader->attr.v);
    glBindBuffer(GL_ARRAY_BUFFER, this->attr.pos);
    glVertexAttribPointer(shader->attr.v, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(shader->attr.n);
    glBindBuffer(GL_ARRAY_BUFFER, this->attr.normal);
    glVertexAttribPointer(shader->attr.n, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(shader->attr.t);
    glBindBuffer(GL_ARRAY_BUFFER, this->attr.uv);
    glVertexAttribPointer(shader->attr.t, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glEnableVertexAttribArray(shader->attr.t_weight);
    glBindBuffer(GL_ARRAY_BUFFER, this->attr.uv_weight);
    glVertexAttribPointer(shader->attr.t_weight, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    
    screen.scene->matrix.mv.to(shader->unif.modelview);
    screen.scene->matrix.mv.transpose().inverse().to(shader->unif.normal);
    
    // Render
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->attr.index);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, NULL);

    // Unbind
    glDisableVertexAttribArray(shader->attr.v);
    glDisableVertexAttribArray(shader->attr.n);
    glDisableVertexAttribArray(shader->attr.t);
    glDisableVertexAttribArray(shader->attr.t_weight);
    glUseProgram(0);
}

Mesh &
Mesh::add(int index_0, int index_1, int index_2)
{
    this->indices.push_back(index_0);
    this->indices.push_back(index_1);
    this->indices.push_back(index_2);
    
    return *this;
}

Mesh &
Mesh::add(const Vertex &v)
{
    this->vertices.push_back(v);
    
    return *this;
}

Mesh &
Mesh::add(const math::Vec3 &pos, const math::Vec3 &normal, const math::Vec2 &uv)
{
    Vertex v;
    v.pos       = pos;
    v.normal    = normal;
    v.uv        = uv;
    v.uv_weight = math::Vec4(1.0, 0.0, 0.0, 0.0);

    this->vertices.push_back(v);
    
    return *this;
}

Mesh &
Mesh::add(const math::Vec3 &pos)
{
    this->add(pos, math::Vec3(0.0f, 1.0f, 0.0f), math::Vec2(pos));
    
    return *this;
}

Mesh &
Mesh::add(const math::Vec3 &pos, const math::Vec2 &uv)
{
    this->add(pos, math::Vec3(0.0f, 1.0f, 0.0f), uv);
    
    return *this;
}

Mesh &
Mesh::add(const math::Vec3 &pos, const math::Vec3 &normal)
{
    this->add(pos, normal, math::Vec2(pos));
    
    return *this;
}

Mesh &
Mesh::add(const Mesh &mesh)
{
    size_t offset = this->vertices.size();
    this->vertices.reserve(offset + mesh.vertices.size());
    this->indices.reserve(this->indices.size() + mesh.vertices.size());
    for (Mesh::Vertices::const_iterator v = mesh.vertices.begin();
        v != mesh.vertices.end(); ++v)
    {
        this->vertices.push_back(*v);
    }

    for (Mesh::Indices::const_iterator i = mesh.indices.begin();
        i != mesh.indices.end(); ++i)
    {
        this->indices.push_back(*i + offset);
    }
    
    return *this;
}

Mesh &
Mesh::add(const Mesh &mesh, const math::Mat4 &transformation)
{
    size_t offset = this->vertices.size();
    this->vertices.reserve(offset + mesh.vertices.size());
    this->indices.reserve(this->indices.size() + mesh.vertices.size());
    for (Mesh::Vertices::const_iterator v = mesh.vertices.begin();
        v != mesh.vertices.end(); ++v)
    {
        this->vertices.push_back(*v);
        this->vertices[this->vertices.size() - 1].pos *= transformation;
    }

    for (Mesh::Indices::const_iterator i = mesh.indices.begin();
        i != mesh.indices.end(); ++i)
    {
        this->indices.push_back(*i + offset);
    }
    
    return *this;
}

Mesh &
Mesh::add(const Mesh &mesh, const math::Vec3 &pos, float scale)
{
    this->add(mesh, math::Mat4::translation(pos).scale(scale));
    
    return *this;
}

Mesh &
Mesh::add(const Mesh &mesh, float x, float y, float z, float scale)
{
    this->add(mesh, math::Mat4::translation(x, y, z).scale(scale));
    
    return *this;
}


Mesh &
Mesh::add(Mesh *mesh)
{
    size_t offset = this->vertices.size();
    this->vertices.reserve(offset + mesh->vertices.size());
    this->indices.reserve(this->indices.size() + mesh->vertices.size());
    for (Mesh::Vertices::const_iterator v = mesh->vertices.begin();
        v != mesh->vertices.end(); ++v)
    {
        this->vertices.push_back(*v);
    }
    
    for (Mesh::Indices::const_iterator i = mesh->indices.begin();
        i != mesh->indices.end(); ++i)
    {
        this->indices.push_back(*i + offset);
    }

    delete mesh;
    return *this;
}

Mesh &
Mesh::add(Mesh *mesh, const math::Mat4 &transformation)
{
    size_t offset = this->vertices.size();
    this->vertices.reserve(offset + mesh->vertices.size());
    this->indices.reserve(this->indices.size() + mesh->vertices.size());
    for (Mesh::Vertices::const_iterator v = mesh->vertices.begin();
        v != mesh->vertices.end(); ++v)
    {
        this->vertices.push_back(*v);
        this->vertices[this->vertices.size() - 1].pos *= transformation;
    }
    
    for (Mesh::Indices::const_iterator i = mesh->indices.begin();
        i != mesh->indices.end(); ++i)
    {
        this->indices.push_back(*i + offset);
    }
    
    delete mesh;
    return *this;
}

Mesh &
Mesh::add(Mesh *mesh, const math::Vec3 &pos, float scale)
{
    this->add(mesh, math::Mat4::translation(pos).scale(scale));
    
    return *this;
}

Mesh &
Mesh::add(Mesh *mesh, float x, float y, float z, float scale)
{
    this->add(mesh, math::Mat4::translation(x, y, z).scale(scale));
    
    return *this;
}

Mesh &
Mesh::transform(const math::Mat4 &transformation, bool transform_uv)
{
    if (transform_uv)
    {
        math::Mat4 inverse = transformation.inverse();
        for (Mesh::Vertices::iterator v = this->vertices.begin();
            v != this->vertices.end(); ++v)
        {
            v->pos *= transformation;
            math::Vec3 foo(1.0f); foo *= inverse;
            v->uv.x *= foo.x;
            v->uv.y *= foo.z;
        }
    }
    else
    {
        for (Mesh::Vertices::iterator v = this->vertices.begin();
            v != this->vertices.end(); ++v)
        {
            v->pos *= transformation;
        }
    }
    
    return *this;
}

Mesh &
Mesh::translate(const math::Vec3 &v, bool transform_uv)
{
    return this->transform(
        math::Mat4::translation(v),
        transform_uv);
}

Mesh &
Mesh::translate(float x, float y, float z, bool transform_uv)
{
    return this->transform(
        math::Mat4::translation(x, y, z),
        transform_uv);
}

Mesh &
Mesh::scale(const math::Vec3 &v, bool transform_uv)
{
    return this->transform(
        math::Mat4::identity().scale(v),
        transform_uv);
}

Mesh &
Mesh::scale(float x, float y, float z, bool transform_uv)
{
    return this->transform(
        math::Mat4::identity().scale(x, y, z),
        transform_uv);
}

Mesh &
Mesh::scale(float s, bool transform_uv)
{
    return this->transform(
        math::Mat4::identity().scale(s),
        transform_uv);
}

Mesh &
Mesh::rotX(float angle, bool transform_uv)
{
    return this->transform(
        math::Mat4::identity().rotX(angle),
        transform_uv);
}

Mesh &
Mesh::rotY(float angle, bool transform_uv)
{
    return this->transform(
        math::Mat4::identity().rotY(angle),
        transform_uv);
}

Mesh &
Mesh::rotZ(float angle, bool transform_uv)
{
    return this->transform(
        math::Mat4::identity().rotZ(angle),
        transform_uv);
}

Mesh &
Mesh::scale_uv(float x, float y)
{
   for (Mesh::Vertices::iterator v = this->vertices.begin();
        v != this->vertices.end(); ++v)
    {
        v->uv.x *= x;
        v->uv.y *= y;
    }
 
    return *this;
}

Mesh &
Mesh::translate_uv(float x, float y)
{
   for (Mesh::Vertices::iterator v = this->vertices.begin();
        v != this->vertices.end(); ++v)
    {
        v->uv.x += x;
        v->uv.y += y;
    }
 
    return *this;
}

Mesh &
Mesh::transform_uv(const math::Mat4 &transformation)
{
   for (Mesh::Vertices::iterator v = this->vertices.begin();
        v != this->vertices.end(); ++v)
    {
        v->uv *= transformation;
    }
 
    return *this;
}

float
Mesh::radius(void)
const
{
    float result = 0.0f;
    
    for (Mesh::Vertices::const_iterator v = this->vertices.begin();
        v != this->vertices.end(); ++v)
    {
        result = math::max(result, v->pos.length_sq());
    }
    
    return sqrt(result);
}

math::Vec3
Mesh::size(void)
const
{
    math::Vec3 pos, neg;
    this->get_bounds(&pos, &neg);
    
    return math::Vec3(
        abs(pos.x - neg.x),
        abs(pos.y - neg.y),
        abs(pos.z - neg.z)
    );
}

math::Vec3
Mesh::center(void)
const
{
    math::Vec3 center = math::Vec3(0.0f);
    
    for (Mesh::Vertices::const_iterator v = this->vertices.begin();
        v != this->vertices.end(); ++v)
    {
        center += v->pos;
    }
    
    return center / (float)this->vertices.size();
}

void
Mesh::get_bounds(math::Vec3 *negative, math::Vec3 *positive)
const
{
    *negative = math::Vec3(+99999.0f, +99999.0f, +99999.0f);
    *positive = math::Vec3(-99999.0f, -99999.0f, -99999.0f);
    
    for (Mesh::Vertices::const_iterator v = this->vertices.begin();
        v != this->vertices.end(); ++v)
    {
        *negative = negative->min(v->pos);
        *positive = positive->max(v->pos);
    }
}

Mesh &
Mesh::build_normals(void)
{
    for (unsigned int i = 0; i < this->indices.size(); i += 3)
    {
        Mesh::Vertex
            *v0 = &this->vertices[this->indices[i]],
            *v1 = &this->vertices[this->indices[i + 1]],
            *v2 = &this->vertices[this->indices[i + 2]];
        
        math::Vec3
            u = v1->pos - v0->pos,
            v = v2->pos - v0->pos;
        
        v0->normal = v1->normal = v2->normal = u.cross(v).normalize();
    }

    return *this;
}

#include <map>
struct Vec3Comparator
{
    bool operator()(const math::Vec3 &a, const math::Vec3 &b)
    const
    {
        if (a.x == b.x)
        {
            return (a.y == b.y)
                ? a.z < b.z
                : a.y < b.y;
        }

        return a.x < b.x;
    }
};

Mesh &
Mesh::roughen(float max_deviation)
{
    typedef
        std::map<math::Vec3, bool, Vec3Comparator>
        Changes;
    
    Changes changes;
    
    for (Mesh::Vertices::iterator v = this->vertices.begin();
        v != this->vertices.end(); ++v)
    {
        changes[v->pos] = true;
    }
    
    for (Changes::iterator c = changes.begin(); c != changes.end(); ++c)
    {
        math::Vec3 translation(
            math::vary(max_deviation),
            math::vary(max_deviation),
            math::vary(max_deviation)
        );

        for (Mesh::Vertices::iterator v = this->vertices.begin();
            v != this->vertices.end(); ++v)
        {
            if (v->pos == c->first)
            {
                v->pos += translation;
            }
        }
    }
    
    return *this;
}

static void
_avg(const Mesh::Vertex &v0, Mesh::Vertex *out, const Mesh::Vertex &v1)
{
    out->pos       = (v0.pos + v1.pos) / 2.0f;
    out->normal    = (v0.normal + v1.normal) / 2.0f;
    out->uv        = (v0.uv + v1.uv) / 2.0f;
    out->uv_weight = (v0.uv_weight + v1.uv_weight) / 2.0f;
}

static void
_weigh(const Mesh::Vertex &v0, Mesh::Vertex *out, const Mesh::Vertex &v1, float f1)
{
    float f0 = 1.0f - f1;
    
    out->pos       = v0.pos * f0 + v1.pos * f1;
    out->normal    = v0.normal * f0 + v1.normal * f1;
    out->uv        = v0.uv * f0 + v1.uv * f1;
    out->uv_weight = v0.uv_weight * f0 + v1.uv_weight * f1;
}

Mesh &
Mesh::clip(void)
{
    Mesh *result = new Mesh();
    int base = 0;
    
    for (unsigned int i = 0; i < this->indices.size(); i += 3)
    {
        Mesh::Vertex
            a, b,
            &v0 = this->vertices[this->indices[i]],
            &v1 = this->vertices[this->indices[i + 1]],
            &v2 = this->vertices[this->indices[i + 2]];

        
        switch ((v0.pos.y >= 0.0f) + (v1.pos.y >= 0.0f) + (v2.pos.y >= 0.0f))
        {
            // All ertices above clipping plane => no changes
            case 3:
                result->add(v0).add(v1).add(v2);
                break;

            // One vertex below clipping plane => split in two
            case 2:
                if (v0.pos.y < 0.0f)
                {
                    _weigh(v0, &a, v1, v0.pos.y / (v0.pos.y - v1.pos.y));
                    _weigh(v0, &b, v2, v0.pos.y / (v0.pos.y - v2.pos.y));
                    
                    (*result)
                    .add(a).add(v1).add(v2)
                    .add(b).add(a).add(v2);
                }
                else if (v1.pos.y < 0.0f)
                {
                    _weigh(v1, &a, v0, v1.pos.y / (v1.pos.y - v0.pos.y));
                    _weigh(v1, &b, v2, v1.pos.y / (v1.pos.y - v2.pos.y));
                    
                    (*result)
                    .add(b).add(v2).add(v0)
                    .add(a).add(b).add(v0);
                }
                else
                {
                    _weigh(v2, &a, v0, v2.pos.y / (v2.pos.y - v0.pos.y));
                    _weigh(v2, &b, v1, v2.pos.y / (v2.pos.y - v1.pos.y));
                    
                    (*result)
                    .add(a).add(v0).add(v1)
                    .add(b).add(a).add(v1);
                }
                result->indices.push_back(base++);
                result->indices.push_back(base++);
                result->indices.push_back(base++);
                break;

            // Only one vertex above clipping plane => cull lower portion
            case 1:
                if (v0.pos.y >= 0.0f)
                {
                    _weigh(v0, &a, v1, v0.pos.y / (v0.pos.y - v1.pos.y));
                    _weigh(v0, &b, v2, v0.pos.y / (v0.pos.y - v2.pos.y));
                    result->add(v0).add(a).add(b);
                }
                else if (v1.pos.y >= 0.0f)
                {
                    _weigh(v1, &a, v0, v1.pos.y / (v1.pos.y - v0.pos.y));
                    _weigh(v1, &b, v2, v1.pos.y / (v1.pos.y - v2.pos.y));
                    result->add(a).add(v1).add(b);
                }
                else
                {
                    _weigh(v2, &a, v0, v2.pos.y / (v2.pos.y - v0.pos.y));
                    _weigh(v2, &b, v1, v2.pos.y / (v2.pos.y - v1.pos.y));
                    result->add(a).add(b).add(v2);
                }
                break;
            
            default:
                continue;
        }
        result->indices.push_back(base++);
        result->indices.push_back(base++);
        result->indices.push_back(base++);
    }
    
    
    this->vertices = result->vertices;
    this->indices  = result->indices;
    delete result;
    
    return *this;
}

Mesh &
Mesh::clip(const math::Vec3 &point_on_plane, const math::Vec3 &plane_normal)
{
    float
        x = atan2(1.0f, plane_normal.x),
        z = atan2(1.0f, plane_normal.z);

    math::Mat4 M = math::Mat4::identity()
        .translate(point_on_plane)
        .rotX(x)
        .rotZ(z);
    
    (*this)
        .transform(M.inverse())
        .clip()
        .transform(M);
    
    return *this;
}

Mesh &
Mesh::subdivide(int iterations)
{
    while (iterations-- > 0)
    {
        Mesh *result = new Mesh();
        
        int base = 0;
        
        for (unsigned int i = 0; i < this->indices.size(); i += 3)
        {
            Mesh::Vertex
                &v0 = this->vertices[this->indices[i]],
                v1,
                &v2 = this->vertices[this->indices[i + 1]],
                v3,
                &v4 = this->vertices[this->indices[i + 2]],
                v5;
            
            _avg(v0, &v1, v2);
            _avg(v2, &v3, v4);
            _avg(v4, &v5, v0);
            
            (*result)
            .add(v0.pos, v0.normal, v0.uv)
            .add(v1.pos, v1.normal, v1.uv)
            .add(v2.pos, v2.normal, v2.uv)
            .add(v3.pos, v3.normal, v3.uv)
            .add(v4.pos, v4.normal, v4.uv)
            .add(v5.pos, v5.normal, v5.uv)
            
            .add(base,     base + 1, base + 5)
            .add(base + 1, base + 2, base + 3)
            .add(base + 1, base + 3, base + 5)
            .add(base + 5, base + 3, base + 4);
            
            base += 6;
        }
        
        this->vertices = result->vertices;
        this->indices  = result->indices;
        delete result;
    }
    
    return *this;
}

Mesh &
Mesh::compress(void)
{
    Mesh *result = new Mesh();
    
    for (Mesh::Indices::const_iterator i = this->indices.begin();
        i != this->indices.end(); ++i)
    {
        Mesh::Vertex &v = this->vertices[*i];

        int n = 0;
        for (Mesh::Vertices::const_iterator r = result->vertices.begin();
            r != result->vertices.end(); ++r, ++n)
        {
            if (v.pos == r->pos && v.normal == r->normal
                && v.uv == r->uv && v.uv_weight == r->uv_weight)
            {
                result->indices.push_back(n);
                goto compress_next;
            }
        }
        
        result->indices.push_back(result->vertices.size());
        result->add(v);

compress_next:;
    }
    
    this->vertices = result->vertices;
    this->indices  = result->indices;
    delete result;

    return *this;
}

Mesh &
Mesh::flip(bool normals, bool faces)
{
    if (normals)
    {
        for (Mesh::Vertices::iterator v = this->vertices.begin();
            v != this->vertices.end(); ++v)
        {
            v->normal *= -1.0f;
        }
    }

    if (faces)
    {
        for (unsigned int i = 0; i < this->indices.size(); i += 3)
        {
            int temp = this->indices[i + 1];
            this->indices[i + 1] = this->indices[i];
            this->indices[i]     = temp;
        }
    }
    
    return *this;
}

bool
Mesh::intersects(const math::Vec3 &relative_origin, const math::Vec3 &direction)
const
{
    for (Mesh::Faces::const_iterator f = this->faces.begin();
        f != this->faces.end(); ++f)
    {
        if (f->triangle.intersects(relative_origin, direction))
        {
            return true;
        }
    }
    
    return false;
}
