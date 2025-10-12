#include <iostream>
#include "Projectiles.hpp"
#include "Graphics.hpp"
#include "AnimationManager.hpp"

Projectile::Projectile( const SDL_Rect& dest_rect, float projectile_speed, float projectile_damage, bool player_projectile_flag, std::string main_texture_key_str, std::vector<SDL_Rect> frames, std::string impact_texture_key_str, std::vector<SDL_Rect> imp_frames, Uint32 frame_cooldown_ms, bool replayable, bool shift_impact_arg)
    : dest_rect(dest_rect),
    speed(projectile_speed),
    damage(projectile_damage),
    player_projectile(player_projectile_flag),
    main_texture_key(main_texture_key_str),
    impact_texture_key(impact_texture_key_str),
    current_texture_key(main_texture_key_str),
    current_frame_index(0),
    main_frames(frames),
    impact_frames(imp_frames),
    frame_time_ms_ms(frame_cooldown_ms),
    last_frame_time_ms(0),
    animation_replayable(replayable),
    state("main"),
    collision_rect(dest_rect),
    current_frames(main_frames),
    shift_impact(shift_impact_arg)

{
}

std::string Projectile::GetTextureKey()
{
    
    return current_texture_key;
}

Projectile::~Projectile()
{
}

void Projectile::MoveProjectile()
{

    dest_rect.y += speed;

}

void Projectile::UpdateState(const char* state_str)
{
    state = state_str;
    last_frame_time_ms = 0;
    current_frame_index = 0;

    if(state_str == "impact" && shift_impact)
        if (player_projectile)
        {
            dest_rect.y -= dest_rect.h / 2;
        }
        else
        {
            dest_rect.y += dest_rect.h / 2;
        }
}

const char* Projectile::GetState()
{
    return state;
}


SDL_Rect* Projectile::GetDstRect()
{
    return(&dest_rect);
}

SDL_Rect* Projectile::GetFrame()
{
    return &current_frames[current_frame_index];
}

Uint32 Projectile::GetFrameTime()
{
    return frame_time_ms_ms;
}
Uint32 Projectile::GetLastFrameStart()
{
    return last_frame_time_ms;
}

bool Projectile::IsReplayable()
{
    return animation_replayable;
}

int Projectile::NumOfFrames()
{
    return current_frames.size();
}

SDL_Rect* Projectile::GetCollisionRect()
{
    return &collision_rect;
}

void Projectile::AdvanceFrame()
{
    current_frame_index++;
}

const char* Projectile::GetSoundEffectImpact()
{
    return sound_effect_impact;
}

// PRIMARY FIRE

PrimaryFire::PrimaryFire(const SDL_Rect& dest_rect, float projectile_speed, float projectile_damage, int PIXEL_SCALE, bool critical_flag)
    : Projectile({(dest_rect.x + dest_rect.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE}
        , projectile_speed, projectile_damage, true, "primary_fire", { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32} }, "primary_fire_impact", { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32} }, 120, true, true)
{
    current_texture_key = "primary_fire";
    current_frames = main_frames;
    critical = critical_flag;

    if (critical)
    {
        projectile_damage *= 2;
        impact_texture_key = "primary_fire_crit_impact";
        impact_frames = { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}, {128,0,32,32}, {160,0,32,32} };
    }
    else
    {
        impact_texture_key = "primary_fire_impact";
        impact_frames = { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32} };
    }
}

void PrimaryFire::MoveProjectile() 
{
    dest_rect.y -= speed;
    collision_rect.y = dest_rect.y;
}

void PrimaryFire::Update()
{
    if (state == "main")
    {
        MoveProjectile();
        collision_rect.w = dest_rect.w / 2;
        collision_rect.h = dest_rect.h / 2;
        collision_rect.x = dest_rect.x + (dest_rect.w / 2) - (collision_rect.w / 2);
        collision_rect.y = dest_rect.y + (dest_rect.h / 2) - (collision_rect.h / 2);
    }

    if (state == "impact")
    {
        collision_rect = { 0,0,0,0 };
        
        current_frames = impact_frames;
        current_texture_key = impact_texture_key;
        if (current_frame_index >= impact_frames.size() - 1)
        {
            state = "delete";
        }
    }
}

// SECONDARY FIRE
SecondaryFire::SecondaryFire(const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE)
    : Projectile({ (dest_rect.x + dest_rect.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, projectile_speed, 0.0f, true, "secondary_fire", { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}, {128,0,32,32} }, "secondary_fire_impact", { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}, {128,0,32,32} }, 200, false, false)
{
}

void SecondaryFire::Update()
{
    MoveProjectile(); // Always move sec. fire down cause of hitting cloud

    if (state == "main")
    {
        current_frames = main_frames;
        current_texture_key = "secondary_fire";
    }
    
    if (state == "impact")
    {
        current_frames = impact_frames;
        current_texture_key = "secondary_fire_impact";
        
        //Laterally Middle of current sizing minus half width of new smaller size
        dest_rect.x = (dest_rect.x + dest_rect.w / 2) - (32 * 2 / 2);
        dest_rect.y = (dest_rect.y + dest_rect.h / 2) - (32 * 2 / 2);

        dest_rect.w = 32 * 2;
        dest_rect.h = 32 * 2;
        speed = -1;
    }
    if (state == "impact" && current_frame_index >= impact_frames.size() - 1)
    {
        state = "delete";
    }
}

