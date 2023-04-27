#ifndef _CORE_ENGINES_SOUND_H
#define _CORE_ENGINES_SOUND_H

#include "core.h"

namespace core
{
    class SoundEngine:
        virtual public EngineCore,
        public EngineInterface
    {

    public:
        SoundEngine();
        ~SoundEngine();
        
        char * const &music_theme;
        
        virtual const char *
        get_module_name(void) { return "sound"; };

        virtual void
        initialize_module(void);

        virtual void
        update_tick(void);

        virtual void
        update_frame(void);

        void
        play_sound(const char *sound, unsigned long delay = 0, float volume = 1.0f);
        
        void
        play_sound_at(const char *sound, float distance, float volume = 1.0f);
        
        void
        play_music(const char *theme);
        
        void
        set_music_volume(float volume);
        
        const char *
        get_music_tag(const char *tag);

        void
        reload_music_library(void);
        
    private:
        Config playlist, music_library, tags;
        char *music_theme_mutable;
        void *sound_data, *music_data;
        
        void
        load_OGG_metadata(const char *filename);
        
        void
        play_next_track(void);

        void
        play_music_file(const char *filename);
    };
}

#endif
