#include "input.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

#include <vector>
#include <algorithm>
#include <cmath>

#include "../util/string.h"

core::Input input;
using namespace core;

typedef
    std::vector<SDL_GameController *>
    Controllers;
    
typedef
    std::vector<SDL_Joystick *>
    Joysticks;
    
static Controllers controllers;
static Joysticks   joysticks;

static Config _types, _actions;
static void _init_names()
{
    _actions["pitch"]             = Input::PITCH;
    _actions["roll"]              = Input::ROLL;
    _actions["rudder"]            = Input::RUDDER;
    _actions["pitch_up"]          = Input::PITCH_UP;
    _actions["pitch_down"]        = Input::PITCH_DOWN;
    _actions["roll_left"]         = Input::ROLL_LEFT;
    _actions["roll_right"]        = Input::ROLL_RIGHT;
    _actions["rudder_left"]       = Input::RUDDER_LEFT;
    _actions["rudder_right"]      = Input::RUDDER_RIGHT;

    _actions["throttle"]          = Input::THROTTLE;
    _actions["throttle_up"]       = Input::THROTTLE_UP;
    _actions["throttle_down"]     = Input::THROTTLE_DOWN;
    _actions["flaps_up"]          = Input::FLAPS_UP;
    _actions["flaps_down"]        = Input::FLAPS_DOWN;
    _actions["brake"]             = Input::BRAKE;

    _actions["look_x"]            = Input::LOOK_X;
    _actions["look_y"]            = Input::LOOK_Y;
    _actions["look_center"]       = Input::LOOK_CENTER;
    _actions["view_next"]         = Input::VIEW_NEXT;
    _actions["view_prev"]         = Input::VIEW_PREV;

    _actions["menu_up"]           = Input::MENU_UP;
    _actions["menu_down"]         = Input::MENU_DOWN;
    _actions["menu_left"]         = Input::MENU_LEFT;
    _actions["menu_right"]        = Input::MENU_RIGHT;
    _actions["menu_ok"]           = Input::MENU_OK;
    _actions["menu_cancel"]       = Input::MENU_CANCEL;

    _actions["weapon_fire1"]      = Input::WEAPON_FIRE1;
    _actions["weapon_fire2"]      = Input::WEAPON_FIRE2;
    _actions["weapon_next"]       = Input::WEAPON_NEXT;
    _actions["weapon_prev"]       = Input::WEAPON_PREV;

    _actions["fullscreen_toggle"] = Input::FULLSCREEN_TOGGLE;
    _actions["pause"]             = Input::PAUSE;
    _actions["quit"]              = Input::QUIT;

    _types["type"]    = Input::TYPE;
    _types["press"]   = Input::PRESS;
    _types["hold"]    = Input::HOLD;
    _types["release"] = Input::RELEASE;
    _types["analog"]  = Input::ANALOG;
}

