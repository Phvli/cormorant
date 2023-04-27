#include "sound.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_Mixer.h>

#include "../util/file.h"
#include "../util/string.h"

#define CROSSFADE_DURATION 2000
#define FADEOUT_DURATION   2500

#define SOUND_QUEUE_LEN 128
#define SOUND_DIRECTORY DATA_DIRECTORY "audio/effects/"
#define MUSIC_DIRECTORY DATA_DIRECTORY "audio/soundtrack/"

using namespace core;

#include "../engine.h"

typedef
    std::map<core::Config::Hash, Mix_Chunk *>
    SoundLibrary;

static struct
{
    Mix_Chunk     *chunk;
    float          volume;
    unsigned long  ticks;
} sound_queue[SOUND_QUEUE_LEN];

void
SoundEngine::update_tick(void)
{
}

void
SoundEngine::update_frame(void)
{
    // play delayed sounds
    for (int i = 0; i < SOUND_QUEUE_LEN; ++i)
    {
        if (sound_queue[i].chunk != NULL && sound_queue[i].ticks <= this->ticks)
        {
            int channel = Mix_PlayChannel(-1, sound_queue[i].chunk, 0);
            
            if (channel != -1)
            {
                Mix_Volume(channel,
                    this->config["audio"]["volume"]["sound"].percentage(100)
                    * sound_queue[i].volume * MIX_MAX_VOLUME);
            }
            else
            {
                this->log("(!) Failed to play sound: %s", Mix_GetError());
            }
            
            sound_queue[i].chunk = NULL;
        }
        
    }

    // play next track
    if (this->music_theme != NULL && Mix_PlayingMusic() == 0)
    {
        this->play_next_track();
    }
}

SoundEngine::SoundEngine() :
    music_theme(music_theme_mutable)
{
    this->register_module(this);
}

void
SoundEngine::initialize_module()
{
    this->music_theme_mutable = NULL;
    this->music_data          = NULL;

    for (int i = 0; i < SOUND_QUEUE_LEN; ++i)
    {
        sound_queue[i].chunk = NULL;
    }

    if (!SDL_WasInit(SDL_INIT_AUDIO))
    {
        SDL_Init(SDL_INIT_AUDIO);
    }
    
    long mix_init_flags = MIX_INIT_MP3 | MIX_INIT_OGG;
    if (Mix_Init(mix_init_flags) != mix_init_flags)
    {
        this->log("(!) Mix_Init failed: %s", SDL_GetError());
        throw 666;
    }
    
    if (Mix_OpenAudio(
        this->config["audio"]["frequency"].integer(22050),
        MIX_DEFAULT_FORMAT,
        this->config["audio"]["stereo"].boolean(true) ? 2 : 1,
        this->config["audio"]["sample_size"].integer(2096)
    ))
    {
        this->log("(!) Mix_Openaudio failed: %s", SDL_GetError());
        throw 666;
    }
    
    sound_data = (void *)(new SoundLibrary());
    
    this->reload_music_library();
}

SoundEngine::~SoundEngine()
{
    this->log(this->get_module_name());

    // stop sound effects
    Mix_HaltChannel(-1);
    
    SoundLibrary *sounds = (SoundLibrary *)this->sound_data;
    for (SoundLibrary::iterator sfx = sounds->begin();
    sfx != sounds->end(); ++sfx)
    {
        Mix_FreeChunk(sfx->second);
    }
    delete sounds;
    
    // stop playing music
    Mix_HaltMusic();
    if (this->music_data)
    {
        Mix_FreeMusic((Mix_Music *)this->music_data);
    }

    delete[] this->music_theme_mutable;
    
    // deinitialize sound library
    Mix_CloseAudio();
}

