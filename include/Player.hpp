#ifndef PLAYER_HPP
#define PLAYER_HPP


#include <SDL.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "Collisions.hpp"

class Graphics;
class Animation;
class AnimationManager;
class SoundManager;
class ItemManager;
class Projectile;
class OverlayTextManager;

class Player
{
    public:
        
        // Constructor
        Player(int PIXEL_SCALE, AnimationManager& animation_manager);

        //UPDATE
        void Update(int SCREEN_WIDTH, int SCREEN_HEIGHT, long loop_flag, Uint32 tick, SoundManager& sound_manager);
        void Draw(SDL_Renderer* renderer, bool collision_box_flag);
        void SetInput(float dx, float dy) { input_dx = dx; input_dy = dy; };
        void AddOverlayAnimation(Animation* animation);

        // RESET
		void ResetPlayer(int SCREEN_WIDTH, int SCREEN_HEIGHT);

        // STATE
        std::string GetPlayerState();
        void UpdatePlayerState(std::string new_state);

        // LOCATION
        SDL_Rect* GetDstRect();
        Collider* GetCollShape(); 
        void SetPosition(float dx, float dy, int SCREEN_WIDTH, int SCREEN_HEIGHT);
        void Move(int SCREEN_WIDTH, int SCREEN_HEIGHT);
        void SetImageScale(int image_scale);
        
        //HEALTH
        int GetCurrentHealth();
        int GetMaxHealth();
        void Heal(int recovery_amount, SoundManager& sound_manager);
        void Hurt(int damage, SoundManager& sound_manager);

        // STATS
        float GetCrit();
        float GetSpeed();
        const float GetVX() { return vx; }
		const float GetVY() { return vy; }
        void SetVX(float new_vx) { vx = new_vx; }
        void SetVY(float new_vy) { vy = new_vy; }
        void SetSpeed(float speed);
        bool CanParryHeal();
        float GetBaseDamage();

        // PRIMARY FIRE
        void ShootPrimaryFire(std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, ItemManager* item_manager);
        bool IsFirePrimaryReady();
        

        // SECONDARY FIRE
        SDL_Rect* GetSecondaryFirePosition();
        float GetSecondaryFireSpeed();
        bool IsSecondaryFireMarkerActive();
        void SetSecondaryFireMarkerActive(bool flag);
        void SetSecondaryFireMarkerPosition();
        Collider* GetSecondaryFireMarkerCollision();
        SDL_Rect* GetSecondaryFireMarkerPosition();
        Collider* GetSecondaryFireHudColl();
        void ShootSecondaryFire(std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, ItemManager* item_manager);
        bool IsFireSecondaryReady();
        

        // SLASH
        void DoSlash(std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, bool left_flag);
        bool IsSlashReady();
        
        
        // SHIELD
        SDL_Rect* GetShieldDstRect();
        Collider* GetShieldColl();
        void DoShield(SoundManager& sound_manager, Projectile* projectile, bool render_coll_boxes, OverlayTextManager* overlay_text_manager);
        void SetShieldLastTimeUsed(Uint32 last_time_used);
        bool IsShieldReady();
        Uint32 GetShieldLastTimeUsed();
        Uint32 GetShieldCooldown();
        
        
        
        // DASH
        float GetDashSpeed();
        bool IsDashReady();
        bool IsDashDone();
        
        // ITEMS
        int GetNumItem(std::string item_name);
        void AddItem(std::string item_name);


        // I-FRAMES
        bool IsIframesDone();

        // POINTS
		void GivePoints(int points_to_add) { points += points_to_add; }
		int GetPoints() { return points; }


    
    private:
        //STATS
		float BASE_SPEED = 5.0f;
        float base_speed = BASE_SPEED;
        float current_speed = base_speed;

        // POSITION 
        float posX = 0.0f;
        float posY = 0.0f;
        float vx = 0.0f;  // velocity X
        float vy = 0.0f;  // velocity Y
		float input_dx = 0.0f;
		float input_dy = 0.0f;

        float accel = 0.03f;     // acceleration rate
        float base_accel = accel;
        float decel = 0.001f;     // deceleration rate
        float max_speed = 5.0f; // max movement speed
        
        int BASE_DAMAGE = 20.0;
        int base_damage = 20;
        float BASE_CRIT = 1.0f;
        float crit_percent = BASE_CRIT; // Percentage
        

        // HEALTH
        int BASE_HEALTH = 200;
        int max_health = 200;
        int current_health = max_health;
        Uint32 last_parry_heal_time = 0;
        Uint32 parry_heal_buffer = 500;
        
        //IMAGE STUFF --- NEEDS TO BE DEPRECIATED
        int BASE_SPRITE_SIZE = 32;
        int image_scale;
        
        // LOCATION DATA
        SDL_Rect player_dest_rect;
        Collider player_coll_shape = Collider(0, 0, 5);

        //States
        std::string state;
        bool invincible = false;
        
		
        // POINTS
		int points = 0;

        // I-FRAMES
        Uint32 last_iframes_start = 0;
        Uint32 iframes_duration = 900;

        // DASH
        Uint32 dash_cooldown_ms = 1500;
        Uint32 last_dash_time = SDL_GetTicks() - dash_cooldown_ms;
        Uint32 dash_duration = 230;
        float dash_accel = 0.3f;
        float dash_speed = 8.0;
        bool dash_overlay_added = false;
		SDL_Rect dash_overlay_dest_rect;
        float dash_overlay_angle;

        struct Primary_fire
        {
            Uint32 last_fire_time = 0;
            Uint32 cooldown_time_ms = 350; //ms
        };
        Primary_fire primary_fire;

        struct Secondary_fire
        {
            // MARKER VARIABLES
            bool marker_active;
            bool ready;
            Collider marker_col_rect = Collider({ 0, 0, 0, 0 });
            SDL_Rect marker_dest_rect;
            

            //HUD VARIABLES
            SDL_Rect source_rect;
            Collider hud_coll_rect = Collider({ 0, 0, 0, 0 });
            SDL_Rect hud_dest_rect;
            
            
            // STATS
            Uint32 cooldown_time_ms = 1500;
            Uint32 last_fire_time = SDL_GetTicks() - cooldown_time_ms;
            float speed = 1.7;
        };
        Secondary_fire secondary_fire;

        struct Slashing_Attack
        {
            Uint32 cooldown_time_ms = 1500;
            Uint32 last_fire_time = SDL_GetTicks() - cooldown_time_ms;
            int damage = 75;
			Projectile* slash_projectile = nullptr;
        };
        Slashing_Attack slashing_attack;

        struct Shield
        {
            SDL_Rect dest_rect;
            Collider coll_shape = Collider(0, 0, 5);
            Uint32 shield_cooldown_ms = 5000;
            Uint32 last_time_used = SDL_GetTicks() - shield_cooldown_ms;
            bool shield_ready = true;
			
            Uint32 shield_duration_ms = 750;
			Uint32 shield_start_time = 0;

        };
        Shield shield;

        struct Player_Items
        {
            int num_glass_toucans = 0;
            int num_garnet_shields = 0;
        };
        Player_Items player_items;
        

        // ANIMATIONS
        std::vector<std::unique_ptr<Animation>> overlay_animations;
        Animation* current_animation;
        AnimationManager& animation_manager;
        
        Animation* health_bar_animation;
        Animation* shield_bar_animation;
        Animation* dash_bar_animation;
        Animation* slash_bar_animation;
        Animation* health_bar_base_animation;
		Animation* shield_ready_animation;
        

};

#endif