void SecondaryFire::MoveProjectile() 
{
    dest_rect.y -= speed;
    collision_rect = { (dest_rect.x + dest_rect.w / 2) - (32 / 2),
                        (dest_rect.y + dest_rect.h / 2) - (32 / 2), 
                            32, 
                           32};
}


// ICE SHARD

IceShard::IceShard(const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE, float damage)
    : Projectile({ (dest_rect.x + dest_rect.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, projectile_speed, damage, false, "ice_shard", { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}, {128,0,32,32}, {160,0,32,32}, {0,32,32,32}, {32,32,32,32}, {64,32,32,32}, {96,32,32,32}, {128,32,32,32}, {160,32,32,32}, {0,64,32,32}, {32,64,32,32}, {64,64,32,32} }, "ice_shard_impact", { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}, {128,0,32,32} }, 120, true, false)
{
    sound_effect_impact = "ice_shard_impact";
}
void IceShard::MoveProjectile()
{
    dest_rect.y += speed;
}

void IceShard::Update()
{
    if (state == "main")
    {
        MoveProjectile();
        collision_rect.w = dest_rect.w / 2;
        collision_rect.h = dest_rect.h / 2;
        collision_rect.x = dest_rect.x + (dest_rect.w / 2) - (collision_rect.w / 2);
        collision_rect.y = dest_rect.y + (dest_rect.h / 2) - (collision_rect.h / 2);
    }
    
    if (state == "impact")
    {
        collision_rect = { 0,0,0,0 };
        current_frames = impact_frames;
        current_texture_key = "ice_shard_impact";
        if (current_frame_index >= impact_frames.size() - 1)
        {
            state = "delete";
        }
    }
}

// LIGHTNING BALL

LightningBall::LightningBall(const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE, float damage, int player_x, int player_y)
    : Projectile({ (dest_rect.x + dest_rect.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, projectile_speed, damage, false, "lightning_ball", { {0,0,40,40}, {40,0,40,40}, {80,0,40,40}, {120,0,40,40}, {160,0,40,40}, {200,0,40,40}, {240,0,40,40}, {280,0,40,40}, {320,0,40,40}, {360,0,40,40} }, "lightning_ball_impact", { {0,0,64,64}, {64,0,64,64}, {192,0,64,64}, {256,0,64,64}, {320,0,64,64} } , 120, true, false)
{
    sound_effect_impact = "lightning_ball_impact";
    
    float delta_x = static_cast<double>(player_x) - (dest_rect.x + (dest_rect.w / 2));
    float delta_y = static_cast<double>(player_y) - (dest_rect.y + (dest_rect.h / 2));
    std::cout << "Delta x: " << delta_x << " Delta Y: " << delta_y << std::endl;

    // Normalize the direction vector
    float length = std::sqrt(delta_x * delta_x + delta_y * delta_y);
    if (length != 0)
    {
        direction_x = delta_x / length;
        direction_y = delta_y / length;
    }
    else
    {
        direction_x = 0;
        direction_y = 0;
    }

    position_x = dest_rect.x;
    position_y = dest_rect.y;
}
void LightningBall::MoveProjectile()
{
    position_x += direction_x * speed;
    position_y += direction_y * speed;

    dest_rect.x = static_cast<int>(position_x) + (dest_rect.w / 2) - (dest_rect.w / 2);
    dest_rect.y = static_cast<int>(position_y) + (dest_rect.h / 2) - (dest_rect.h / 2);
    collision_rect.x = static_cast<int>(position_x) + (dest_rect.w / 2) - (collision_rect.w / 2);
    collision_rect.y = static_cast<int>(position_y) + (dest_rect.h / 2) - (collision_rect.h / 2);
    collision_rect.w = dest_rect.w / 2;
    collision_rect.h = dest_rect.h / 2;
}

void LightningBall::Update()
{
    if (state == "main")
    {
        MoveProjectile();
        collision_rect.w = dest_rect.w / 2;
        collision_rect.h = dest_rect.h / 2;
        collision_rect.x = dest_rect.x + (dest_rect.w / 2) - (collision_rect.w / 2);
        collision_rect.y = dest_rect.y + (dest_rect.h / 2) - (collision_rect.h / 2);
    }

    if (state == "impact")
    {
        collision_rect = { 0,0,0,0 };
        current_frames = impact_frames;
        current_texture_key = "lightning_ball_impact";
        if (current_frame_index >= impact_frames.size() - 1)
        {
            state = "delete";
        }
    }
}

// Lightning Strike ===============================================

LightningStrike::LightningStrike(const SDL_Rect& dest_rect, int PIXEL_SCALE, float damage, bool right_flag_arg)
    : Projectile(dest_rect, 0, damage, false,
        "lightning_strike", { {0,0,111,48}, {0,48,111,48}, {0,96,111,48}, {0,144,111,48}, {0,192,111,48}, {0,240,111,48} },
        "lightning_strike_impact", { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}, {128,0,32,32} }, 
        115, false, false)
{
    sound_effect_impact = "lightning_ball_impact";
    right_flag = right_flag_arg;
    if (right_flag)
        current_texture_key = "lightning_strike_right";
    else
        current_texture_key = "lightning_strike_left";

    current_frame_index = 0;
    last_frame_time_ms = 0;
}
void LightningStrike::MoveProjectile()
{
    return;
}

void LightningStrike::Update()
{
    if (current_frame_index == current_frames.size() - 1)
        state = "delete";

    if (state == "main")
    {
        collision_rect = dest_rect;
    }

    if (state == "impact")
        collision_rect = { 0,0,0,0 };


}