void
SoundEngine::reload_music_library(void)
{
    // flush sound library
    SoundLibrary *sounds = (SoundLibrary *)this->sound_data;
    for (SoundLibrary::iterator sfx = sounds->begin();
    sfx != sounds->end(); ++sfx)
    {
        Mix_FreeChunk(sfx->second);
    }
    sounds->clear();

    // preload sound files
    core::Dir wav(SOUND_DIRECTORY,
        core::Dir::FILES | core::Dir::OMIT_EXTENSION,
        "wav");
    
    for (int i = 0; wav[i] != NULL; ++i)
    {
        char *name     = core::str::dup(wav[i]);
        char *filename = core::str::format(SOUND_DIRECTORY "%s.wav", name);
        
        core::Config::Hash hash = core::Config::get_hash(core::str::lcase(name));
        
        if (sounds->find(hash) == sounds->end())
        {
            Mix_Chunk *chunk = Mix_LoadWAV(filename);
            
            if (chunk == NULL)
            {
                this->log("(!) Failed to preload %s (%s)", filename, Mix_GetError());
            }
            else
            {
                sounds->insert(std::make_pair(hash, chunk));
            }
        }
        
        delete[] name;
        delete[] filename;
    }

    // music library
    this->playlist = this->music_library;
    this->music_library.empty();
    core::Dir theme(MUSIC_DIRECTORY, core::Dir::DIRECTORIES);
    for (int i = 0; theme[i] != NULL; ++i)
    {
        char *theme_path = core::str::cat(theme.path, theme[i]);
        core::Dir song(theme_path, core::Dir::FILES | core::Dir::FULL_PATH, "ogg");

        for (int j = 0; song[j] != NULL; ++j)
        {
            this->music_library[theme[i]][core::Config::NEXT] = song[j];
        }
        
        song.filter(theme_path, "mp3");
        for (int j = 0; song[j] != NULL; ++j)
        {
            this->music_library[theme[i]][core::Config::NEXT] = song[j];
        }
        delete[] theme_path;
    }

    this->playlist = this->music_library;
}

void
SoundEngine::play_sound_at(const char *sound, float distance, float volume)
{
    static const float min_damping_dist   = 25.0f;
    static const float damping_multiplier = .08f;
    
    if (distance >= min_damping_dist)
    {
        float f = min_damping_dist / (min_damping_dist + (distance - min_damping_dist) * damping_multiplier);
        volume = volume * f * f;
    }

    // speed of sound: 3.432 m/s
    this->play_sound(sound, distance / .3432f, volume);
}

void
SoundEngine::play_sound(const char *sound, unsigned long delay, float volume)
{
    SoundLibrary *sounds       = (SoundLibrary *)this->sound_data;
    core::Config::Hash hash    = core::Config::get_hash(sound);
    SoundLibrary::iterator sfx = sounds->find(hash);
    
    if (sfx == sounds->end())
    {
        this->log("(!) Unknown sound effect: '%s'", sound);
        return;
    }

    for (int i = 0; i < SOUND_QUEUE_LEN; ++i)
    {
        if (sound_queue[i].chunk == NULL)
        {
            sound_queue[i].chunk  = sfx->second;
            sound_queue[i].volume = volume;
            sound_queue[i].ticks  = this->ticks + delay;
            
            if (delay)
            {
                this->log("Sound effect: %s (delayed %0.1f s)", sound, delay / 1000.0f);
            }
            else
            {
                this->log("Sound effect: %s", sound);
            }

            return;
        }
    }
    
    this->log("(!) Failed to play sound effect %s: queue full", sound);
}

void
SoundEngine::play_music(const char *theme)
{
    if (theme != NULL)
    {
        if (Mix_PlayingMusic())
        {
            Mix_FadeOutMusic(CROSSFADE_DURATION);
        }

        // start playing music / change track
        if (this->music_theme == NULL || strcmp(this->music_theme, theme))
        {
            this->log("Music theme: %s", theme);

            if (this->playlist[theme].is_empty())
            {
                this->playlist[theme] = this->music_library[theme];
            }
            
            core::str::set(this->music_theme_mutable,
                (this->playlist[theme].is_empty())
                    ? NULL
                    : core::str::dup(theme)
            );

            // on unknown theme, just try to play the track
            if (this->music_theme == NULL)
            {
                char *filename = core::str::cat(MUSIC_DIRECTORY, theme);
                try
                {
                    this->play_music_file(filename);
                }
                catch (int e)
                {
                    delete[] filename;
                    throw e;
                }
            }
        }
    }
    else if (this->music_theme != NULL)
    {
        this->log("Stopping music");
        
        // stop playing music
        if (Mix_PlayingMusic())
        {
            Mix_FadeOutMusic(FADEOUT_DURATION);
        }
        
        core::str::set(this->music_theme_mutable, NULL);
    }
}

