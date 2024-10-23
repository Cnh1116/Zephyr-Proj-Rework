#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <vector>
#include <SDL.h>
#include <string>

#include "Player.hpp"


class Enemy
{

    public:
        Enemy(const SDL_Rect& dest_rect, const SDL_Rect& coll_rect, std::vector<SDL_Rect> main_frames, std::vector<SDL_Rect> death_frames, float move_speed, int health_arg, float crit, float start_damage);

        // Setters and Getters
        SDL_Rect* GetCollRect();
        SDL_Rect* GetDstRect();
        std::string GetTextureKey();
        int NumOfFrames();
        Uint32 GetLastFrameStart();
        Uint32 GetFrameTime();
        void SetLastFrameTime(Uint32 current_time);
        int GetFrameIndex();
        void SetFrameIndex(int index);
        int GetHealth();
        void ChangeHealth(int health_diff);

        // Functions
        void AdvanceFrame();
        void UpdateState(std::string state);
        SDL_Rect* GetFrame();
        std::string GetState();
        bool IsReadyToAttack();

    
        //STATS
        float movement_speed;
        int base_damage;
        int base_health;
        float crit_percent = 0;
        Uint32 fire_cooldown_ms = 2000;
        Uint32 last_fire_time = 0;

        //IMAGE STUFF
        int BASE_SPRITE_SIZE = 32;
        int image_scale;
        std::string current_texture_key;
        std::string main_texture_key;
        std::string death_texture_key;

        std::vector<SDL_Rect> main_frames;
        std::vector<SDL_Rect> death_frames;
        std::vector<SDL_Rect> current_frames;

        int current_frame_index = 0;
        Uint32 frame_time_ms = 150; // Might need seperate frame cool downs per animation type.
        Uint32 last_frame_time = 0;


        SDL_Rect enemy_dest_rect;
        SDL_Rect enemy_coll_rect;

        //States
        std::string state;
        bool invincible;

        // Override Functions
        virtual void Update(Player *player) = 0;
        virtual void Move(Player* player) = 0;


};

class IceCrystal : public Enemy {
public:
    IceCrystal(const SDL_Rect& rect);
    void Update(Player *player) override;
    void Move(Player* player) override;
};


#endif