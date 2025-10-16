#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <vector>
#include <SDL.h>
#include <string>
#include "Player.hpp"
#include "Animation.hpp"

class Enemy
{
    protected:
        AnimationManager& animation_manager;
    public:
        Enemy(AnimationManager& animation_manager, const SDL_Rect& dest_rect, const SDL_Rect& coll_rect,
            float move_speed, int health_arg, float crit, float start_damage);

       
        // Setters and Getters
        SDL_Rect* GetCollRect();
        SDL_Rect* GetDstRect();
        int GetHealth();
        void ChangeHealth(int health_diff);

		int GetPoints();

        void UpdateState(std::string state);
        std::string GetState();
    
        //STATS
        float movement_speed;
        

        float base_damage;
        int base_health;
        float crit_percent = 0;
        Uint32 fire_cooldown_ms = 1000; 
        Uint32 last_fire_time = 0;
        bool IsDoneAttacking();

        //IMAGE STUFF
        int BASE_SPRITE_SIZE = 32;
        int image_scale;
        
        bool shiny;
        bool shiny_sound_played = false;

        // ANIMATIONS
        std::vector<std::unique_ptr<Animation>> overlay_animations;         
        std::unique_ptr<Animation> current_animation;

        // Location
        SDL_Rect enemy_dest_rect;
        SDL_Rect enemy_coll_rect;

        int points;

        //States
        std::string state;
        bool invincible;

        // Override Functions
        virtual void Update(Player *player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager) = 0;
        virtual void Attack(std::vector<Projectile*>& game_projectiles, Player* player) = 0;
        virtual void Move(Player* player) = 0;
        virtual bool IsReadyToAttack() = 0;
        virtual void Draw(SDL_Renderer* renderer, bool collision_box_flag) = 0;
};

class IceCrystal : public Enemy 
{
    public:
        IceCrystal(AnimationManager& animation_manager, const SDL_Rect& rect);
        void Update(Player *player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager) override;
        void Move(Player* player) override;
        bool IsReadyToAttack() override;
        bool BulletReady();
        bool WaitDone();
        void Attack(std::vector<Projectile*>& game_projectiles, Player* player) override;
        void Draw(SDL_Renderer* renderer, bool collision_box_flag) override;

private:
    bool added_death_animation = false;
    float target_x = 0;
    int num_proj_shot = 0;
    int max_proj_shot = 4;
    Uint32 bullet_rate = 100;
    Uint32 last_bullet_time = 0;
    
    Uint32 last_wait_time;
	Uint32 wait_duration = 1000;
    // --- Tunable parameters ---
    float velocity = 1.0f;
    // --- Tunable parameters ---
    const float stiffness = 2.0f;     // increase after normalization
    const float damping = 0.7f;
    const float hover_amp = 1.5f;
    const float hover_freq = 0.07f;
    // In your class:
    float posX = 0.0f; // store precise position
};

class StormCloud : public Enemy
{
    public:
        StormCloud(AnimationManager& animation_manager, int screen_width, int screen_height, int player_x, int player_y);
        void Update(Player* player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager) override;
        void Move(Player* player) override;
        bool IsReadyToAttack() override;
        void Attack(std::vector<Projectile*>& game_projectiles, Player* player) override;
        void Draw(SDL_Renderer* renderer, bool collision_box_flag) override;

        int GetGoalX();
        int GetGoalY();
    private:
        double direction_x, direction_y;
        double position_x = enemy_dest_rect.x;
        double position_y = enemy_dest_rect.y;

        Uint32 fire_cooldown_ms = 300;
        Uint32 time_to_wait_ms = 1500;
        Uint32 start_of_wait_state;
        int goal_x, goal_y;
        bool shot_fired = false;
        bool first_time_waiting = true;
        bool added_death_animation = false;
        
        
};

class 
    StormGenie: public Enemy
{
public:
    StormGenie(AnimationManager& animation_manager, const SDL_Rect& rect);
    void Update(Player* player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager) override;
    void Move(Player* player) override;
    bool IsReadyToAttack() override;
    void Attack(std::vector<Projectile*>& game_projectiles, Player* player) override;
    void Draw(SDL_Renderer* renderer, bool collision_box_flag) override;

    bool IsDoneAttacking();

private:
    bool shot_fired;
    bool spawned_lightning;

    Projectile* left_lightning_bolt = nullptr;
    Projectile* right_lightning_bolt = nullptr;

    Uint32 last_fire_time = 0;
    Uint32 lightning_strike_duration = 1500;

    bool death_animation_played  = false;
    
    // --- Tunable parameters ---
    float velocity = 1.0f;
    const float stiffness = 0.03f;   // "pull" toward player
    const float damping = 0.88f;   // smoothness (0.85–0.92 feels good)
    const float hover_amp = 0.6f;    // gentle floating motion amplitude
    const float hover_freq = 0.04f;  // oscillation speed
    // In your class:
    float posY = 0.0f; // store precise position
    
	
};
#endif