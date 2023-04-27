#ifndef _CORE_ENGINES_INPUT_H
#define _CORE_ENGINES_INPUT_H

#include "core.h"
#include <vector>

namespace core
{
    class Input
    {
    public:
        class Device;
        
        static const int MAX_BINDINGS = 128;

        typedef
            enum
            {
                PITCH,
                ROLL,
                RUDDER,
                THROTTLE,
                PITCH_UP,
                PITCH_DOWN,
                ROLL_LEFT,
                ROLL_RIGHT,
                RUDDER_LEFT,
                RUDDER_RIGHT,

                THROTTLE_UP,
                THROTTLE_DOWN,
                FLAPS_UP,
                FLAPS_DOWN,
                BRAKE,

                LOOK_X,
                LOOK_Y,
                LOOK_CENTER,
                VIEW_NEXT,
                VIEW_PREV,
                
                MENU_UP,
                MENU_DOWN,
                MENU_LEFT,
                MENU_RIGHT,
                MENU_OK,
                MENU_CANCEL,

                WEAPON_FIRE1,
                WEAPON_FIRE2,
                WEAPON_NEXT,
                WEAPON_PREV,
                
                FULLSCREEN_TOGGLE,
                PAUSE,

                QUIT
            }
            Action;

        typedef
            enum
            {
                TYPE,
                PRESS,
                HOLD,
                RELEASE,
                ANALOG
            }
            Type;

        typedef
            struct
            {
                Type    type;
                Device *device;
                int     button;
            }
            Event;
        
        class Device
        {
        public:
            Device();
            ~Device();
            
            void
            init(const char *id, int buttons, int axes);

            bool // Test for button presses
            operator[](int button);
            
            float // Test for analog axis states
            operator()(int axis);
            
            Input::Event // key down events
            press(int button);
            
            Input::Event // key up events
            release(int button);
            
            Input::Event // continuously, whenever key is pressed
            hold(int button);
            
            Input::Event // key down and key repeat events
            type(int button);
            
            Input::Event // joystick and mouse movements
            analog(int axis);
            
            void // Reset per-frame stats to defaults (= no keys pressed, no changes)
            clear(void);
            
            bool
            was_pressed(int button);
            
            bool
            was_released(int button);
            
            char  *id;
            
            bool  *button_data;
            int    buttons;

            float *axis_data;
            int    axes;
            
            int    button; // shortcut to pressed/repeated button (0 if nothing pressed)
            float  x, y;   // shortcuts to axes 0 and 1 (respectively)

            struct {
                int    button;
                float  x, y;
            } diff;
            
            std::vector<int>
                presses,
                releases,
                repeats;
        };

        Input();
        ~Input();

        Device mouse, keyboard, *joystick;
        int joysticks;

        Input *
        unbind(Action action);

        Input *
        bind(Action action, Event event);
        
        Input *
        bind(Action action, Event event0, Event event1);
        
        Input *
        bind(Action action, Event event0, Event event1, Event event2);
        
        float // Check whether an input action is triggered right now
        operator[](Action action);
        
        Input *
        save(const char *filename);
        
        Input *
        load(const char *filename);
        
    protected:
        typedef
            struct
            {
                Event event[4];
            }
            Signal;
        
        typedef
            std::vector<Signal>
            Binding;
        
        Binding binding[MAX_BINDINGS];
    };
    
    class InputEngine:
        virtual public EngineCore,
        public EngineInterface
    {

    public:
        InputEngine();
        ~InputEngine();
        
        virtual const char *
        get_module_name(void) { return "input"; };

        virtual void
        initialize_module(void);

        virtual void
        update_tick(void);

        virtual void
        update_frame(void);
    
    protected:
        void
            init_controllers(void),
            update_keyboard(void),
            update_mouse(void),
            update_controllers(void);
    };

}

extern core::Input input;

#endif
