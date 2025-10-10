#ifndef PLAYER_HPP
#define PLAYER_HPP


#include <SDL.h>
#include <vector>
#include <string>
#include <unordered_map>

class Graphics;
class Animation;
class AnimationManager;

class Player
{
    public:
        
        // Constructor
        Player(int PIXEL_SCALE, AnimationManager* animation_manager);

        // Actions
        // void FirePrimary();
        bool IsFirePrimaryReady();
        bool IsFireSecondaryReady();
        // void UseShield();

        //UPDATE
        void Update(int x_pos, int y_pos, int SCREEN_WIDTH, int SCREEN_HEIGHT, long loop_flag, Uint32 tick);

        void AddItem(std::string item_name);

        void Draw(SDL_Renderer* renderer, bool collision_box_flag);
        
        // Setters and Getters
        SDL_Rect* GetDstRect();
        SDL_Rect* GetCollRect();
        SDL_Rect* GetShieldFrame();
        SDL_Rect* GetShieldDstRect();
        SDL_Rect* GetShieldColl();
        Uint32 GetShieldFrameTime();
        Uint32 GetLastShieldFrameStart();
        Uint32 GetShieldLastTimeUsed();
        Uint32 GetShieldCooldown();
        void SetShieldLastTimeUsed(Uint32 last_time_used);
        void SetShieldLastFrameTime(Uint32 current_time);
        bool IsShieldReady();
        void AdvanceShieldFrame();
        int GetShieldNumFrames();
        int GetHealth();
        float GetCrit();
        float GetSpeed();
        void SetSpeed(float speed);
        void ChangeHealth(float health_modifier);
        std::string GetPlayerState();
        void UpdatePlayerState(std::string new_state);
        float GetDashSpeed();
        bool IsDashReady();
        bool IsDashDone();
        int GetNumItem(std::string item_name);
        
        //Effects Functions
        bool IsHealingEffectsActive();
        void SetHealingEffectsActive(bool flag);
        SDL_Rect* GetHealEffectsFrame();
        Uint32 GetHealEffectsFrameTime();
        Uint32 GetHealEffectsLastFrameTime();
        void SetHealEffectsLastFrameTime(Uint32 last_time);
        int GetHealEffectsFrameIndex();
        int NumHealEffectsFrames();
        void AdvanceHealEffectFrame();
        void SetHealEffectsFrame(int frame_index);


        bool IsShieldEffectsActive();
        void SetShieldReadyEffectsActive(bool flag);
        SDL_Rect* GetShieldEffectsFrame();
        Uint32 GetShieldReadyEffectsFrameTime();
        Uint32 GetShieldReadyEffectsLastFrameTime();
        void SetShieldReadyEffectsLastFrameTime(Uint32 last_time);
        int GetShieldReadyEffectsFrameIndex();
        int NumShieldReadyEffectsFrames();
        void AdvanceShieldReadyEffectFrame();
        void SetShieldReadyEffectsFrame(int frame_index);


        void SetPosition(int x, int y, int SCREEN_WIDTH, int SCREEN_HEIGHT);
        void SetImageScale(int image_scale);


        // ANIMATION FUNCTIONS
        std::string GetTextureKey();
        Uint32 GetFrameTime();
        Uint32 GetLastFrameStart();
        SDL_Rect* GetFrame();
        //bool IsReplayable();
        int NumOfFrames();
        void SetLastFrameTime(Uint32 current_time);
        int GetFrameIndex();
        void AdvanceFrame();
        void SetFrameIndex(int index);
        
        //const char* GetState();
        Uint32 GetIframeTime();
        Uint32 GetLastIFrameStart();
        void AdvanceIFrame();
        void SetLastIFrameTime(Uint32 last_time);
        

        bool IsIframesDone();
        
        SDL_Rect* GetSecondaryFirePosition();
        float GetSecondaryFireSpeed();
        
        // SECONDARY FIRE HUD
        SDL_Rect* GetSecondaryFireHudColl();
        
        // SECOND FIRE MARKER

        
        bool IsSecondaryFireMarkerActive();
        void SetSecondaryFireMarkerActive(bool flag);
        void SetSecondaryFireMarkerPosition();
        SDL_Rect* GetSecondaryFireMarkerCollision();
        SDL_Rect* GetSecondaryFireMarkerPosition();