void
InputEngine::initialize_module()
{
    int keyboard_keys;
 
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GetRelativeMouseState(NULL, NULL);
 
    SDL_GetKeyboardState(&keyboard_keys);
    
    input.keyboard.init("keyboard", keyboard_keys, 0);
    input.mouse.init("mouse", 5, 2);

    input.bind(Input::PITCH,             input.mouse.analog(0));
    input.bind(Input::ROLL,              input.mouse.analog(1));
    input.bind(Input::PITCH_UP,          input.keyboard.hold(SDL_SCANCODE_DOWN));
    input.bind(Input::PITCH_DOWN,        input.keyboard.hold(SDL_SCANCODE_UP));
    input.bind(Input::ROLL_LEFT,         input.keyboard.hold(SDL_SCANCODE_LEFT));
    input.bind(Input::ROLL_RIGHT,        input.keyboard.hold(SDL_SCANCODE_RIGHT));
    input.bind(Input::RUDDER_LEFT,       input.keyboard.hold(SDL_SCANCODE_Q));
    input.bind(Input::RUDDER_RIGHT,      input.keyboard.hold(SDL_SCANCODE_E));

    input.bind(Input::THROTTLE_UP,       input.keyboard.hold(SDL_SCANCODE_R));
    input.bind(Input::THROTTLE_DOWN,     input.keyboard.hold(SDL_SCANCODE_F));
    input.bind(Input::FLAPS_UP,          input.keyboard.press(SDL_SCANCODE_PAGEUP));
    input.bind(Input::FLAPS_DOWN,        input.keyboard.press(SDL_SCANCODE_PAGEDOWN));
    input.bind(Input::BRAKE,             input.keyboard.press(SDL_SCANCODE_F));
    input.bind(Input::BRAKE,             input.keyboard.press(SDL_SCANCODE_PERIOD));

    input.bind(Input::LOOK_X,            input.keyboard.hold(SDL_SCANCODE_LSHIFT), input.mouse.analog(0));
    input.bind(Input::LOOK_Y,            input.keyboard.hold(SDL_SCANCODE_LSHIFT), input.mouse.analog(1));
    input.bind(Input::LOOK_CENTER,       input.keyboard.press(SDL_SCANCODE_KP_5));
    input.bind(Input::VIEW_PREV,         input.keyboard.press(SDL_SCANCODE_HOME));
    input.bind(Input::VIEW_NEXT,         input.keyboard.press(SDL_SCANCODE_END));

    input.bind(Input::MENU_UP,           input.keyboard.press(SDL_SCANCODE_W));
    input.bind(Input::MENU_DOWN,         input.keyboard.press(SDL_SCANCODE_S));
    input.bind(Input::MENU_LEFT,         input.keyboard.press(SDL_SCANCODE_A));
    input.bind(Input::MENU_RIGHT,        input.keyboard.press(SDL_SCANCODE_D));
    input.bind(Input::MENU_OK,           input.keyboard.press(SDL_SCANCODE_SPACE));
    input.bind(Input::MENU_CANCEL,       input.keyboard.press(SDL_SCANCODE_LCTRL));
    
    input.bind(Input::WEAPON_FIRE1,      input.mouse.hold(0));
    input.bind(Input::WEAPON_FIRE2,      input.mouse.hold(2));
    input.bind(Input::WEAPON_NEXT,       input.mouse.press(3));
    input.bind(Input::WEAPON_PREV,       input.mouse.press(4));

    input.bind(Input::FULLSCREEN_TOGGLE, input.keyboard.hold(SDL_SCANCODE_LALT), input.keyboard.press(SDL_SCANCODE_RETURN));
    input.bind(Input::FULLSCREEN_TOGGLE, input.keyboard.hold(SDL_SCANCODE_RALT), input.keyboard.press(SDL_SCANCODE_RETURN));
    input.bind(Input::PAUSE,             input.keyboard.press(SDL_SCANCODE_F1));
    input.bind(Input::QUIT,              input.keyboard.press(SDL_SCANCODE_ESCAPE));

    // input.load("input.cfg");
    input.save("input.cfg");

    this->init_controllers();
}

void
InputEngine::update_tick(void)
{
}

void
InputEngine::update_frame(void)
{
    this->update_keyboard();
    this->update_mouse();
    this->update_controllers();
}

void
InputEngine::update_keyboard(void)
{
    input.keyboard.clear();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
                // Key repeats
                input.keyboard.repeats.push_back(event.key.keysym.scancode);

                if (!input.keyboard.button_data[event.key.keysym.scancode])
                {
                    // Key presses (ignore repeats)
                    input.keyboard.presses.push_back(event.key.keysym.scancode);
                }
                input.keyboard.button_data[event.key.keysym.scancode] = true;
                input.keyboard.button = event.key.keysym.sym;

                break;


            case SDL_KEYUP:
                // Key releases
                input.keyboard.releases.push_back(event.key.keysym.scancode);
                input.keyboard.button_data[event.key.keysym.scancode] = false;

                break;


            case SDL_MOUSEWHEEL:
                input.mouse.presses.push_back(4 + (event.wheel.y < 0));

                break;


            case SDL_QUIT:
                throw 666;
        }
    }
}

void
InputEngine::update_mouse(void)
{
    input.mouse.clear();

    int
        x, y,
        old_x      = input.mouse.x,
        old_y      = input.mouse.y,
        old_button = input.mouse.button;
    
    Uint32 button_flags = SDL_GetMouseState(&x, &y);
    
    input.mouse.x = input.mouse.axis_data[0] = x;
    input.mouse.y = input.mouse.axis_data[1] = y;
    
    bool button[3];
    button[0] = (button_flags & SDL_BUTTON(SDL_BUTTON_LEFT));
    button[1] = (button_flags & SDL_BUTTON(SDL_BUTTON_MIDDLE));
    button[2] = (button_flags & SDL_BUTTON(SDL_BUTTON_RIGHT));
    
    for (int i = 0; i < 3; ++i)
    {
        if (input.mouse.button_data[i] != button[i])
        {
            input.mouse.button_data[i] = button[i];
            if (button[i])
            {
                // Mouse button presses
                input.mouse.presses.push_back(i);
                input.mouse.button = i;
            }
            else
            {
                // Mouse button releases
                input.mouse.releases.push_back(i);
            }
        }
    }
    
    if (SDL_GetRelativeMouseMode())
    {
        SDL_GetRelativeMouseState(&x, &y);
        input.mouse.diff.x = x;
        input.mouse.diff.y = y;
    }
    else
    {
        input.mouse.diff.x = input.mouse.x - old_x;
        input.mouse.diff.y = input.mouse.y - old_y;
    }
    
    input.mouse.diff.button = input.mouse.button - old_button;
}

