#include "dynamic.h"

#include "../../../core/engine.h"

using namespace game;

void
DynamicGraphics::render(void)
{
    if (this->visible)
    {
        if (this->static_meshes.size() + this->dynamic_meshes.size() == 0)
        {
            this->sort();
        }

        math::Mat4 modelview =
            (math::Mat4(this->entity->transformation) * this->entity->rot)
            .translate(this->entity->pos)
            * screen.scene->matrix.camera;

        screen.scene->matrix.mv = modelview;

        // Render regular, static meshes
        for (gfx::Model::Meshes::const_iterator
            regular = this->static_meshes.begin();
            regular != this->static_meshes.end();
            ++regular)
        {
            (*regular)->render();
        }

        // Render meshes that have their own cache
        for (DynamicIndex::const_iterator
            dynamic = this->dynamic_mesh_index.begin();
            dynamic != this->dynamic_mesh_index.end(); ++dynamic)
        {
            screen.scene->matrix.mv
                = (*dynamic)->transformation
                * modelview;
            (*dynamic)->mesh->render();
        }
    }
}

DynamicGraphics::DynamicGraphics()
{
    this->visible = true;
    this->model   = NULL;
}

DynamicGraphics::~DynamicGraphics()
{
    if (this->model != NULL)
    {
        this->model->unload();
    }
}

int
DynamicGraphics::make_dynamic(const char *mesh_name, int id, bool dynamic)
{
    return (this->model != NULL)
        ? this->make_dynamic(this->model->get(mesh_name), id, dynamic)
        : -1;
}

int
DynamicGraphics::make_dynamic(const gfx::Mesh *mesh, int id, bool dynamic)
{
    if (mesh == NULL)
    {
        return -1;
    }
    
    // (!) FIXME: TODO: bool dynamic doesn't do anything (should remove existing if false)
    if (id < 0)
    {
        id = this->dynamic_meshes.size();
        this->dynamic_meshes.resize(id + 1);
    }
    else if (id >= (int)this->dynamic_meshes.size())
    {
        int i = (int)this->dynamic_meshes.size() - 1;
        this->dynamic_meshes.resize(id + 1);
        for (; i < id; ++i)
        {
            this->dynamic_meshes[i].id   = -1;
            this->dynamic_meshes[i].mesh = NULL;
        }
    }
    
    this->dynamic_meshes[id].id   = id;
    this->dynamic_meshes[id].mesh = mesh;
    this->dynamic_meshes[id].transformation = math::Mat4::identity();
    this->sort();

    return id;
}

void
DynamicGraphics::sort(void)
{
    this->static_meshes.clear();
    this->dynamic_mesh_index.clear();
    if (this->model == NULL)
    {
        return;
    }

    for (gfx::Model::Meshes::const_iterator mesh = this->model->meshes.begin();
        mesh != this->model->meshes.end(); ++mesh)
    {
        this->static_meshes.push_back(*mesh);

        for (DynamicMeshes::iterator
            dynamic = this->dynamic_meshes.begin();
            dynamic != this->dynamic_meshes.end(); ++dynamic)
        {
            if (*mesh == dynamic->mesh)
            {
                this->dynamic_mesh_index.push_back(&(*dynamic));
                this->static_meshes.pop_back();
                break;
            }
        }
    }
}

DynamicGraphics::DynamicMesh &
DynamicGraphics::operator[](const gfx::Mesh *mesh)
{
    for (DynamicIndex::iterator indexed = this->dynamic_mesh_index.begin();
        indexed != this->dynamic_mesh_index.end(); ++indexed)
    {
        if (mesh == (*indexed)->mesh)
        {
            return **indexed;
        }
    }
    
    throw 666;
}

#include <cstring> // strcmp
DynamicGraphics::DynamicMesh &
DynamicGraphics::operator[](const char *mesh_name)
{
    for (DynamicIndex::iterator indexed = this->dynamic_mesh_index.begin();
        indexed != this->dynamic_mesh_index.end(); ++indexed)
    {
        if ((*indexed)->mesh != NULL && (*indexed)->mesh->name != NULL
            && strcmp((*indexed)->mesh->name, mesh_name) == 0)
        {
            return **indexed;
        }
    }
    
    throw 666;
}
