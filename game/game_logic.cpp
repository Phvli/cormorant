#include "game_logic.h"

#include <cmath>

using namespace core;

#include "../core/engine.h"
#include "../math/util.h"
#include "entity/graphics/rigid.h"
#include "entity/graphics/dynamic.h"
#include "entity/physics/dynamic.h"
#include "entity/activation/timed.h"
#include "entity/ai/missile.h"
#include "../gfx/font.h"
#include "terrain.h"

#include "../core/util/string.h"
#include "menu.h"

static game::Menu *menu = NULL;
static game::DynamicGraphics *player_meshes = NULL;
// static bool sonar = false, old_sonar = false;
// static unsigned long next_surf = 0;
// (!) FIXME: find a better way ^


#include "../version.h"
static void
_update_mfd(void)
{
    gfx::Texture *framebuffer = gfx::Texture::framebuffer("MFD");

    gfx::Font
        *header = gfx::Font::get("normal"),
        *text   = gfx::Font::get("small");

    framebuffer->target();
    glClearColor(0.01, 0.02, 0.03, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    bool terrain_alert = (screen.scene->player->physics->altitude()
        < screen.scene->player->physics->velocity.length() * .4f + 5);

    game::Entity *missile = NULL;
    long furthest = 0;
    for (gfx::Scene::EntityContainer::const_iterator
        e = screen.scene->entities.begin();
        e != screen.scene->entities.end(); ++e)
    {
        if (((*e)->flags & game::Entity::ACTIVE) && (*e)->has_tag("missile"))
        {
            long dist = (*e)->pos.dist_sq(screen.scene->player->pos);
            if (dist > furthest)
            {
                furthest = dist;
                missile = *e;
            }
        }
    }

    if (input[Input::WEAPON_FIRE2])
    {
        gfx::Texture *map = screen.scene->ground_plane->material->color_map;
        float
            r = .2f,
            x = screen.scene->player->x
                / ((float)game::terrain.w * game::TerrainNode::SIZE) - r,
            y = screen.scene->player->z
                / ((float)game::terrain.h * game::TerrainNode::SIZE) - r;
            
        r *= 2.0f;
        map->blit(-.9f, -.9f, 1.8f, 1.8f, x, y, r, r);

        text
            ->set_size(40)
            ->set_color(0xff0000)
            ->add_effect(gfx::Font::OUTLINE, 0x000000)
            ->at(0.0f, 0.0f)
            ->center("+");
    }
    else if (missile != NULL)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        game::Entity
            &camera = screen.scene->camera,
            backup;
        
        backup.rot = camera.rot;
        backup.pos = camera.pos;
        
        screen.scene->player->graphics->visible = true;

        camera.rot = math::Quat::rotation_y(math::PI)
            * missile->rot;

        camera.pos = missile->pos
            + math::Vec3() * (math::Mat4::identity()
                .translate(0.0f, 0.0f, -5.0f)
                * camera.rot);

        screen.scene->render();
        screen.scene->player->graphics->visible = false;

        camera.rot = backup.rot;
        camera.pos = backup.pos;
    }
    else
    {
        float
            font_size = 25,
            velocity  = screen.scene->player->physics->velocity.length();
        
        math::Vec3
            vector = screen.scene->player->physics->velocity,
            rot    = screen.scene->player->rot.vec3();

        header
            ->prepare()
            ->set_size(30)
            ->set_color(0x10f030)
            ->add_effect(gfx::Font::ITALIC)
            ->at(-1.0f, +1.0f)->align_left(APP_NAME " " VER_STRING)
            ->at(+1.0f, +1.0f)->align_right("pre-alpha");

        text
            ->set_color(0x10f030)
            ->add_effect(gfx::Font::MONOSPACE)

            ->set_size(42)
            ->at(+.15f, -0.5f)->align_left("HDG: %03i",
                (int)(screen.scene->player->physics->heading()))
            ->at(+.15f, -0.7f)->align_left("PWR: %03i%%",
                (int)screen.scene->player->physics->accel.z / 5)
            
            ->set_size(font_size)
            ->at(0.0f, +0.8f)->align_middle("ALT")

            ->at(0.0f, +0.7f)->align_middle("CLIMB")
            ->at(-.2f, +0.7f)->align_right("%+0.1f ft/s", vector.y * math::convert::MS_TO_FTS)
            ->at(+.2f, +0.7f)->align_left("%+0.1f m/s", vector.y)
            
            ->at(0.0f, +0.45f)->align_middle("VEL")
            ->at(-.2f, +0.5f)->align_right("%0.1f kt", velocity * math::convert::MS_TO_KN)
            ->at(+.2f, +0.5f)->align_left("%0.1f m/s", velocity)
            ->at(-.2f, +0.4f)->align_right("%0.1f mach", velocity * math::convert::MS_TO_MACH)
            ->at(+.2f, +0.4f)->align_left("%0.1f km/h", velocity * math::convert::MS_TO_KMH)

            ->at(-.2f, +0.2f)->align_right("X:")
            ->at(+.2f, +0.2f)->align_left("Z:")
            ->at(0.0f, +0.1f)->align_middle("POS")
            ->at(-.2f, +0.1f)->align_right("%4.1f km",
                screen.scene->player->x / 1000.0f)
            ->at(+.2f, +0.1f)->align_left("%4.1f km",
                screen.scene->player->z / 1000.0f)
            ->at(0.0f, +0.0f)->align_middle("DIFF")
            ->at(-.2f, +0.0f)->align_right("%+.2f m/s",
                vector.x / 1000.0f)
            ->at(+.2f, +0.0f)->align_left("%+.2f m/s",
                vector.z / 1000.0f)

            ->at(-.8f, -0.3f)->align_left("YAW:    %+03i",
                (int)(screen.scene->player->physics->yaw() * math::convert::RAD_TO_DEG))

            ->at(-.8f, -0.4f)->align_left("PITCH:  %+03i",
                (int)(screen.scene->player->physics->pitch() * math::convert::RAD_TO_DEG))

            ->at(-.8f, -0.5f)->align_left("ROLL:   %+03i",
                (int)(screen.scene->player->physics->roll() * math::convert::RAD_TO_DEG))

            ->at(-.8f, -0.7f)->align_left("QUAT X: %+03i",
                (int)(rot.x * math::convert::RAD_TO_DEG))

            ->at(-.8f, -0.8f)->align_left("QUAT Y: %+03i",
                (int)(rot.y * math::convert::RAD_TO_DEG))

            ->at(-.8f, -0.9f)->align_left("QUAT Z: %+03i",
                (int)(rot.z * math::convert::RAD_TO_DEG))
            ;
        
        if (terrain_alert)
        {
            if ((core::engine.ticks % 1000) < 500)
            {
                text
                    ->set_background(0x10f030)
                    ->set_color(0xff000000);
                text
                    ->set_background(0xf03010)
                    ->set_color(0xff000000);
            }
            else
            {
                text
                    ->set_color(0xf03010);
            }
        }

        text
            ->at(-.2f, +0.8f)->align_right(" %0.1f ft", screen.scene->player->y * math::convert::M_TO_FT)
            ->at(+.2f, +0.8f)->align_left("%0.1f m ", screen.scene->player->y);

        // menu->render();

        text->prepare(false);
    }

    if (terrain_alert)
    {
        if ((core::engine.ticks % 500) < 250)
        {
            text
                ->prepare()
                ->set_background(0x80c0b010)
                ->set_color(0xff000000)
                ->set_size(80)
                ->at(0.0f, +0.0f)
                ->align_middle(((core::engine.ticks % 3000) < 1500)
                    ? " PULL UP "
                    : " TERRAIN ")
                ->prepare(false);
        }

        if (((screen.scene->frame * 10) % core::engine.frames_per_second) == 0)
        {
            core::engine.play_sound("alert", 0, .5f);
        }
    }
    

    framebuffer->target(false);
    
    delete text;
    delete header;
}

