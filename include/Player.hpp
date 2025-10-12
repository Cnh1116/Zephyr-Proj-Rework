#ifndef PLAYER_HPP
#define PLAYER_HPP


#include <SDL.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

class Graphics;
class Animation;
class AnimationManager;
class SoundManager;
class ItemManager;
class Projectile;

class Player
{
    public:
        
        // Constructor
        Player(int PIXEL_SCALE, AnimationManager& animation_manager);

        //UPDATE
        void Update(int x_pos, int y_pos, int SCREEN_WIDTH, int SCREEN_HEIGHT, long loop_flag, Uint32 tick, SoundManager& sound_manager);
        void Draw(SDL_Renderer* renderer, bool collision_box_flag);

        // STATE
        std::string GetPlayerState();
        void UpdatePlayerState(std::string new_state);

        // LOCATION
        SDL_Rect* GetDstRect();
        SDL_Rect* GetCollRect(); 
        void SetPosition(int x, int y, int SCREEN_WIDTH, int SCREEN_HEIGHT);
        void SetImageScale(int image_scale);
        
        //HEALTH
        int GetCurrentHealth();
        int GetMaxHealth();
        void Heal(int recovery_amount, SoundManager& sound_manager);
        void Hurt(int damage, SoundManager& sound_manager);

        // STATS
        float GetCrit();
        float GetSpeed();
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
        SDL_Rect* GetSecondaryFireMarkerCollision();
        SDL_Rect* GetSecondaryFireMarkerPosition();
        SDL_Rect* GetSecondaryFireHudColl();
        void ShootSecondaryFire(std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, ItemManager* item_manager);
        bool IsFireSecondaryReady();
        
        
        // SHIELD
        SDL_Rect* GetShieldDstRect();
        SDL_Rect* GetShieldColl();
        // void UseShield();
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
        Uint32 GetIframeTime();
        Uint32 GetLastIFrameStart();
        void AdvanceIFrame();
        void SetLastIFrameTime(Uint32 last_time);
        bool IsIframesDone();

        // OLD FUNCTIONS X X X X
        // ANIMATION FUNCTIONS
        //std::string GetTextureKey();
        //Uint32 GetFrameTime();
        //Uint32 GetLastFrameStart();
        //SDL_Rect* GetFrame();
        //bool IsReplayable();
        //int NumOfFrames();
        //void SetLastFrameTime(Uint32 current_time);
        //int GetFrameIndex();
        //void AdvanceFrame();
        // //SDL_Rect* GetShieldFrame();
        //void SetFrameIndex(int index);
        //Uint32 GetShieldFrameTime();
        //Uint32 GetLastShieldFrameStart();
        //const char* GetState();
    
    private:
        //STATS
        float base_speed;
        float current_speed;
        int base_damage;
        float crit_percent = 0;
        Uint32 last_parry_heal_time = 0;
        Uint32 parry_heal_buffer = 500;

        int max_health;
        int current_health;
        
        //IMAGE STUFF --- NEEDS TO BE DEPRECIATED
        int BASE_SPRITE_SIZE = 32;
        int image_scale;
        
        // LOCATION DATA
        SDL_Rect player_dest_rect;
        SDL_Rect player_coll_rect;

        //States
        std::string state;
        bool invincible;
        
		

        // I-FRAMES
        Uint32 last_iframes_start = 0;
        Uint32 iframes_duration = 1000;

        // DASH
        Uint32 last_dash_time = 0;
        Uint32 dash_cooldown_ms = 1500;
        Uint32 dash_duration = 200;
        float dash_speed;

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
            SDL_Rect marker_col_rect;
            SDL_Rect marker_dest_rect;
            

            //HUD VARIABLES
            SDL_Rect source_rect;
            SDL_Rect hud_coll_rect;
            SDL_Rect hud_dest_rect;
            
            
            // STATS
            Uint32 last_fire_time = 0;
            Uint32 cooldown_time_ms = 1500; //ms
            float speed = 2.1;
        };
        Secondary_fire secondary_fire;

        struct Shield
        {
            SDL_Rect dest_rect;
            SDL_Rect coll_rect;
            Uint32 last_time_used = 0;
            Uint32 shield_cooldown_ms = 5000;
            bool shield_ready = true;

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
};

#endif