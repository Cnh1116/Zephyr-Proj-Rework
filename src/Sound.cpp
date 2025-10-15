#include "Sound.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include <map>
#include <algorithm>

SoundManager::SoundManager()
{
    std::cout << "[*] Initializing Audio.\n";
    if (0 != SDL_Init(SDL_INIT_AUDIO))
    {
        std::cout << "[!] SDL_Init Audio did not return 0 code.\n" << SDL_GetError() << std::endl;
        exit(1);
    }

    if(0 > Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096))
    {
        std::cout << "[!] Mix_OpenAudio did not return 0 code./n" << Mix_GetError() << std::endl;
        exit(1);
    }

    std::cout << "[*] Allocating 32 channels for sound!\n";
    Mix_AllocateChannels(32); //8 Player, 16 Enemies, 8 items
    
    // Player Sounds
    LoadSoundEffect("player_primary_fire", "../../assets/sounds/player-sounds/primary-fire.wav");
    LoadSoundEffect("player_secondary_fire", "../../assets/sounds/player-sounds/secondary-fire.wav");
    LoadSoundEffect("player_secondary_fire_impact", "../../assets/sounds/player-sounds/secondary-fire-impact.wav");
    LoadSoundEffect("player_hurt", "../../assets/sounds/player-sounds/player-hit.wav");
    LoadSoundEffect("player_shield_activate", "../../assets/sounds/player-sounds/secondary-fire-impact.wav");
    LoadSoundEffect("player_shield_hit", "../../assets/sounds/player-sounds/parry-sfx-aracde.wav");
    LoadSoundEffect("shield_activate", "../../assets/sounds/player-sounds/shield-activate.wav");
    LoadSoundEffect("dash_sound", "../../assets/sounds/player-sounds/dash.wav");
    LoadSoundEffect("player_crit", "../../assets/sounds/player-sounds/player-crit.wav");
    
    // Overlay Effect Sounds
    LoadSoundEffect("player_heal", "../../assets/sounds/items-sounds/healing.wav");    
    LoadSoundEffect("jade_drum", "../../assets/sounds/enemy-sounds/jade-drum.wav");

    // Projectile Sounds
    LoadSoundEffect("ice_shard_impact", "../../assets/sounds/enemy-sounds/ice-shard-impact.wav");
    LoadSoundEffect("lightning_ball_impact", "../../assets/sounds/enemy-sounds/electric-ball-impact.wav");

    // Item Sounds
    LoadSoundEffect("item_collection_sound", "../../assets/sounds/item-sounds/item-collection.wav");
    
    // Music Files
    music_map["first_level_song"] = "../../assets/soundsmusic/ChrisChristodoulou-TheyMightAsWellBeDeadROR2_SurvivorsoftheVoid(2022).mp3";
}

SoundManager::~SoundManager()
{
    std::cout << "[*] Turning off the SDL Audio./n";
    for (auto& pair : sound_effects_map)
    {
        Mix_FreeChunk(pair.second);
    }
    Mix_CloseAudio();
    Mix_Quit();
}



void SoundManager::PlaySound(std::string sound_map_key, int volume_percent)
{
    // Clamp volume between 0 and 100
    volume_percent = std::clamp(volume_percent, 0, 100);

    Mix_Chunk* chunk = sound_effects_map[sound_map_key];
    if (!chunk) {
        std::cerr << "[!] Null sound chunk for key: " << sound_map_key << std::endl;
        return;
    }

    Mix_VolumeChunk(chunk, (volume_percent * MIX_MAX_VOLUME) / 100);

    int channel = Mix_PlayChannel(-1, chunk, 0);  // -1 = pick first available channel
    if (channel == -1) {
        std::cerr << "[!] Failed to play sound: " << Mix_GetError() << std::endl;
    }
}

void SoundManager::PlayMusic(const char* music_key) 
{
    Mix_Music* music = Mix_LoadMUS(music_map[music_key]);
    //std::cout << "[*] Playing the sound effect " << music_map[music_key] << std::endl;
    if (!music) 
    {
        std::cerr << "[!] Failed to load MP3 file: " << Mix_GetError() << std::endl;  
    }

    std::cout << "[*] Playing Music" << music_map[music_key] << std::endl;
    // Check parameters ! loop_flag, how to pause, switch music ?
    if (-1 == Mix_PlayMusic(music, 1)) 
    {
        std::cerr << "[!] Failed to play music: " << Mix_GetError() << std::endl;
        Mix_FreeMusic(music);
        return;
    }
}

void SoundManager::LoadSoundEffect(const char* sound_key, const char* wav_file)
{
    
    // Load a WAV sound effect
    Mix_Chunk* sound_effect = Mix_LoadWAV(wav_file);
    if (!sound_effect)
    {
        std::cerr << "[!] Failed to load WAV file " << wav_file << " : " << Mix_GetError() << std::endl;
        return;
    }

    // Store the loaded sound effect in the map
    sound_effects_map[sound_key] = sound_effect;
}

void SoundManager::FadeOutMusic()
{
 
    
    if (Mix_PlayingMusic()) 
    {
        std::cout << "[*] Fading music out over " << song_fade_time_ms << " milliseconds." << std::endl;
        if (0 == Mix_FadeOutMusic(song_fade_time_ms)) 
        {
            std::cerr << "[!] Failed to fade out music: " << Mix_GetError() << std::endl;
        }
    } else {
        std::cerr << "[!] No music to fade out." << std::endl;
    }
}