void
GameLogic::initialize_module()
{
    menu = new game::Menu();
    core::str::set(menu->container->caption, core::str::dup("ROOT"));
    menu->container
        ->set_vertical()
        ->add((new game::Menu::Container("a", .8))
            ->set_horizontal()
            ->add((new game::Menu::Container(.3))
                ->add(new game::Menu::Container("hi"))
                ->add(new game::Menu::Container("mid"))
                ->add(new game::Menu::Container("low", .5))
            )
            ->add((new game::Menu::Container("main"))
                ->add(new game::Menu::Container("Hello, World!", .2))
                ->add((new game::Menu::Container())
                    ->set_horizontal()
                    ->add(new game::Menu::Container("left"))
                    ->add(menu->selection = new game::Menu::Container("center"))
                    ->add(new game::Menu::Container("right"))
                )
                ->add((new game::Menu::Container("b", .2))
                    ->set_horizontal()
                    ->add(new game::Menu::Container())
                    ->add(new game::Menu::Container())
                    ->add(new game::Menu::Container())
                    ->add(new game::Menu::Container())
                    ->add((new game::Menu::Container("c", .4))
                        ->add(new game::Menu::Container())
                        ->add(new game::Menu::Container())
                        ->add((new game::Menu::Container("d"))
                            ->set_horizontal()
                            ->add(new game::Menu::Container())
                            ->add(new game::Menu::Container())
                            ->add(new game::Menu::Container("test", .6))
                            ->add(new game::Menu::Container())
                        )
                        ->add(new game::Menu::Container())
                    )
                    ->add(new game::Menu::Container())
                    ->add(new game::Menu::Container())
                    ->add(new game::Menu::Container())
                    ->add(new game::Menu::Container())
                    ->add(new game::Menu::Container())
                )
            )
        )

        ->add((new game::Menu::Container("bottom"))
            ->set_horizontal()
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
            ->add(new game::Menu::Container())
        )
    ;
    menu->container->compose();
}