void
SoundEngine::play_next_track(void)
{
    if (this->playlist[this->music_theme].is_empty())
    {
        // all tracks played => restore all tracks for this theme
        this->playlist[this->music_theme]
            = this->music_library[this->music_theme];
    }
    
    this->log("Now playing:");
    core::Config *song
        = this->playlist[this->music_theme][core::Config::RANDOM].pop();
    
    if (song)
    {
        const char *filename = song->string();

        try
        {
            this->play_music_file(filename);
        }
        catch (int)
        {
            delete song;
            throw 666;
        }
        delete song;
    }
    else
    {
        this->log("<(empty playlist)");
        core::str::set(this->music_theme_mutable, NULL);
    }
}

void
SoundEngine::set_music_volume(float volume)
{
    this->config["audio"]["volume"]["music"] = volume * 100.0f;
    Mix_VolumeMusic(this->config["audio"]["volume"]["music"].percentage(100) * MIX_MAX_VOLUME);
}

void
SoundEngine::play_music_file(const char *filename)
{
    try
    {
        Mix_VolumeMusic(this->config["audio"]["volume"]["music"].percentage(100) * MIX_MAX_VOLUME);
        
        this->tags.empty();
        
        if (strstr(filename, ".ogg") != NULL ||
            strstr(filename, ".OGG") != NULL)
        {
            this->load_OGG_metadata(filename);
        }
        
        if (this->tags["TITLE"].exists())
        {
            if (this->tags["ARTIST"].exists())
            {
                this->log("<%s - %s", this->tags["ARTIST"].string(), this->tags["TITLE"].string());
            }
            else
            {
                this->log("<%s", this->tags["TITLE"].string());
            }
        }
        else
        {
            this->log("<%s", filename);
        }

        Mix_Music *music = Mix_LoadMUS(filename);
        
        if (music == NULL || Mix_PlayMusic(music, 1))
        {
            throw 666;
        }
        
        Mix_FreeMusic((Mix_Music *)this->music_data);
        this->music_data = music;
    }
    catch (int)
    {
        this->log("(!) Failed to load and play %s", filename);
        core::str::set(this->music_theme_mutable, NULL);
    }
}

void
SoundEngine::load_OGG_metadata(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        throw 666;
    }
    
    try
    {
        for (int page_number = 0; page_number < 2; ++page_number)
        {
            // read OGG page header and check for magic word
            char header[7];
            fread(header, 5, 1, file);
            if (strncmp("OggS", header, 5))
                throw 666;
            
            // skip to and read segment table (= page size)
            fseek(file, 21, SEEK_CUR);
            unsigned int  segment_count = 0;
            unsigned char segment_table[255];
            fread(&segment_count, 1, 1, file);
            fread(segment_table, segment_count, 1, file);

            // read and validate vorbis header from the first segment
            fread(header, 7, 1, file);
            
            // validate vorbis page: type must be 0x01, 0x03 (and 0x05)
            // respectively, followed by "vorbis"
            if (header[0] != page_number * 2 + 1 || strncmp(header + 1, "vorbis", 6))
                throw 666;
            
            if (page_number)
            // ogg vorbis metadata is located on the second page:
            {
                // vendor-specific string
                unsigned int len = 0;
                fread(&len, 4, 1, file);
                fseek(file, len, SEEK_CUR);

                // number of tags ('comment items')
                unsigned int tag_count = 0;
                fread(&tag_count, 4, 1, file);
                
                len = 0;
                // read each individual tag
                while (tag_count--)
                {
                    // read tag
                    fread(&len, 4, 1, file);
                    char *tag = new char[len + 1];
                    fread(tag, len, 1, file);
                    tag[len] = '\0';
                    
                    char *value = strchr(tag, '=');
                    if (value != NULL)
                    // tag formatted as TAG=value
                    {
                        *value = 0;
                        this->tags[tag] = value + 1;
                    }
                    else
                    // invalid formatting but read it anyways
                    {
                        this->tags[core::Config::NEXT] = tag;
                    }
                    delete[] tag;
                }
            }
            else
            // just skip the first page, nothing interesting there
            {
                int page_size = -7;
                for (unsigned int i = 0; i < segment_count; ++i)
                {
                    page_size += segment_table[i];
                }
                fseek(file, page_size, SEEK_CUR);
            }
        }
    }
    catch (int)
    {
        fclose(file);
        throw 666;
    }
    
    fclose(file);
}

const char *
SoundEngine::get_music_tag(const char *tag)
{
    core::Config *t = &this->tags[tag];
    return t->exists()
        ? t->string()
        : NULL;
}
