#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <vector>
#include <SDL.h>
#include <string>
#include "Player.hpp"
#include "Animation.hpp"
#include "Collisions.hpp"

class Enemy
{
    protected:
        AnimationManager& animation_manager;
    public:
        Enemy(  AnimationManager& animation_manager, 
                const SDL_Rect& dest_rect,
                const Collider& coll_rect,
                float move_speed, 
                int health_arg, 
                float crit,
                float start_damage);

       
        // Setters and Getters
        Collider* GetCollShape();
        SDL_Rect* GetDstRect();
        int GetHealth();
        void ChangeHealth(int health_diff);

		int GetPoints();
		void AddOverlayAnimation(Animation* animation);
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
		float SHINY_CHANCE = 0.05f;
        bool shiny_sound_played = false;

        // ANIMATIONS
        std::vector<std::unique_ptr<Animation>> overlay_animations;         
        std::unique_ptr<Animation> current_animation;

        // Location
        SDL_Rect enemy_dest_rect;
        Collider enemy_coll_shape = Collider(0, 0, 5);

        int points;

        //States
        std::string state;
        bool invincible;

        // Override Functions
        virtual void Update(Player *player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, int screen_width, int screen_height) = 0;
        virtual void Attack(std::vector<Projectile*>& game_projectiles, Player* player) = 0;
        virtual void Move(Player* player,int screen_width, int screen_height) = 0;
        virtual bool IsReadyToAttack() = 0;
        virtual void Draw(SDL_Renderer* renderer, bool collision_box_flag) = 0;
};

class IceCrystal : public Enemy 
{
    public:
        IceCrystal(AnimationManager& animation_manager, const SDL_Rect& rect);
        void Update(Player *player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, int screen_width, int screen_height) override;
        void Move(Player* player, int screen_width, int screen_height) override;
        bool IsReadyToAttack() override;
        bool BulletReady();
        bool WaitDone();
        void Attack(std::vector<Projectile*>& game_projectiles, Player* player) override;
        void Draw(SDL_Renderer* renderer, bool collision_box_flag) override;
        void UpdateSwing();

private:
    bool added_death_animation = false;
    int num_proj_shot = 0;
    int max_proj_shot = 4;
    Uint32 bullet_rate = 100;
    Uint32 last_bullet_time = 0;
    
    Uint32 last_wait_time;
	Uint32 wait_duration = 1000;

    int player_distance_threshold = 10;

	// Movement Parameters
    float target_x = 0;
    float old_target_x = 0;
    bool do_swing = false;

    float acceleration = 0.04f;
    float deceleration = 0.0005f;  
    float max_speed = 1.4f;   
    float velocity = 0.0f;   
    float dead_zone; 
    float posX = 0.0f;

    int direction;

    bool going_down = false;
    float distance_to_swing = 100.0f;
    double swing_angle = 0.0;         // current rotation angle
	double max_swing_angle = 60.0; // maximum swing angle
    double BASE_STEP_ANGLE = 3.0;
    double step_angle = BASE_STEP_ANGLE;        // angle change per update
	double scale_factor = 0.80;          // scale factor for size change
    bool swinging_initialized = false;
};

class StormCloud : public Enemy
{
    public:
        StormCloud(AnimationManager& animation_manager, int screen_width, int screen_height, int player_x, int player_y);
        void Update(Player* player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, int screen_width, int screen_height) override;
        void Move(Player* player, int screen_width, int screen_height) override;
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
    void Update(Player* player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, int screen_width, int screen_height) override;
    void Move(Player* player, int screen_width, int screen_height) override;
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
    int lightning_strike_channel;

    bool death_animation_played  = false;
    
    // Movement Parameters
    float velocity = 0.5f;
    float posY = 0.0f;
    const float stiffness = 1.1f;
    const float damping = 0.7f;
    const float hover_amp = 1.5f;
    const float hover_freq = 0.07f;
    

	
};
#endif