GameLogic::GameLogic()
{
    this->register_module(this, false);
}

GameLogic::~GameLogic()
{
    this->log(this->get_module_name());
    delete menu;
}

void
GameLogic::update_tick(void)
{
    math::Vec3
        throttle,
        rotation;
    
    rotation.x = .1f *  (input[Input::PITCH_UP]      - input[Input::PITCH_DOWN]);
    rotation.z = .3f *  (input[Input::ROLL_LEFT]     - input[Input::ROLL_RIGHT]);
    rotation.y = .1f *  (input[Input::RUDDER_RIGHT]  - input[Input::RUDDER_LEFT]);
    throttle.z = (float)(input[Input::THROTTLE_UP] - input[Input::THROTTLE_DOWN]);

    // rotation.x     = .03f * (input[Input::PITCH] + input[Input::PITCH_UP]  - input[Input::PITCH_DOWN]);
    // rotation.y     = .01f * (input[Input::ROLL] + input[Input::ROLL_RIGHT] - input[Input::ROLL_LEFT]);
    // throttle.z = 3.5f * (input[Input::THROTTLE] + input[Input::THROTTLE_UP] - input[Input::THROTTLE_DOWN]);

    if (input.joysticks)
    {
        rotation.z -= 0.35f * math::deadzone(input.joystick[0](0), .08f);
        rotation.x += 0.15f * math::deadzone(input.joystick[0](1), .08f);
        rotation.y += 0.15f * math::deadzone(input.joystick[0](2), .08f);
        throttle.z += pow((.5f - input.joystick[0](3) / 2.0f), 3.0f);
    }
    
    screen.scene->fog.w = math::clamp(screen.scene->fog.w
        + 0.000005f * (input[Input::WEAPON_PREV] - input[Input::WEAPON_NEXT]),
        0.00000f, 0.00050f);
    
    // Accelerate
    
    screen.scene->player->physics->accel.z
        = math::interpolate::linear(screen.scene->player->physics->accel.z,
            500.0f * throttle.z,
            0.1f);
    // screen.scene->player->physics->accel     += throttle * 500.0f;
    // screen.scene->player->physics->accel     += throttle * .0005f;
    screen.scene->player->physics->turn_rate += rotation * .005f;
    
    // Gravity pulls nose down
    // float turbulence = 1.0f / (1.0f
    //     + screen.scene->player->physics->vector().xz().length());
    
    // screen.scene->player->physics->accel.y
    //     -= (9.81f * math::clamp(
    //         1.0f - 5.0f * screen.scene->player->physics->vector().xz().length(),
    //         0.0f, 1.0f))
    //         / core::engine.ticks_per_second;

    // screen.scene->player->rot.slerp(
    //     math::Quat::facing(screen.scene->player->physics->vector()),
    //     // math::Quat::rotation_x(-math::HALF_PI),
    //     .01f);
    //     // turbulence * .01f);

    // Turbulence
    // if (math::probability(.05f))
    // {
        // screen.scene->player->physics->turn_rate
        //     += math::Vec3::random(math::rnd(turbulence * .0001f));
    // }
    
    // Dampen acceleration and speed
    // screen.scene->player->physics->accel     *= .9f;
    screen.scene->player->physics->velocity  *= .98f;
    screen.scene->player->physics->turn_rate *= .97f;
    
goto skip_dynamic;
    // Rotate player model components (persicopes, propellers etc)
    // screen.scene->player->rot.z = 128.0f * screen.scene->player->physics->turn_rate.y;
    (*player_meshes)["propeller"].transformation
        .translateY(1.63f)
        .rotZ(-screen.scene->player->physics->velocity.z * 0.1f)
        // .rotZ(-screen.scene->player->physics->accel.z * 27.0f)
        .translateY(-1.63f);

    (*player_meshes)["rudder"].transformation = math::Mat4::identity()
        .translateZ(3.93f)
        .rotY(-screen.scene->player->physics->turn_rate.y * 50.0f)
        .translateZ(-3.93f);

    (*player_meshes)["periscope"].transformation = math::Mat4::identity()
        .translateZ(-.26f)
        .rotY(.001f * core::engine.ticks)
        .translate(0.0f, math::clamp(
            .07f * (screen.scene->player->y + 2.5f), -3.0f, 0.0f),
            .26f);

    (*player_meshes)["plane_fl"].transformation = math::Mat4::identity()
        .translate(0.0f, 0.47f, -2.68f)
        .rotX(-screen.scene->player->rot.x * 2.0f)
        .rotX(+screen.scene->player->rot.z * 1.8f)
        .translate(0.0f, -0.47f, 2.68f);

    (*player_meshes)["plane_fr"].transformation = math::Mat4::identity()
        .translate(0.0f, 0.47f, -2.68f)
        .rotX(-screen.scene->player->rot.x * 2.0f)
        .rotX(-screen.scene->player->rot.z * 1.8f)
        .translate(0.0f, -0.47f, 2.68f);

    (*player_meshes)["plane_br"].transformation = math::Mat4::identity()
        .translate(0.0f, 0.44f, 2.87f)
        .rotX(-screen.scene->player->rot.x * 2.0f)
        .rotX(+screen.scene->player->rot.z * 1.8f)
        .translate(0.0f, -0.44f, -2.87f);

    (*player_meshes)["plane_bl"].transformation = math::Mat4::identity()
        .translate(0.0f, 0.44f, 2.87f)
        .rotX(-screen.scene->player->rot.x * 2.0f)
        .rotX(-screen.scene->player->rot.z * 1.8f)
        .translate(0.0f, -0.44f, -2.87f);
skip_dynamic:

    // Update scene
    screen.scene->update();
}

