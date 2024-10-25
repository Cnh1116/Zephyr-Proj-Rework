
#ifndef PROJECTILES_HPP
#define PROJECTILES_HPP

#include <SDL.h>

#include <iostream>
#include <vector>
//#include "Animation.hpp"

class Graphics;


class Projectile
{
    public:
        // Constructors
        Projectile(const SDL_Rect& dest_rect, float projectile_speed, float projectile_damage, bool player_projectile_flag,
            std::string main_texture_key, std::vector<SDL_Rect> main_frames, std::string impact_texture_key, std::vector<SDL_Rect> impact_frames, Uint32 frame_cooldown_ms, bool animation_replayable, bool shift_impact);
        ~Projectile();

        // Setters and Getters
        SDL_Rect* GetDstRect();
        std::string GetTextureKey();
        Uint32 GetFrameTime();
        Uint32 GetLastFrameStart();
        SDL_Rect* GetFrame();
        bool IsReplayable();
        int NumOfFrames();
        const char* GetState();
        SDL_Rect* GetCollisionRect();
        const char* GetSoundEffectImpact();

        //Other Functions
        void AdvanceFrame();
        void UpdateState(const char* state);
        
        
        // Override Functions
        virtual void MoveProjectile() = 0;
        virtual void Update() = 0;
        

        // Stats   
        float speed;
        float damage;

        // Other vars
        const char* state;

        // Texture Stuff
        std::vector<SDL_Rect> main_frames;
        std::vector<SDL_Rect> impact_frames;
        std::vector<SDL_Rect> current_frames;
        
        std::string main_texture_key;
        std::string impact_texture_key;
        std::string current_texture_key;

        const char* sound_effect_impact;
        
        int current_frame_index;
        SDL_Rect dest_rect;
        SDL_Rect collision_rect;
        Uint32 frame_time_ms;
        Uint32 last_frame_time;
        
        int BASE_SPRITE_SIZE = 32;
        int pixel_scale;
        bool player_projectile;
        bool shift_impact;
        bool animation_replayable;

    private:
        //Animation main_animation;
        
};

class PrimaryFire : public Projectile 
{
    public:
        PrimaryFire(const SDL_Rect& rect, float projectile_speed, float projectile_damage, int PIXEL_SCALE);
        void MoveProjectile() override;
        void Update() override;
};

class SecondaryFire : public Projectile 
{
    public:
        SecondaryFire(const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE);
        void MoveProjectile() override;
        void Update() override;
    
};

class IceShard : public Projectile 
{
    public:
        IceShard(const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE, float damage);
        void MoveProjectile() override;
        void Update() override;

};
class LightningBall : public Projectile 
{
    public:
        LightningBall(const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE, float damage, int player_x, int player_y);
        void MoveProjectile() override;
        void Update() override;
    private:
        int delta_x, delta_y;
        float direction_x, direction_y;


};

#endif