        float GetBaseDamage();
       

    
    private:
        //STATS
        float base_speed;
        float dash_speed;
        Uint32 last_dash_time = 0;
        Uint32 dash_cooldown_ms = 1500;
        Uint32 dash_duration = 200;
        float current_speed;
        int base_damage;
        int base_health;
        float crit_percent = 0;
        
        //IMAGE STUFF
        int BASE_SPRITE_SIZE = 32;
        int image_scale;
        
        std::string current_texture_key;
        std::string main_texture_key = "player_main_texture";
        std::vector<SDL_Rect> current_frames;
        std::vector<SDL_Rect> main_frames = { {0,0,64,64}, {0,64,64,64}, {0,128,64,64}, {0,192,64,64} };
        int current_frame_index = 0;
        
        Uint32 frame_time_ms_ms = 230;
        Uint32 last_frame_time_ms = 0;


        SDL_Rect player_dest_rect;
        SDL_Rect player_coll_rect;

        //States
        std::string state;
        bool invincible;
        
        // I FRAMES
        std::vector<SDL_Rect> i_frames = { {0,0,64,64}, {0,64,64,64}, {0,128,64,64}, {0,192,64,64} };
        Uint32 i_frame_time_ms = 240;
        Uint32 last_i_frame_time_ms = 0;
        int current_iframe_index = 0;

        Uint32 last_iframes_start = 0;
        Uint32 iframes_duration = 1000;

        struct Primary_fire
        {
            Uint32 last_fire_time = 0;
            Uint32 cooldown_time_ms = 350; //ms
            // std::string png sprite sheet
            // vector<sdl_rect>
            //const char* .wav effect
        };
        Primary_fire primary_fire;

        struct Secondary_fire
        {
            // MARKER VARIABLES
            bool marker_active;
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
            
            
            // vector<sdl_rect>
            //std::string .wav effect
        };
        Secondary_fire secondary_fire;

        struct Shield
        {
            std::vector<SDL_Rect> shield_frames = { { 0, 0, 32, 32 }, { 32, 0, 32, 32 }, { 64, 0, 32, 32 }, { 96, 0, 32, 32 }, { 128, 0, 32, 32 }, { 160, 0, 32, 32 }, { 192, 0, 32, 32 }, { 224, 0, 32, 32 }, { 256, 0, 32, 32 } };
            SDL_Rect dest_rect;
            SDL_Rect coll_rect;
            std::string shield_texture_key = "player_shield";
            Uint32 last_shield_frame_time_ms = 0;
            Uint32 shield_frame_time_ms_ms = 30;
            
            Uint32 last_time_used = 0;
            Uint32 shield_cooldown_ms = 5000;

            bool shield_ready = true; // Bool var is used for trakcing when to play the ready animation after a state transistion. IsShieldReady is the function game.cpp uses to check

            int shield_frame_index = 0;
        };
        Shield shield;

        struct Player_Items
        {
            int num_glass_toucans = 0;
            int num_garnet_shields = 0;
        };
        Player_Items player_items;

        // EFFECTS
        bool show_healing_effects = false;
        int heal_effect_frame_index = 0;
        std::vector<SDL_Rect> heal_effect_frames = { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}, {128,0,32,32} };
        Uint32 last_heal_effect_frame_time_ms = 0;
        Uint32 heal_effect_frame_time_ms = 100;

        bool show_shield_ready_effects = false;
        int shield_ready_effect_frame_index = 0;
        std::vector<SDL_Rect> shield_ready_effect_frames = { {0,0,32,32}, {0,0,32,32}, {0,0,32,32}, {0,0,32,32} };
        Uint32 shield_ready_last_frame_time_ms = 0;
        Uint32 shield_ready_frame_time_ms = 100;

        // ANIMATIONS
		std::unordered_map<std::string, Animation> animations;  //The State related animations to choose from
		std::vector<Animation> overlay_animations;              //Animations that play over the base animation (like i-frames)
        Animation* current_animation;
};

#endif