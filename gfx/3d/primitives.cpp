#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "primitives.h"
#include "../../core/math.h"

#include <cmath> // trigonometry

using namespace gfx;

Mesh *
primitive::quad()
{
    Mesh *mesh = new Mesh();
    mesh->vertices.reserve(4);

    math::Vec3 n = math::Vec3::up();

    (*mesh)
    .add(math::Vec3(-1.0f, +1.0f, 0.0f), n, math::Vec2(0.0f, 1.0f))
    .add(math::Vec3(+1.0f, +1.0f, 0.0f), n, math::Vec2(1.0f, 1.0f))
    .add(math::Vec3(-1.0f, -1.0f, 0.0f), n, math::Vec2(0.0f, 0.0f))
    .add(math::Vec3(+1.0f, -1.0f, 0.0f), n, math::Vec2(1.0f, 0.0f));

    return mesh;
}

Mesh *
primitive::triangle()
{
    Mesh *mesh = new Mesh();
    mesh->vertices.reserve(3);
    mesh->indices.reserve(3);
    
    math::Vec3 n = math::Vec3::up();
    (*mesh)
    .add(math::Vec3( 0.0f, 0.0f, -1.0f), n, math::Vec2(0.5f, 1.0f)) // 0
    .add(math::Vec3(-1.0f, 0.0f, +1.0f), n, math::Vec2(0.0f, 0.0f)) // 1
    .add(math::Vec3(+1.0f, 0.0f, +1.0f), n, math::Vec2(1.0f, 0.0f)) // 2

    .add(0, 1, 2);
    
    return mesh;
}

Mesh *
primitive::plane()
{
    Mesh *mesh = new Mesh();
    mesh->vertices.reserve(4);
    mesh->indices.reserve(6);
    
    math::Vec3 n = math::Vec3::up();

    (*mesh)
    .add(math::Vec3(-1.0f, 0.0f, -1.0f), n, math::Vec2(0.0f, 1.0f)) // 0
    .add(math::Vec3(-1.0f, 0.0f, +1.0f), n, math::Vec2(0.0f, 0.0f)) // 1
    .add(math::Vec3(+1.0f, 0.0f, +1.0f), n, math::Vec2(1.0f, 0.0f)) // 2
    .add(math::Vec3(+1.0f, 0.0f, -1.0f), n, math::Vec2(1.0f, 1.0f)) // 3

    .add(0, 1, 3)
    .add(1, 2, 3);
    return mesh;
}

Mesh *
primitive::circle(int corners)
{
    float
        a    = 0.0f,
        step = math::DOUBLE_PI / (float)corners;
    
    Mesh *mesh = new Mesh();
    mesh->vertices.reserve(corners);
    mesh->indices.reserve(3 * (corners - 2));

    math::Vec3 n = math::Vec3::up();
    for (int i = 0; i < corners; ++i, a += step)
    {
        math::Vec2 v = math::Vec2(cos(a), sin(a));
        
        mesh->add(
            math::Vec3(v.x, 0.0f, v.y),
            n,
            math::Vec2(v.x / 2.0f + 0.5f, v.y / 2.0f + 0.5f));
        
        if (i > 1)
        {
            mesh->add(0, i, i - 1);
        }
    }
    
    return mesh;
}

