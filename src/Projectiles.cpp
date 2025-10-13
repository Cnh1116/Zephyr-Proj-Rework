#include <iostream>
#include "Projectiles.hpp"
#include "Graphics.hpp"
#include "AnimationManager.hpp"

Projectile::Projectile(AnimationManager& animation_manager, const SDL_Rect& dest_rect, float projectile_speed, float projectile_damage, bool player_projectile_flag, bool shift_impact_arg)
    : dest_rect(dest_rect),
    speed(projectile_speed),
    damage(projectile_damage),
    player_projectile(player_projectile_flag),
    state("main"),
    collision_rect(dest_rect),
    shift_impact(shift_impact_arg),
	animation_manager(animation_manager)

{
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


SDL_Rect* Projectile::GetCollisionRect()
{
    return &collision_rect;
}


const char* Projectile::GetSoundEffectImpact()
{
    return sound_effect_impact;
}

// PRIMARY FIRE

PrimaryFire::PrimaryFire(AnimationManager& animation_manager, const SDL_Rect& dest_rect, float projectile_speed, float projectile_damage, int PIXEL_SCALE, bool critical_flag)
    : Projectile(animation_manager, {(dest_rect.x + dest_rect.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE}, projectile_speed, projectile_damage, true, true)
{

    critical = critical_flag;

    current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-primary", "main"));

    if (critical)
    {
        projectile_damage *= 2;

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
		if (current_animation->GetName() != "proj-zephyr-primary-main")
		{
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-primary", "main"));
		}
        MoveProjectile();
        collision_rect.w = dest_rect.w / 2;
        collision_rect.h = dest_rect.h / 2;
        collision_rect.x = dest_rect.x + (dest_rect.w / 2) - (collision_rect.w / 2);
        collision_rect.y = dest_rect.y + (dest_rect.h / 2) - (collision_rect.h / 2);
    }

    if (state == "impact")
    {
        collision_rect = { 0,0,0,0 };
        
        if (!critical)
        {

            if (current_animation->GetName() != "proj-zephyr-primary-impact")
            {
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-primary", "impact"));
            }

        }
        else
        {
            if (current_animation->GetName() != "proj-zephyr-primary-impact-crit")
            {
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-primary", "impact-crit"));
            }
        }

        if (current_animation->IsFinished())
        {
            state = "delete";
        }
    }

    current_animation->Update();
    for (auto& animation : overlay_animations)
    {
        animation->Update();
    }
}

void PrimaryFire::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{

    current_animation->Draw(renderer, dest_rect, SDL_FLIP_NONE);
    for (auto& animation : overlay_animations)
    {
        std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
        SDL_Rect* current_frame = animation->GetCurrentFrame();
        SDL_Rect temp = { (dest_rect.x + dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2,
                            (dest_rect.y + dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
                            current_frame->w,
                            current_frame->h };
        animation->Draw(renderer, temp, SDL_FLIP_NONE);
    }

    if (collision_box_flag)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        if (0 != SDL_RenderDrawRect(renderer, &collision_rect))
        {
            std::cout << "[*] Error rendering storm genie collision box ...\n";
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

// SECONDARY FIRE
SecondaryFire::SecondaryFire(AnimationManager& animation_manager, const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE)
    : Projectile(animation_manager, { (dest_rect.x + dest_rect.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, projectile_speed, 0.0f, true, false)
{
    current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-secondary", "main"));
}

void SecondaryFire::Update()
{
    MoveProjectile();

    if (state == "main")
    {
        if (current_animation->GetName() != "proj-zephyr-secondary-main")
        {
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-secondary", "main"));
        }

    }
    
    if (state == "impact")
    {
        if (current_animation->GetName() != "proj-zephyr-secondary-impact")
        {
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-secondary", "impact"));
        }
        
        
        //Laterally Middle of current sizing minus half width of new smaller size
        dest_rect.x = (dest_rect.x + dest_rect.w / 2) - (32 * 2 / 2);
        dest_rect.y = (dest_rect.y + dest_rect.h / 2) - (32 * 2 / 2);

        dest_rect.w = 32 * 2;
        dest_rect.h = 32 * 2;
        speed = -1;
    }
    if (state == "impact" && current_animation->IsFinished())
    {
        state = "delete";
    }

    current_animation->Update();
    for (auto& animation : overlay_animations)
    {
        animation->Update();
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

void SecondaryFire::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{

    current_animation->Draw(renderer, dest_rect, SDL_FLIP_NONE);
    for (auto& animation : overlay_animations)
    {
        std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
        SDL_Rect* current_frame = animation->GetCurrentFrame();
        SDL_Rect temp = { (dest_rect.x + dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2,
                            (dest_rect.y + dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
                            current_frame->w,
                            current_frame->h };
        animation->Draw(renderer, temp, SDL_FLIP_NONE);
    }

    if (collision_box_flag)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        if (0 != SDL_RenderDrawRect(renderer, &collision_rect))
        {
            std::cout << "[*] Error rendering storm genie collision box ...\n";
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

// ICE SHARD

IceShard::IceShard(AnimationManager& animation_manager, const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE, float damage)
    : Projectile(animation_manager, { (dest_rect.x + dest_rect.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, projectile_speed, damage, false, false)
{
    sound_effect_impact = "ice_shard_impact";
    current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-ice-crystal-attack", "main"));
}
void IceShard::MoveProjectile()
{
    dest_rect.y += speed;
}

void IceShard::Update()
{
    if (state == "main")
    {
        if (current_animation->GetName() != "proj-ice-crystal-attack-main")
        {
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-ice-crystal-attack", "main"));
        }
        MoveProjectile();
        collision_rect.w = dest_rect.w / 2;
        collision_rect.h = dest_rect.h / 2;
        collision_rect.x = dest_rect.x + (dest_rect.w / 2) - (collision_rect.w / 2);
        collision_rect.y = dest_rect.y + (dest_rect.h / 2) - (collision_rect.h / 2);
    }
    
    if (state == "impact")
    {
        if (current_animation->GetName() != "proj-ice-crystal-attack-impact")
        {
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-ice-crystal-attack", "impact"));
        }
        collision_rect = { 0,0,0,0 };
        if (current_animation->IsFinished())
        {
            state = "delete";
        }
    }

    current_animation->Update();
    for (auto& animation : overlay_animations)
    {
        animation->Update();
    }
}

void IceShard::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{

    current_animation->Draw(renderer, dest_rect, SDL_FLIP_NONE);
    for (auto& animation : overlay_animations)
    {
        std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
        SDL_Rect* current_frame = animation->GetCurrentFrame();
        SDL_Rect temp = { (dest_rect.x + dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2,
                            (dest_rect.y + dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
                            current_frame->w,
                            current_frame->h };
        animation->Draw(renderer, temp, SDL_FLIP_NONE);
    }

    if (collision_box_flag)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        if (0 != SDL_RenderDrawRect(renderer, &collision_rect))
        {
            std::cout << "[*] Error rendering storm genie collision box ...\n";
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

// LIGHTNING BALL

LightningBall::LightningBall(AnimationManager& animation_manager, const SDL_Rect& dest_rect, float projectile_speed, int PIXEL_SCALE, float damage, int player_x, int player_y)
    : Projectile(animation_manager, { (dest_rect.x + dest_rect.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, projectile_speed, damage, false, false)
{
    sound_effect_impact = "lightning_ball_impact";
    current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-cloud-attack", "main"));
    
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
        if (current_animation->GetName() != "proj-storm-cloud-attack-main")
        {
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-cloud-attack", "main"));
        }
        MoveProjectile();
        collision_rect.w = dest_rect.w / 2;
        collision_rect.h = dest_rect.h / 2;
        collision_rect.x = dest_rect.x + (dest_rect.w / 2) - (collision_rect.w / 2);
        collision_rect.y = dest_rect.y + (dest_rect.h / 2) - (collision_rect.h / 2);
    }

    if (state == "impact")
    {
        if (current_animation->GetName() != "proj-storm-cloud-attack-impact")
        {
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-cloud-attack", "impact"));
        }
        collision_rect = { 0,0,0,0 };
        if (current_animation->IsFinished())
        {
            state = "delete";
        }
    }

    current_animation->Update();
    for (auto& animation : overlay_animations)
    {
        animation->Update();
    }
}

void LightningBall::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{

    current_animation->Draw(renderer, dest_rect, SDL_FLIP_NONE);
    for (auto& animation : overlay_animations)
    {
        std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
        SDL_Rect* current_frame = animation->GetCurrentFrame();
        SDL_Rect temp = { (dest_rect.x + dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2,
                            (dest_rect.y + dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
                            current_frame->w,
                            current_frame->h };
        animation->Draw(renderer, temp, SDL_FLIP_NONE);
    }

    if (collision_box_flag)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        if (0 != SDL_RenderDrawRect(renderer, &collision_rect))
        {
            std::cout << "[*] Error rendering storm genie collision box ...\n";
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

// Lightning Strike ===============================================

LightningStrike::LightningStrike(AnimationManager& animation_manager, const SDL_Rect& dest_rect, int PIXEL_SCALE, float damage, bool right_flag_arg)
    : Projectile(animation_manager, dest_rect, 0, damage, false, false)
{
    sound_effect_impact = "lightning_ball_impact";
    right_flag = right_flag_arg;
    if (right_flag)
    {
        current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-right"));
    }
    else
    {
        current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-left"));
        
    }
}
void LightningStrike::MoveProjectile()
{
    return;
}

void LightningStrike::Update()
{
    if (current_animation->IsFinished())
        state = "delete";

    if (state == "main")
    {
        collision_rect = dest_rect;
    }

    if (state == "impact")
        collision_rect = { 0,0,0,0 };

    current_animation->Update();
    for (auto& animation : overlay_animations)
    {
        animation->Update();
    }
}

void LightningStrike::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{

    current_animation->Draw(renderer, dest_rect, SDL_FLIP_NONE);
    for (auto& animation : overlay_animations)
    {
        std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
        SDL_Rect* current_frame = animation->GetCurrentFrame();
        SDL_Rect temp = { (dest_rect.x + dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2,
                            (dest_rect.y + dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
                            current_frame->w,
                            current_frame->h };
        animation->Draw(renderer, temp, SDL_FLIP_NONE);
    }

    if (collision_box_flag)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        if (0 != SDL_RenderDrawRect(renderer, &collision_rect))
        {
            std::cout << "[*] Error rendering storm genie collision box ...\n";
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}