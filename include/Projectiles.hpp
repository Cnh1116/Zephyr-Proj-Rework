
#ifndef PROJECTILES_HPP
#define PROJECTILES_HPP

#include <SDL.h>

#include <iostream>
#include <vector>
#include "Animation.hpp"

class Graphics;
class Animation;
class AnimationManager;

class Projectile
{
protected:
	    AnimationManager& animation_manager;
        int sound_effect_noise;
    public:
        // Constructors
        Projectile(AnimationManager& animation_manager, 
                    const SDL_Rect& dest_rect, 
                    float projectile_speed, 
                    float projectile_damage, 
                    bool player_projectile_flag, 
                    bool shift_impact,
                    bool shiny);
        ~Projectile();

        // Setters and Getters
        SDL_Rect* GetDstRect();
        const char* GetState();
        SDL_Rect* GetCollisionRect();
        std::string GetSoundEffectImpact();
		bool GetSoundPlayed() { return sound_played; }
		void SetSoundPlayed(bool played) { sound_played = played; }
        std::string GetPrintableDamage();
        virtual int GetSoundEffectImpactNoise() const { return sound_effect_noise; }
        

        //Other Functions
        void UpdateState(const char* state);
        
        
        // Override Functions
        virtual void MoveProjectile() = 0;
        virtual void Update() = 0;
        virtual void Draw(SDL_Renderer* renderer, bool collision_box_flag) = 0;

        //Shiny Flag
        bool shiny;
        

        // Stats   
        float speed;
        float damage;

        // Other vars
        const char* state;



        // ANIMATIONS
        std::vector<std::unique_ptr<Animation>> overlay_animations;
        std::unique_ptr<Animation> current_animation;

        std::string sound_effect_impact;
        

        SDL_Rect dest_rect;
        SDL_Rect collision_rect;

        
        int BASE_SPRITE_SIZE = 32;
        int pixel_scale;
        bool player_projectile;
        bool shift_impact;
        //bool animation_replayable;

    private:
		bool sound_played = false;
        
};

class PrimaryFire : public Projectile 
{
    public:
        PrimaryFire(AnimationManager& animation_manager, const SDL_Rect& rect, float projectile_speed, float projectile_damage, int PIXEL_SCALE, bool critical);
        void MoveProjectile() override;
        void Update() override;
        void Draw(SDL_Renderer* renderer, bool collision_box_flag) override;
        bool critical;

};

class SecondaryFire : public Projectile 
{
    public:
        SecondaryFire(AnimationManager& animation_manager, const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE);
        void MoveProjectile() override;
        void Update() override;
        void Draw(SDL_Renderer* renderer, bool collision_box_flag) override;


    
};

class IceShard : public Projectile 
{
    public:
        IceShard(AnimationManager& animation_manager, const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE, float damage, bool shiny);
        void MoveProjectile() override;
        void Update() override;
        void Draw(SDL_Renderer* renderer, bool collision_box_flag) override;


};
class LightningBall : public Projectile 
{
    public:
        LightningBall(AnimationManager& animation_manager, const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE, float damage, int player_x, int player_y, bool shiny);
        void MoveProjectile() override;
        void Update() override;
        void Draw(SDL_Renderer* renderer, bool collision_box_flag) override;
    private:
        double direction_x, direction_y;
        double position_x = dest_rect.x;
        double position_y = dest_rect.y;

};

class LightningStrike : public Projectile
{
public:
    LightningStrike(AnimationManager& animation_manager, const SDL_Rect& dest_rect, int PIXEL_SCALE, float damage, bool right_flag, bool shiny);
    void MoveProjectile() override;
    void Update() override;
    void Draw(SDL_Renderer* renderer, bool collision_box_flag) override;
    bool right_flag;

};

#endif