Mesh *
primitive::ring(int corners, float thickness)
{
    float
        a    = 0.0f,
        step = math::DOUBLE_PI / (float)corners;
    
    Mesh *mesh = new Mesh();
    
    mesh->vertices.reserve(4 * corners);
    mesh->indices.reserve(6 * corners);

    corners  *= 2;
    thickness = 1.0f - thickness;
    
    math::Vec3 n = math::Vec3::up();
    for (int i = 0; i <= corners; i += 2, a += step)
    {
        math::Vec2 v = math::Vec2(cos(a), sin(a));
        
        // Outer vertices
        mesh->add(
            math::Vec3(v.x, 0.0f, v.y),
            n,
            math::Vec2(v.x / 2.0f + 0.5f, v.y / 2.0f + 0.5f));

        // Inner vertices
        v *= thickness;
        mesh->add(
            math::Vec3(v.x, 0.0f, v.y),
            n,
            math::Vec2(v.x / 2.0f + 0.5f, v.y / 2.0f + 0.5f));

        if (i > 1)
        {
            (*mesh)
            .add(i, i - 2, i - 1)
            .add(i, i - 1, i + 1);
        }
    }

    // Glue last vertices to the beginning to prevent gaps
    mesh->vertices[corners].pos     = mesh->vertices[0].pos;
    mesh->vertices[corners + 1].pos = mesh->vertices[1].pos;
    
    return mesh;
}

Mesh *
primitive::cone(int corners, bool hollow)
{
    float
        a      = 0.0f,
        t      = 0.0f,
        a_step = math::DOUBLE_PI / (float)corners,
        t_step = 1.0f / (float)corners;
    
    Mesh *mesh = new Mesh();
    
    mesh->vertices.reserve(3 * corners);
    mesh->indices.reserve(3 * corners);

    math::Vec2 v = math::Vec2(cos(a), sin(a));
    for (int i = 0; i / 3 < corners; i += 3)
    {
        math::Vec3 n = math::Vec3(
            cos(a + a_step / 2.0),
            0.0f,
            sin(a + a_step / 2.0));

        // Current edge
        mesh->add(math::Vec3(v.x, -1.0f, v.y), n, math::Vec2(t, 0.0f));

        // Peak
        mesh->add(math::Vec3(0.0f, +1.0f, 0.0f), n,
            math::Vec2(t + t_step / 2.0f, 1.0f));

        a += a_step;
        t += t_step;

        // Next edge
        v = math::Vec2(cos(a), sin(a));
        mesh->add(math::Vec3(v.x, -1.0f, v.y), n, math::Vec2(t, 0.0f));

        mesh->add(i, i + 1, i + 2);
    }
    
    // Glue last vertices to the beginning to prevent gaps
    mesh->vertices[mesh->vertices.size() - 1].pos = mesh->vertices[0].pos;
    
    if (!hollow)
    {
        mesh->add(&primitive::circle(corners)->translate(0.0f, -1.0f, 0.0f)
        .flip());
    }
    
    return mesh;
}

Mesh *
primitive::cylinder(int corners, bool hollow)
{
    return primitive::tapered_cylinder(corners, 1.0f, hollow);
}

Mesh *
primitive::tapered_cylinder(int corners, float taper, bool hollow)
{
    float
        a      = 0.0f,
        t      = 0.0f,
        a_step = math::DOUBLE_PI / (float)corners,
        t_step = 1.0f / (float)corners;
    
    Mesh *mesh = new Mesh();
    
    mesh->indices.reserve(4 * corners);
    mesh->vertices.reserve(6 * corners);

    corners *= 4;
    
    math::Vec2 v = math::Vec2(cos(a), sin(a));
    for (int i = 0; i < corners; i += 4)
    {
        math::Vec3 n = math::Vec3(
            cos(a + a_step / 2.0),
            0.0f,
            sin(a + a_step / 2.0));

        // Current edge
        mesh->add(math::Vec3(v.x, -1.0f, v.y), n, math::Vec2(t, 0.0f));
        mesh->add(math::Vec3(v.x * taper, +1.0f, v.y * taper), n,
            math::Vec2(t, 1.0f));

        a += a_step;
        t += t_step;

        // Next edge
        v = math::Vec2(cos(a), sin(a));
        mesh->add(math::Vec3(v.x, -1.0f, v.y), n, math::Vec2(t, 0.0f));
        mesh->add(math::Vec3(v.x * taper, +1.0f, v.y * taper), n,
            math::Vec2(t, 1.0f));

        mesh->add(i, i + 1, i + 3);
        mesh->add(i, i + 3, i + 2);
    }
    
    // Glue last vertices to the beginning to prevent gaps
    mesh->vertices[corners - 2].pos = mesh->vertices[0].pos;
    mesh->vertices[corners - 1].pos = mesh->vertices[1].pos;
    
    if (!hollow)
    {
        corners /= 4;
        mesh->add(&primitive::circle(corners)->scale(taper).translate(0.0f, 1.0f, 0.0f));
        mesh->add(&primitive::circle(corners)->translate(0.0f, -1.0f, 0.0f)
        .flip());
    }

    return mesh;
}

