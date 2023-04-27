#include "rigid.h"

#include "../../../core/engine.h"

using namespace game;

void
RigidGraphics::render(void)
{
    if (this->visible)
    {
        screen.scene->matrix.mv =
            (math::Mat4(this->entity->transformation) * this->entity->rot)
            .translate(this->entity->pos)
            * screen.scene->matrix.camera;

        this->model->render();
    }
}

RigidGraphics::RigidGraphics()
{
    this->visible = true;
    this->model   = NULL;
}

RigidGraphics::~RigidGraphics()
{
    if (this->model != NULL)
    {
        this->model->unload();
    }
}
