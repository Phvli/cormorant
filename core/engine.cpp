#include "engine.h"
#include "../version.h"
#include "../game/terrain.h"

#include <SDL2/SDL.h>

using namespace core;

Cormorant core::engine;

#include "../game/cutscene.h"
#include "../math/util.h"
#include "../gfx/font.h"

#include "../game/menu.h"
#include "util/string.h"
// static game::Menu *menu = NULL;

#include "../game/entity/graphics/dynamic.h"
#include "../game/entity/physics/dynamic.h"

Cormorant::Cormorant()
{
    this->frames_per_second = 120;
    this->ticks_per_second  = 50;
}

Cormorant::~Cormorant()
{
    this->log_header("Shutdown");

    // delete menu;

    // Re-enable cursor in case something goes wrong
    SDL_ShowCursor(SDL_ENABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
}

void
Cormorant::render(void)
{
    // menu->render();


    // SDL_GL_SwapWindow(screen.window);

    if (screen.scene->player == NULL)
        return;
    
    screen.scene->render();

    // Render cockpit
    if (!screen.scene->player->graphics->visible)
    {
        screen.scene->clip(0.01f, 20.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);

        screen.scene->matrix.mv =
            (math::Mat4(screen.scene->player->transformation)
                .translate(-0.0f, -1.1f, -7.47f)
                * screen.scene->player->rot
                * -screen.scene->camera.rot);

        // (!) TODO: render explicit cockpit geometry here instead of player model
        screen.scene->player->graphics->model->render();
    }
}

void
Cormorant::update(void)
{
    // menu->run();
}

static void
_render_loading_screen(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gfx::Font *text = gfx::Font::get();

    if (1)
    {
        text
            ->prepare()
            ->set_color(0xc0c0c0)
            ->set_size(22)
            ->add_effect(gfx::Font::ITALIC)
            ->at(-.5f,  0.25f)->write("\"The duty of the fighter pilot is to patrol his area")
            ->at(-.5f,  0.15f)->write("of the sky, and shoot down any enemy fighters")
            ->at(-.5f,  0.05f)->write("in that area. Anything else is rubbish.\"")
            ->remove_effect(gfx::Font::ITALIC)
            ->at( .5f, -0.25f)->align_right("Manfred von Richthofen");
    }
    else
    {
        glClear(GL_COLOR_BUFFER_BIT);
        text
            ->prepare()
            ->set_color(0xd0d0d0)
            ->add_effect(gfx::Font::BOLD)
            ->set_size(15)
            ->at(0.0f, 0.0f)
            ->center("Loading...");
    }

    SDL_GL_SwapWindow(screen.window);

    delete text;
}

void
Cormorant::initialize()
{
    this->log("Initializing main engine");
    
    // Render player so that there's something to look at
    _render_loading_screen();

    screen.scene->camera.rot.x = .3f;
    screen.scene->camera.rot.y = .1f + math::PI;

    game::terrain.load("galapagos.png");

    // menu = new game::Menu();
    // menu->load("game/menus.txt");

    // core::str::set(menu->container->caption, core::str::dup("ROOT"));

    // menu->container
    //     ->set_vertical()
    //     ->add((new game::Menu::Container("a", .8))
    //         ->set_horizontal()
    //         ->add((new game::Menu::Container(.3))
    //             ->add(new game::Menu::Container("hi"))
    //             ->add(new game::Menu::Container("mid"))
    //             ->add(new game::Menu::Container("low", .5))
    //         )
    //         ->add((new game::Menu::Container("main"))
    //             ->add(new game::Menu::Container("Hello, World!", .2))
    //             ->add((new game::Menu::Container())
    //                 ->set_horizontal()
    //                 ->add(new game::Menu::Container("left"))
    //                 ->add(menu->selection = new game::Menu::Container("center"))
    //                 ->add(new game::Menu::Container("right"))
    //             )
    //             ->add((new game::Menu::Container("b", .2))
    //                 ->set_horizontal()
    //                 ->add(new game::Menu::Container())
    //                 ->add(new game::Menu::Container())
    //                 ->add(new game::Menu::Container())
    //                 ->add(new game::Menu::Container())
    //                 ->add((new game::Menu::Container("c", .4))
    //                     ->add(new game::Menu::Container())
    //                     ->add(new game::Menu::Container())
    //                     ->add((new game::Menu::Container("d"))
    //                         ->set_horizontal()
    //                         ->add(new game::Menu::Container())
    //                         ->add(new game::Menu::Container())
    //                         ->add(new game::Menu::Container("test", .6))
    //                         ->add(new game::Menu::Container())
    //                     )
    //                     ->add(new game::Menu::Container())
    //                 )
    //                 ->add(new game::Menu::Container())
    //                 ->add(new game::Menu::Container())
    //                 ->add(new game::Menu::Container())
    //                 ->add(new game::Menu::Container())
    //                 ->add(new game::Menu::Container())
    //             )
    //         )
    //     )

    //     ->add((new game::Menu::Container("bottom"))
    //         ->set_horizontal()
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //         ->add(new game::Menu::Container())
    //     )
    // ;
    // menu->container->compose();

    // Create player
    screen.scene->player = screen.scene->add(new game::Entity());

    // screen.scene->player->attach(new game::RigidGraphics());
    // player_meshes = new game::DynamicGraphics();
    // screen.scene->player->attach(player_meshes);
    screen.scene->player->attach(new game::DynamicGraphics());
    screen.scene->player->attach(new game::DynamicPhysics());
    screen.scene->player->graphics->model = gfx::Model::load("aircraft/xf-42/cormotest.obj");

    screen.scene->player->x = 70000.0f;
    screen.scene->player->z = 35000.0f;
    screen.scene->player->z = -1000.0f;
    screen.scene->player->y = 500.0f
        + std::max(0.0f, game::terrain.at(screen.scene->player->x, screen.scene->player->z).height);
    
    screen.scene->player->rot.rotate_y(.01f);
    screen.scene->player->physics->velocity.z = 60.0f;
    
    gfx::Mesh *mfd = screen.scene->player->graphics->model->get("MFD");
    mfd->material->color_map = gfx::Texture::framebuffer("MFD", 1024, 512, false, 24);
    
    // player_meshes->make_dynamic("propeller");
    // player_meshes->make_dynamic("rudder");
    // player_meshes->make_dynamic("periscope");
    // player_meshes->make_dynamic("plane_fl");
    // player_meshes->make_dynamic("plane_fr");
    // player_meshes->make_dynamic("plane_bl");
    // player_meshes->make_dynamic("plane_br");

    game::terrain.prefetch(screen.scene->player->pos);

    if (!this->config["video"]["skip_intro"].boolean(false))
    {
        game::Cutscene *intro = new game::Cutscene("intro");
        intro->run();
        delete intro;

        // (!) TODO: use messages instead to skip this
        if (input[Input::QUIT])
        {
            throw 666;
        }
    }

    if (input.joysticks > 0)
    {
        input.bind(Input::WEAPON_FIRE1, input.joystick[0].press(0));
        input.bind(Input::WEAPON_FIRE2, input.joystick[0].hold(1));
        input.bind(Input::VIEW_PREV,    input.joystick[0].press(2));
        input.bind(Input::VIEW_NEXT,    input.joystick[0].press(4));
        input.bind(Input::WEAPON_PREV,  input.joystick[0].hold(3));
        input.bind(Input::WEAPON_NEXT,  input.joystick[0].hold(5));
    }

    core::engine.activate_module("game");
    core::engine.play_music("tropical");
}