void
GameLogic::update_frame()
{
    static const int  MAX_VIEWS  = 7;
    static int        view_index = 1;
    static math::Vec3 look_offset;

    game::Entity
        &camera  =  screen.scene->camera,
        &subject = *screen.scene->player;

 
    // Fire ze missiles! Fire our shit!
    if (input[Input::WEAPON_FIRE1])
    {
        static int missile_side = 0;
        game::Entity *missile = screen.scene->add(new game::Entity());
        missile->add_tag("missile");
        missile->attach(new game::DynamicPhysics());
        missile->attach(new game::RigidGraphics());
        missile->attach(new game::MissileAI());
        missile->attach(new game::TimedDeactivation(10000));
        missile->graphics->model = gfx::Model::load("ordnance/sidewinder/sidewinder.obj");
        missile->pos = screen.scene->player->pos
            + math::Vec3(-6.0f + 12.0f * missile_side, -0.5, 2.0f)
            * (math::Mat4::identity() * screen.scene->player->rot);
        missile_side = (missile_side + 1) % 2;
        missile->rot = screen.scene->player->rot;
        missile->physics->velocity = screen.scene->player->physics->velocity;
        missile->physics->collision_detection = false; // (!) FIXME
    }

    // Change views (F1 - Fn)
    int old_view = view_index;
    for (int i = 0; i < MAX_VIEWS; ++i)
    {
        if (input.keyboard[SDL_SCANCODE_F1 + i])
        {
            old_view   = -1;
            view_index = i;
        }
    }

    // Change views (bound keys)
    view_index = ((view_index
        + (int)input[Input::VIEW_NEXT]
        - (int)input[Input::VIEW_PREV])
        + MAX_VIEWS) % MAX_VIEWS;
    
    // Rotate view
    math::Quat mouselook = math::Quat::rotation_x(look_offset.x)
        * math::Quat::rotation_y(look_offset.y);

    look_offset.x += input.mouse.diff.y / 250.0f
        + input[Input::LOOK_X];
    look_offset.y += input.mouse.diff.x / 250.0f
        + input[Input::LOOK_Y];

    if (input[Input::LOOK_CENTER] || view_index != old_view)
    {
        // Center view
        look_offset = math::Vec3();
        core::engine.play_sound("select", 0, .2f);
    }

    math::Vec3 v;
    switch (view_index)
    {
        case 0:
            // Nose camera
            subject.graphics->visible = true;

            camera.pos = subject.pos
                + math::Vec3() * (math::Mat4::identity()
                    .translate(0.0f, 1.5f, subject.graphics->model->radius())
                    * subject.rot);
            camera.rot = math::Quat::rotation_y(-math::PI)
                    * subject.rot;

            break;

        
        case 1:
            // Cockpit camera
            _update_mfd();
            
            subject.graphics->visible = false;
            
            camera.pos = subject.pos
                + math::Vec3() * (math::Mat4::identity()
                    .translate(0.0f, 1.1f, 7.47f) // note: this should the correct cockpit position
                    * subject.rot);

            // Rotate with player
            camera.rot.slerp(mouselook
                * (math::Quat::rotation_y(math::PI)
                * subject.rot),
                .15f);
            
            // Limit view
            look_offset.x = math::clamp(look_offset.x, -1.4f, 1.0f);
            look_offset.y = math::clamp(look_offset.y, -2.0f, 2.0f);
            break;

        case 2:
            // Tail camera
            subject.graphics->visible = true;

            camera.rot.slerp(math::Quat::rotation_y(
                    subject.physics->yaw() + math::HALF_PI), .1);
            camera.pos = subject.pos
                + math::Vec3() * (math::Mat4::identity()
                    .translate(0.0f, 7.5f, subject.graphics->model->radius() + 20.0f)
                    * camera.rot);
            break;


        case 3:
            // Outside view, camera circles player
            subject.graphics->visible = true;

            camera.rot = mouselook
                * math::Quat::rotation_y(5.5f + subject.physics->yaw());
            camera.pos = subject.pos
                + math::Vec3(0.0f, 6.0f, 40.0f)
                * camera.rot.mat4();
            break;


        case 4:
            // Autocircling camera
            subject.graphics->visible = true;

            camera.rot.slerp(
                mouselook
                * math::Quat::rotation_y(core::engine.ticks / 5000.0f),
                .05f);

            camera.pos = subject.pos
                + math::Vec3(0.0f, 0.0f, 70.0f)
                * camera.rot.mat4();
            break;


        case 5:
            // Static camera, always facing player
            subject.graphics->visible = true;

            if (view_index != old_view)
            {
                // Start static camera
                camera.pos = subject.pos
                    + math::Vec3(0.0f, math::vary(-50.0f, 100.0f), 50 + 2 * subject.physics->accel.z)
                    * subject.rot.mat4().rotY(math::vary(.2f));
            }

            camera.rot.smooth_slerp(
                -math::Quat::facing(
                    camera.pos,
                    subject.pos),
                .2f
            );
            break;


        case 6:
            // Bird's eye view
            subject.graphics->visible = true;

            v = subject.physics->vector();
            v.y = 0.0f;
            v.normalize();
            v *= 200.0f;
            v.y = 500.0f;
            camera.pos = subject.pos + v;
            camera.rot = math::Quat::rotation_x(math::HALF_PI)
                * math::Quat::rotation_y(
                    subject.physics->yaw() + math::HALF_PI);
            break;
    }
    
    camera.rot.normalize();

    // Camera-ground collision prevention
    if (screen.scene->player->physics->collision_detection)
    {
        camera.y = math::max(
            camera.y,
            math::max(0.0f, game::terrain.at(camera.pos).height) + 1.5f
        );
    }
}