Mesh *
primitive::tube(int corners, float thickness)
{
    Mesh *mesh = primitive::cylinder(corners, true);
    mesh->flip().scale(1.0f - thickness, 1.0f, 1.0f - thickness);
    mesh->add(primitive::cylinder(corners, true));

    mesh->add(&primitive::ring(corners, thickness)
        ->translate(0.0f, +1.0f, 0.0f));

    mesh->add(&primitive::ring(corners, thickness)
        ->translate(0.0f, -1.0f, 0.0f).flip());
    
    return mesh;
}

Mesh *
primitive::cube(bool uv_mapped)
{
    Mesh *mesh = new Mesh();
    mesh->indices.reserve(36);

    if (uv_mapped)
    {
        mesh->vertices.reserve(24);
        math::Vec3 n;

        // left
        n = math::Vec3::left();
        (*mesh)
        .add(math::Vec3(-1.0f, +1.0f, -1.0f), n, math::Vec2(0.0f, 1.0f)) // 0
        .add(math::Vec3(-1.0f, -1.0f, -1.0f), n, math::Vec2(0.0f, 0.0f)) // 1
        .add(math::Vec3(-1.0f, +1.0f, +1.0f), n, math::Vec2(1.0f, 1.0f)) // 2
        .add(math::Vec3(-1.0f, -1.0f, +1.0f), n, math::Vec2(1.0f, 0.0f)) // 3
        .add(0, 1, 2)
        .add(2, 1, 3);

        // right
        n = math::Vec3::right();
        (*mesh)
        .add(math::Vec3(+1.0f, +1.0f, -1.0f), n, math::Vec2(1.0f, 1.0f)) // 4
        .add(math::Vec3(+1.0f, -1.0f, -1.0f), n, math::Vec2(1.0f, 0.0f)) // 5
        .add(math::Vec3(+1.0f, +1.0f, +1.0f), n, math::Vec2(0.0f, 1.0f)) // 6
        .add(math::Vec3(+1.0f, -1.0f, +1.0f), n, math::Vec2(0.0f, 0.0f)) // 7
        .add(4, 6, 5)
        .add(6, 7, 5);
        
        // top
        n = math::Vec3::up();
        (*mesh)
        .add(math::Vec3(-1.0f, +1.0f, -1.0f), n, math::Vec2(0.0f, 1.0f)) // 8
        .add(math::Vec3(+1.0f, +1.0f, -1.0f), n, math::Vec2(1.0f, 1.0f)) // 9
        .add(math::Vec3(+1.0f, +1.0f, +1.0f), n, math::Vec2(1.0f, 0.0f)) // 10
        .add(math::Vec3(-1.0f, +1.0f, +1.0f), n, math::Vec2(0.0f, 0.0f)) // 11
        .add(8, 10, 9)
        .add(10, 8, 11);
        
        // bottom
        n = math::Vec3::down();
        (*mesh)
        .add(math::Vec3(-1.0f, -1.0f, -1.0f), n, math::Vec2(0.0f, 0.0f)) // 12
        .add(math::Vec3(+1.0f, -1.0f, -1.0f), n, math::Vec2(1.0f, 0.0f)) // 13
        .add(math::Vec3(-1.0f, -1.0f, +1.0f), n, math::Vec2(0.0f, 1.0f)) // 14
        .add(math::Vec3(+1.0f, -1.0f, +1.0f), n, math::Vec2(1.0f, 1.0f)) // 15
        .add(12, 13, 14)
        .add(15, 14, 13);

        // front
        n = math::Vec3::forward();
        (*mesh)
        .add(math::Vec3(-1.0f, +1.0f, +1.0f), n, math::Vec2(0.0f, 1.0f)) // 16
        .add(math::Vec3(+1.0f, +1.0f, +1.0f), n, math::Vec2(1.0f, 1.0f)) // 17
        .add(math::Vec3(-1.0f, -1.0f, +1.0f), n, math::Vec2(0.0f, 0.0f)) // 18
        .add(math::Vec3(+1.0f, -1.0f, +1.0f), n, math::Vec2(1.0f, 0.0f)) // 19
        .add(16, 18, 17)
        .add(19, 17, 18);

        // back
        n = math::Vec3::back();
        (*mesh)
        .add(math::Vec3(-1.0f, +1.0f, -1.0f), n, math::Vec2(1.0f, 1.0f)) // 20
        .add(math::Vec3(+1.0f, +1.0f, -1.0f), n, math::Vec2(0.0f, 1.0f)) // 21
        .add(math::Vec3(+1.0f, -1.0f, -1.0f), n, math::Vec2(0.0f, 0.0f)) // 22
        .add(math::Vec3(-1.0f, -1.0f, -1.0f), n, math::Vec2(1.0f, 0.0f)) // 23
        .add(20, 21, 22)
        .add(20, 22, 23);
    }
    else
    {
        mesh->vertices.reserve(8);

        (*mesh)
        .add(math::Vec3(-1.0f, +1.0f, -1.0f)) // 0
        .add(math::Vec3(+1.0f, +1.0f, -1.0f)) // 1
        .add(math::Vec3(+1.0f, -1.0f, -1.0f)) // 2
        .add(math::Vec3(-1.0f, -1.0f, -1.0f)) // 3
        .add(math::Vec3(-1.0f, +1.0f, +1.0f)) // 4
        .add(math::Vec3(+1.0f, +1.0f, +1.0f)) // 5
        .add(math::Vec3(+1.0f, -1.0f, +1.0f)) // 6
        .add(math::Vec3(-1.0f, -1.0f, +1.0f)) // 7

            // left
        .add(0, 3, 4)
        .add(3, 7, 4)

            // right
        .add(1, 5, 2)
        .add(2, 5, 6)

            // top
        .add(0, 5, 1)
        .add(0, 4, 5)

            // bottom
        .add(2, 7, 3)
        .add(2, 6, 7)

            // front
        .add(5, 4, 7)
        .add(5, 7, 6)

            // back
        .add(0, 1, 2)
        .add(0, 2, 3);
    }

    return mesh;
}

