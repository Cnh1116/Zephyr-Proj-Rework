#include <iostream>
#include <map>
#include <SDL_mixer.h>


class SoundManager
{
    public:
        SoundManager(); //Maybe give directory where sounds are ??
        ~SoundManager();
        void PlaySound(std::string current_key, int volume_percent);
        void PlayMusic(const char* song_key);
        int PlaySoundTracking(std::string current_key, int volume_percent);
        void StopSoundChannel(int sound_channel);
        void FadeOutMusic();
        

        std::map<std::string, Mix_Chunk*> sound_effects_map;
        std::map<const char*, const char*> music_map;
        

        const int PLAYER_CHANNEL = 0;
        const int ITEMS_CHANNEL = 0;
        const int PROJECTILE_CHANNEL = 0;

        

    private:
        void LoadSoundEffect(const char* sound_key, const char* wav_path);
        int song_fade_time_ms = 1000;
        
};