void
InputEngine::update_controllers(void)
{
    for (int j = 0; j < input.joysticks; ++j)
    {
        input.joystick[j].clear();
    }

    int n = 0;
    for (Controllers::iterator i = controllers.begin();
        i != controllers.end(); ++i, ++n)
    {
        Input::Device &controller = input.joystick[n];
        SDL_GameControllerButton button[15] = {
            /* 01 */ SDL_CONTROLLER_BUTTON_A,
            /* 02 */ SDL_CONTROLLER_BUTTON_B,
            /* 03 */ SDL_CONTROLLER_BUTTON_X,
            /* 04 */ SDL_CONTROLLER_BUTTON_Y,
            /* 05 */ SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
            /* 06 */ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
            /* 07 */ SDL_CONTROLLER_BUTTON_LEFTSTICK,
            /* 08 */ SDL_CONTROLLER_BUTTON_RIGHTSTICK,
            /* 09 */ SDL_CONTROLLER_BUTTON_START,
            /* 10 */ SDL_CONTROLLER_BUTTON_BACK,
            /* 11 */ SDL_CONTROLLER_BUTTON_GUIDE,
            /* 12 */ SDL_CONTROLLER_BUTTON_DPAD_UP,
            /* 13 */ SDL_CONTROLLER_BUTTON_DPAD_DOWN,
            /* 14 */ SDL_CONTROLLER_BUTTON_DPAD_LEFT,
            /* 15 */ SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
        };
        
        for (int b = 1; b <= 15; ++b)
        {
            controller.button_data[b]
                = SDL_GameControllerGetButton(*i, button[b - 1]);
        }
        
        // Left stick X
        controller.x =
        controller.axis_data[1] = SDL_GameControllerGetAxis(*i,
            SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;

        // Left stick Y
        controller.y =
        controller.axis_data[2] = SDL_GameControllerGetAxis(*i,
            SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;

        // Right stick X
        controller.axis_data[3] = SDL_GameControllerGetAxis(*i,
            SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;

        // Right stick Y
        controller.axis_data[4] = SDL_GameControllerGetAxis(*i,
            SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;

        // Left trigger
        controller.axis_data[5] = SDL_GameControllerGetAxis(*i,
            SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;

        // Right trigger
        controller.axis_data[6] = SDL_GameControllerGetAxis(*i,
            SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
    }
    
    for (Joysticks::iterator i = joysticks.begin();
        i != joysticks.end(); ++i, ++n)
    {
        Input::Device &joystick = input.joystick[n];
        for (int axis = 0; axis < input.joystick[n].axes; ++axis)
        {
            joystick.axis_data[axis]
                = SDL_JoystickGetAxis(*i, axis) / 32767.0f;
        }
        
        for (int button = 0; button < joystick.buttons; ++button)
        {
            bool pressed = SDL_JoystickGetButton(*i, button);
            if (pressed && joystick.button_data[button] == false)
            {
                joystick.presses.push_back(button);
            }
            else if (!pressed && joystick.button_data[button] == true)
            {
                joystick.releases.push_back(button);
            }
            joystick.button_data[button] = pressed;
        }
    }
}

InputEngine::InputEngine()
{
    this->register_module(this);
}

void
InputEngine::init_controllers(void)
{
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    {
        this->log("(!) Failed to initialize controller support: ", SDL_GetError());
        return;
    }
    
    SDL_GameControllerAddMappingsFromFile("../data/gamecontrollerdb.txt");

    for (int i = SDL_NumJoysticks() - 1; i >= 0; --i)
    {
        if (SDL_IsGameController(i))
        {
            SDL_GameController *controller = SDL_GameControllerOpen(i);
            
            if (controller != NULL)
            {
                this->log("Controller %s detected" , SDL_GameControllerName(controller));
                controllers.push_back(controller);
            }
            else
            {
                this->log("(!) Failed to open controller: ", SDL_GetError());
            }
        }
        else
        {
            SDL_Joystick *joystick = SDL_JoystickOpen(i);
            
            if (joystick != NULL)
            {
                this->log("Joystick %s detected" , SDL_JoystickName(joystick));
                joysticks.push_back(joystick);
            }
            else
            {
                this->log("(!) Failed to open joystick: ", SDL_GetError());
            }
        }
    }
    
    input.joysticks = controllers.size() + joysticks.size();
    input.joystick  = (input.joysticks)
        ? new Input::Device[input.joysticks]
        : NULL;
    
    switch (input.joysticks)
    {
        case 0:
            this->log("No controller detected");
            break;
        
        case 1:
            this->log("1 controller detected");
            break;
        
        default:
            this->log("%i controllers detected", input.joysticks);
    }

    input.joysticks = 0;
    for (Controllers::iterator i = controllers.begin();
        i != controllers.end(); ++i)
    {
        input.joystick[input.joysticks++].init(
            SDL_GameControllerName(*i), 16, 7);
    }
    
    for (Joysticks::iterator i = joysticks.begin();
        i != joysticks.end(); ++i)
    {
        input.joystick[input.joysticks++].init(
            SDL_JoystickName(*i),
            SDL_JoystickNumButtons(*i),
            SDL_JoystickNumAxes(*i)
        );
    }
}

InputEngine::~InputEngine()
{
    this->log(this->get_module_name());
    
    for (Controllers::iterator i = controllers.begin();
        i != controllers.end(); ++i)
    {
        SDL_GameControllerClose(*i);
    }
    
    for (Joysticks::iterator i = joysticks.begin();
        i != joysticks.end(); ++i)
    {
        SDL_JoystickClose(*i);
    }
}

Input::Input()
{
    this->joysticks = 0;
    this->joystick  = NULL;
}

Input::~Input()
{
    delete[] this->joystick;
}

Input *
Input::unbind(Action action)
{
    this->binding[action].clear();
    return this;
}

Input *
Input::bind(Action action, Event event)
{
    Input::Signal s;
    
    s.event[0] = event;
    s.event[1].device = NULL;

    this->binding[action].push_back(s);

    return this;
}

Input *
Input::bind(Action action, Event event0, Event event1)
{
    Input::Signal s;
    
    s.event[0] = event0;
    s.event[1] = event1;
    s.event[2].device = NULL;

    this->binding[action].push_back(s);

    return this;
}

Input *
Input::bind(Action action, Event event0, Event event1, Event event2)
{
    Input::Signal s;
    
    s.event[0] = event0;
    s.event[1] = event1;
    s.event[2] = event2;
    s.event[3].device = NULL;

    this->binding[action].push_back(s);

    return this;
}

float
Input::operator[](Action action)
{
    Binding *alternatives = &this->binding[action];
    float result = 0.0f;
    
    for (Input::Binding::iterator i = alternatives->begin();
        i != alternatives->end(); ++i)
    {
        float output = 1.0f;
        for (int e = 0; i->event[e].device != NULL; ++e)
        {
            float f;
            Input::Device *device = i->event[e].device;
            switch (i->event[e].type)
            {
                case ANALOG:
                    f = (*device)(i->event[e].button);
                    output = f * (output * f != 0.0f);
                    break;
                
                case HOLD:
                    output *= (*device)[i->event[e].button];
                    break;
                
                case PRESS:
                    output *= (std::find(
                        device->presses.begin(), device->presses.end(),
                        i->event[e].button) != device->presses.end());
                    break;

                case RELEASE:
                    output *= (std::find(
                        device->releases.begin(), device->releases.end(),
                        i->event[e].button) != device->releases.end());
                    break;

                case TYPE:
                    output *= (std::find(
                        device->repeats.begin(), device->repeats.end(),
                        i->event[e].button) != device->repeats.end());
                    break;
            }
        }

        if (output && std::abs(output) > std::abs(result))
        {
            result = output;
        }
    }
    
    return result;
}

Input *
Input::save(const char *filename)
{
    _init_names();

    core::Config serialized;

    for (int action = 0; action < MAX_BINDINGS; ++action)
    {
        core::Config *action_name = _actions.search(action);
        if (action_name == NULL)
        {
            continue;
        }
        
        Binding *alternatives = &this->binding[action];
        
        for (Input::Binding::iterator i = alternatives->begin();
            i != alternatives->end(); ++i)
        {
            core::Config *combo = new core::Config();
            
            for (int e = 0; i->event[e].device != NULL; ++e)
            {
                core::Config *trigger  = _types.search(i->event[e].type);
                (*combo)[e]["value"]   = i->event[e].button;
                (*combo)[e]["device"]  = i->event[e].device->id;
                (*combo)[e]["trigger"] = (trigger != NULL)
                    ? trigger->key()
                    : "unknown";
            }
            
            serialized
                ["actions"]
                    [action_name->key()]
                        [core::Config::NEXT] = *combo;

            delete combo;
        }
    }
    
    serialized.save(filename);
    
    return this;
}

#include "../engine.h"
Input *
Input::load(const char *filename)
{
    _init_names();

    core::Config serialized;
    serialized.load(filename);
    
    if (serialized["actions"].exists())
    {
        for (int action = 0; action < MAX_BINDINGS; ++action)
        {
            this->unbind((Action)action);

            core::Config *action_name = _actions.search(action);
            if (!action_name)
            {
                continue;
            }
            
            core::Config *alternatives
                = &serialized["actions"][action_name->key()];

            if (!alternatives->exists())
            {
                continue;
            }

            for (core::Config::Value::iterator i = alternatives->begin();
                i != alternatives->end(); ++i)
            {
                core::Config *combo = i->second;
                int e;
                Input::Signal s;
        
                for (e = 0; e < combo->count(); ++e)
                {
                    s.event[e].type    = (Type)
                        _types[(*combo)[e]["trigger"].string()].integer();
                    s.event[e].button  = (*combo)[e]["value"].integer();
                    const char *device = (*combo)[e]["device"].string();
                    if (strcmp(this->mouse.id, device) == 0)
                    {
                        s.event[e].device = &this->mouse;
                    }
                    else if (strcmp(this->keyboard.id, device) == 0)
                    {
                        s.event[e].device = &this->keyboard;
                    }
                    else
                    {
                        s.event[e].device = NULL;
                        for (int j = 0; j < this->joysticks; ++i)
                        {
                            if (strcmp(this->joystick[j].id, device) == 0)
                            {
                                s.event[e].device = &this->joystick[j];
                                break;
                            }
                        }
                    }
                }
                
                if (e > 0)
                {
                    s.event[e].device = NULL;
                    this->binding[action].push_back(s);
                }
            }
        }
    }

    return this;
}
        
Input::Device::Device()
{
    this->id          = NULL;
    
    this->button_data = NULL;
    this->buttons     = 0;

    this->axis_data   = NULL;
    this->axes        = 0;

    this->presses.reserve(8);
    this->releases.reserve(8);
    this->repeats.reserve(8);
    
    this->button = 0;
    this->x      = 0.0f;
    this->y      = 0.0f;
}

Input::Device::~Device()
{
    delete[] this->id;
    delete[] this->button_data;
    delete[] this->axis_data;
}

void
Input::Device::init(const char *id, int buttons, int axes)
{
    core::str::set(this->id, core::str::dup(id));
    
    delete[] this->button_data;
    this->button_data = new bool[buttons];
    this->buttons     = buttons;
    for (int i = 0; i < buttons; ++i)
    {
        this->button_data[i] = false;
    }

    delete[] this->axis_data;
    this->axis_data   = new float[axes];
    this->axes        = axes;
    for (int i = 0; i < axes; ++i)
    {
        this->axis_data[i] = 0.0f;
    }
    
}

bool
Input::Device::operator[](int button)
{
    return button >= 0
        && button < this->buttons
        && this->button_data[button];
}

float
Input::Device::operator()(int axis)
{
    return (axis >= 0 && axis < this->axes)
        ? this->axis_data[axis]
        : 0.0f;
}

void
Input::Device::clear(void)
{
    this->presses.clear();
    this->releases.clear();
    this->repeats.clear();
    this->button = 0;
}

Input::Event
Input::Device::type(int button)
{
    Input::Event event;
    event.type   = Input::TYPE;
    event.device = this;
    event.button = button;
    
    return event;
}

Input::Event
Input::Device::press(int button)
{
    Input::Event event;
    event.type   = Input::PRESS;
    event.device = this;
    event.button = button;
    
    return event;
}

Input::Event
Input::Device::hold(int button)
{
    Input::Event event;
    event.type   = Input::HOLD;
    event.device = this;
    event.button = button;
    
    return event;
}

Input::Event
Input::Device::release(int button)
{
    Input::Event event;
    event.type   = Input::RELEASE;
    event.device = this;
    event.button = button;
    
    return event;
}

Input::Event
Input::Device::analog(int axis)
{
    Input::Event event;
    event.type   = Input::ANALOG;
    event.device = this;
    event.button = axis;
    
    return event;
}

bool
Input::Device::was_pressed(int button)
{
    return std::find(this->presses.begin(), this->presses.end(), button)
        != this->presses.end();
}

bool
Input::Device::was_released(int button)
{
    return std::find(this->releases.begin(), this->releases.end(), button)
        != this->releases.end();
}