Mesh *
primitive::sphere(int parallels, int meridians)
{
    float
        m       = 0.0f,
        mt      = 0.0f,
        p_step  = math::PI / (float)parallels,
        m_step  = math::DOUBLE_PI / (float)meridians,
        pt_step = -1.0f / (float)parallels,
        mt_step =  1.0f / (float)meridians;
 
    Mesh *mesh = new Mesh();
    
    int i = 0;
    for (int m_i = 0; m_i < meridians; ++m_i)
    {
        float
            p   = 0.0f + p_step,
            pt  = 1.0f + pt_step,
            next_m  = m  + m_step,
            next_mt = mt + mt_step;

        // Glue last vertices to the beginning to prevent gaps
        if (m_i == meridians - 1)
        {
            next_m  = 0.0f;
            next_mt = 0.0f;
        }

        // Square faces
        for (int p_i = 2; p_i < parallels; ++p_i)
        {
            // Current parallel
            mesh->add(
                math::Vec3(cos(m) * sin(p), cos(p), sin(m) * sin(p)),
                math::Vec2(mt, pt));

            mesh->add(
                math::Vec3(cos(next_m) * sin(p), cos(p), sin(next_m) * sin(p)),
                math::Vec2(next_mt, pt));
            
            p  += p_step;
            pt += pt_step;
            
            // Next parallel
            mesh->add(
                math::Vec3(cos(m) * sin(p), cos(p), sin(m) * sin(p)),
                math::Vec2(mt, pt));

            mesh->add(
                math::Vec3(cos(next_m) * sin(p), cos(p), sin(next_m) * sin(p)),
                math::Vec2(next_mt, pt));
            
            mesh->add(i, i + 1, i + 3);
            mesh->add(i, i + 3, i + 2);
            i += 4;
        }

        // North pole (triangles)
        mesh->add(math::Vec3::up(),
            math::Vec2((mt + next_mt) / 2.0f, 1.0f));

        mesh->add(math::Vec3(
            cos(m) * sin(p_step), cos(p_step), sin(m) * sin(p_step)),
            math::Vec2(mt, 1.0f + pt_step));

        mesh->add(math::Vec3(
            cos(next_m) * sin(p_step), cos(p_step), sin(next_m) * sin(p_step)),
            math::Vec2(next_mt, 1.0f + pt_step));

        mesh->add(i, i + 2, i + 1);
        i += 3;
            
        // South pole (triangles)
        mesh->add(math::Vec3::down(),
            math::Vec2((mt + next_mt) / 2.0f, 0.0f));

        mesh->add(math::Vec3(cos(m) * sin(p), cos(p), sin(m) * sin(p)),
            math::Vec2(mt, pt));

        mesh->add(math::Vec3(
            cos(next_m) * sin(p), cos(p), sin(next_m) * sin(p)),
            math::Vec2(next_mt, pt));

        mesh->indices.push_back(i++);
        mesh->indices.push_back(i++);
        mesh->indices.push_back(i++);

        m  = next_m;
        mt = next_mt;
    }
    
    mesh->build_normals();
    
    return mesh;
}

Mesh *
primitive::half_sphere(int parallels, int meridians, bool hollow)
{
    parallels /= 2;
 
    float
        m       = 0.0f,
        mt      = 0.0f,
        p_step  = math::HALF_PI / (float)parallels,
        m_step  = math::DOUBLE_PI / (float)meridians,
        pt_step = -0.5f / (float)parallels,
        mt_step =  1.0f / (float)meridians;
 
    Mesh *mesh = new Mesh();
    
    int i = 0;
    for (int m_i = 0; m_i < meridians; ++m_i)
    {
        float
            p   = 0.0f + p_step,
            pt  = 1.0f + pt_step,
            next_m  = m  + m_step,
            next_mt = mt + mt_step;

        // Glue last vertices to the beginning to prevent gaps
        if (m_i == meridians - 1)
        {
            next_m  = 0.0f;
            next_mt = 0.0f;
        }

        // Square faces
        for (int p_i = 1; p_i < parallels; ++p_i)
        {
            // Current parallel
            mesh->add(
                math::Vec3(cos(m) * sin(p), cos(p), sin(m) * sin(p)),
                math::Vec2(mt, pt));

            mesh->add(
                math::Vec3(cos(next_m) * sin(p), cos(p), sin(next_m) * sin(p)),
                math::Vec2(next_mt, pt));
            
            p  += p_step;
            pt += pt_step;
            
            // Next parallel
            mesh->add(
                math::Vec3(cos(m) * sin(p), cos(p), sin(m) * sin(p)),
                math::Vec2(mt, pt));

            mesh->add(
                math::Vec3(cos(next_m) * sin(p), cos(p), sin(next_m) * sin(p)),
                math::Vec2(next_mt, pt));
            
            mesh->add(i, i + 1, i + 3);
            mesh->add(i, i + 3, i + 2);
            i += 4;
        }

        // North pole (triangles)
        mesh->add(math::Vec3(0.0f, 1.0f, 0.0f),
            math::Vec2((mt + next_mt) / 2.0f, 1.0f));

        mesh->add(math::Vec3(
            cos(m) * sin(p_step), cos(p_step), sin(m) * sin(p_step)),
            math::Vec2(mt, 1.0f + pt_step));

        mesh->add(math::Vec3(
            cos(next_m) * sin(p_step), cos(p_step), sin(next_m) * sin(p_step)),
            math::Vec2(next_mt, 1.0f + pt_step));

        mesh->add(i, i + 2, i + 1);
        i += 3;

        m  = next_m;
        mt = next_mt;
    }
    
    mesh->build_normals();
    
    if (!hollow)
    {
       mesh->add(&primitive::circle(meridians)->flip());
     }
    
    return mesh;
}

Mesh *
primitive::icosphere(int subdiv)
{

    Mesh *mesh = new Mesh();
    
    // Build an icosahedron
    float x = 2.0f / (1.0f + sqrt(5.0f));

    (*mesh)
    .add(math::Vec3(-x,  1.0f,  0.0f)) // 0
    .add(math::Vec3( x,  1.0f,  0.0f)) // 1
    .add(math::Vec3(-x, -1.0f,  0.0f)) // 2
    .add(math::Vec3( x, -1.0f,  0.0f)) // 3

    .add(math::Vec3( 0.0f, -x,  1.0f)) // 4
    .add(math::Vec3( 0.0f,  x,  1.0f)) // 5
    .add(math::Vec3( 0.0f, -x, -1.0f)) // 6
    .add(math::Vec3( 0.0f,  x, -1.0f)) // 7

    .add(math::Vec3( 1.0f,  0.0f, -x)) // 8
    .add(math::Vec3( 1.0f,  0.0f,  x)) // 9
    .add(math::Vec3(-1.0f,  0.0f, -x)) // 10
    .add(math::Vec3(-1.0f,  0.0f,  x)) // 11

    .add(0,  11, 5)
    .add(0,  5,  1)
    .add(0,  1,  7)
    .add(0,  7,  10)
    .add(0,  10, 11)

    .add(1,  5,  9)
    .add(5,  11, 4)
    .add(11, 10, 2)
    .add(10, 7,  6)
    .add(7,  1,  8)

    .add(3,  9,  4)
    .add(3,  4,  2)
    .add(3,  2,  6)
    .add(3,  6,  8)
    .add(3,  8,  9)

    .add(4,  9,  5)
    .add(2,  4,  11)
    .add(6,  2,  10)
    .add(8,  6,  7)
    .add(9,  8,  1)
    
    .subdivide(subdiv);
    
    // Make america round again & wrap texture
    for (Mesh::Vertices::iterator i = mesh->vertices.begin();
        i != mesh->vertices.end(); ++i)
    {
        i->pos.normalize();

        float a = atan2(i->pos.z, i->pos.x) / math::DOUBLE_PI;

        i->uv = math::Vec2(
            1.0f - (a + (a < 0.0f)),
            i->pos.y / 2.0f + 0.5
        );
    }

    mesh->build_normals();

    return mesh;
}

Mesh *
primitive::grid(int size, bool double_sided)
{
    Mesh *mesh = new Mesh();

    int cells = size * size;
    mesh->vertices.reserve(cells * 4);
    mesh->indices.reserve(cells * 6);

    math::Vec3 n = math::Vec3::up();

    struct
    {
       float x, z;
    } offset[4] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

    int base = 0;
    for (int z = 0; z < size; ++z)
    {
        for (int x = 0; x < size; ++x)
        {
            for (int i = 0; i < 4; ++i)
            {
                math::Vec3 v = math::Vec3(
                    x + offset[i].x - size / 2.0f,
                    0.0f,
                    z + offset[i].z - size / 2.0f);
                mesh->add(v, n, math::Vec2(v.x, v.z));
            }

            mesh->add(base, base + 1, base + 2);
            mesh->add(base, base + 2, base + 3);
            
            if (double_sided)
            {
                mesh->add(base, base + 2, base + 1);
                mesh->add(base, base + 3, base + 2);
            }
            base += 4;
        }
    }

    return mesh;
}
