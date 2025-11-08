#include <iostream>
#include <sstream>
#include "Projectiles.hpp"
#include "Graphics.hpp"
#include "AnimationManager.hpp"
#include <iomanip>

Projectile::Projectile(AnimationManager& animation_manager, 
    const SDL_Rect& dest_rect, 
    float projectile_speed, 
    float projectile_damage, 
    bool player_projectile_flag, 
    bool shift_impact_arg, 
    bool shiny_arg)
    : dest_rect(dest_rect),
    speed(projectile_speed),
    damage(projectile_damage),
    player_projectile(player_projectile_flag),
    state("main"),
    collision_shape(dest_rect),
    shift_impact(shift_impact_arg),
	animation_manager(animation_manager),
	shiny(shiny_arg)

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

     sound_effect_noise = 20;
    if(state_str == "impact" && shift_impact)
        if (player_projectile)
        {
            dest_rect.y -= dest_rect.h / 4;
        }
        else
        {
            dest_rect.y += dest_rect.h / 4;
        }
}

void Projectile::SetPosition(int x, int y)
{
	dest_rect.x = x + dest_rect.w / 2;
	dest_rect.y = y + dest_rect.h / 2;

}

const char* Projectile::GetState()
{
    return state;
}


SDL_Rect* Projectile::GetDstRect()
{
    return(&dest_rect);
}


Collider* Projectile::GetCollisionShape()
{
    return &collision_shape;
}


std::string Projectile::GetSoundEffectImpact()
{
    return sound_effect_impact;
}

std::string Projectile::GetPrintableDamage()
{
	if (damage == 0.0f)
		return "0";
    else
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << damage;

        return ss.str();
    }
}

// PRIMARY FIRE
PrimaryFire::PrimaryFire(AnimationManager& animation_manager, const SDL_Rect& dest_rect_arg, float projectile_speed, float projectile_damage, int PIXEL_SCALE, bool critical_flag)
    : Projectile(animation_manager, {(dest_rect_arg.x + dest_rect_arg.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect_arg.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE}, projectile_speed, projectile_damage, true, true, false)
{

    critical = critical_flag;
    sound_effect_noise = 20;
    current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-primary", "main"));
    
	dest_rect.x = (dest_rect_arg.x + dest_rect_arg.w / 2) - (current_animation->GetFrameWidth() * current_animation->GetScale() / 2);
	dest_rect.y = dest_rect_arg.y;
    pos_y = dest_rect.y;
    dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
    dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();
    collision_shape.type = ColliderType::CIRCLE;
	collision_shape.circle.x = dest_rect.x + dest_rect.w / 2;
    collision_shape.circle.y = dest_rect.y + dest_rect.h / 2;
    collision_shape.circle.r = dest_rect.w / 4;

    
    if (!critical)
        sound_effect_impact = "player_primary_fire_impact";
    else
        sound_effect_impact = "player_primary_fire_crit";

    if (critical)
    {
        damage *= 2;

    }

}

void PrimaryFire::MoveProjectile() 
{
	pos_y -= speed;
    dest_rect.y = static_cast<int>(pos_y);
    collision_shape.circle.y = dest_rect.y;
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
        collision_shape.circle.x = dest_rect.x + dest_rect.w / 2;
        collision_shape.circle.y = dest_rect.y + dest_rect.h / 2;
        collision_shape.circle.r = dest_rect.w / 3;

    }

    if (state == "impact")
    {
        collision_shape.circle.r = 0;
        
     
        
        if (current_animation->GetName() != "proj-zephyr-primary-impact")
        {
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-primary", "impact"));
            if (critical) overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-primary", "crit-effect")));
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
                            current_frame->w * animation->GetScale(),
                            current_frame->h * animation->GetScale() };
        animation->Draw(renderer, temp, SDL_FLIP_NONE);
    }

    if (collision_box_flag)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        Collisions::DrawCircle(renderer, collision_shape.circle);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

// SECONDARY FIRE
SecondaryFire::SecondaryFire(AnimationManager& animation_manager, const SDL_Rect& dest_rect_arg, float projectile_speed, int PIXEL_SCALE)
    : Projectile(animation_manager, { (dest_rect_arg.x + dest_rect_arg.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect_arg.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, projectile_speed, 0.0f, true, true, false)
{
    current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-secondary", "main"));
    dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
    dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();
	dest_rect.x = (dest_rect_arg.x + dest_rect_arg.w / 2) - (dest_rect.w / 2);
    pos_y = dest_rect.y;

    sound_effect_noise = 20;
	collision_shape.type = ColliderType::CIRCLE;
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
        
        // Store old center
        int old_center_x = dest_rect.x + dest_rect.w / 2;
        int old_center_y = dest_rect.y + dest_rect.h / 2;

        // Set new size
        dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
        dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();

        // Re-center
        dest_rect.x = old_center_x - dest_rect.w / 2;
        dest_rect.y = old_center_y - dest_rect.h / 2;

        speed = -0.6;
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
    pos_y -= speed;
    dest_rect.y = static_cast<int>(pos_y);
    collision_shape.circle.x = (dest_rect.x + dest_rect.w / 2);
    collision_shape.circle.y = (dest_rect.y + dest_rect.h / 2);
	collision_shape.circle.r = dest_rect.w / 4;
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
		Collisions::DrawCircle(renderer, collision_shape.circle);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

// SLASH
Slash::Slash(AnimationManager& animation_manager, SDL_Rect& dest_rect_arg, float projectile_damage, int PIXEL_SCALE, bool critical_flag, bool left_flag_arg)
    : Projectile(animation_manager, { (dest_rect.x + dest_rect.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, 0, projectile_damage, true, false, false)
{

    critical = critical_flag;
    sound_effect_noise = 20;
    current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-slash", "main"));
    collision_shape.type = ColliderType::RECT;
	left_flag = left_flag_arg;



    player_dest_rect = &dest_rect_arg;

    //float perc_thinner = 0.8;
	
    if (left_flag)
    {
        dest_rect.x = dest_rect.x - current_animation->GetFrameWidth() * current_animation->GetScale();
        dest_rect.y = dest_rect.y + (dest_rect.y / 2) - current_animation->GetFrameHeight() * current_animation->GetScale();
        dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
        dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();
        
        collision_shape.rect = dest_rect;
    }

    else
    {
		dest_rect.x = dest_rect.x + dest_rect.w;
        dest_rect.y = dest_rect.y + (dest_rect.y / 2) - current_animation->GetFrameHeight() * current_animation->GetScale();
        dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
        dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();
        collision_shape.rect = dest_rect;

    }

    if (critical)
    {
        damage *= 2;

    }

}

void Slash::MoveProjectile()
{
    int proj_w = current_animation->GetFrameWidth() * current_animation->GetScale();
    int proj_h = current_animation->GetFrameHeight() * current_animation->GetScale();

    // Set dest_rect normally (visual stays same)
    dest_rect.w = proj_w;
    dest_rect.h = proj_h;
    dest_rect.y = player_dest_rect->y + (player_dest_rect->h / 2) - (proj_h / 2);

    if (left_flag)
        dest_rect.x = player_dest_rect->x - proj_w;
    else
        dest_rect.x = player_dest_rect->x + player_dest_rect->w;

    // ---- COLLISION BOX THINNESS ----
    // Clamp thinness for safety
    float thinness = 0.6;

    int scaled_h = static_cast<int>(proj_h * thinness);
    int offset_y = (proj_h - scaled_h) / 2;  // center collision box vertically

    collision_shape.rect.x = dest_rect.x;
    collision_shape.rect.y = dest_rect.y + offset_y;
    collision_shape.rect.w = proj_w;
    collision_shape.rect.h = scaled_h;
}

void Slash::Update()
{
    if (state == "main")
    {
        if (current_animation->GetName() != "proj-zephyr-slash-main")
        {
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-zephyr-slash", "main"));
        }
        MoveProjectile();

    }

    if (current_animation->GetCurrentFrameIndex() > 0)
    {
        collision_shape.rect.w = 0;
        collision_shape.rect.h = 0;
    }

    if (current_animation->IsFinished())
    {
        state = "delete";
    }


    current_animation->Update();
    for (auto& animation : overlay_animations)
    {
        animation->Update();
    }
}

void Slash::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{

    if (left_flag)
        current_animation->Draw(renderer, dest_rect, SDL_FLIP_HORIZONTAL);
    else
        current_animation->Draw(renderer, dest_rect, SDL_FLIP_NONE);
    
    for (auto& animation : overlay_animations)
    {
        std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
        SDL_Rect* current_frame = animation->GetCurrentFrame();
        SDL_Rect temp = { (dest_rect.x + dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2,
                            (dest_rect.y + dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
                            current_frame->w,
                            current_frame->h };
        if (left_flag)
            animation->Draw(renderer, temp, SDL_FLIP_HORIZONTAL);
        else
            animation->Draw(renderer, temp, SDL_FLIP_NONE);
    }

    if (collision_box_flag)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &collision_shape.rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

// ICE SHARD

IceShard::IceShard(AnimationManager& animation_manager, const SDL_Rect& dest_rect_arg, float projectile_speed, int PIXEL_SCALE, float damage, bool shiny)
    : Projectile(animation_manager, { (dest_rect_arg.x + dest_rect_arg.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect_arg.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, projectile_speed, damage, false, false, shiny)
{
    sound_effect_impact = "ice_shard_impact";
    sound_effect_noise = 20;
	collision_shape.type = ColliderType::CIRCLE;
	collision_shape.circle.x = dest_rect.x + dest_rect.w / 2;
	collision_shape.circle.y = dest_rect.y + dest_rect.h / 2;
	collision_shape.circle.r = dest_rect.w / 4;
    state = "spawn";
    if (shiny)
        current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-ice-crystal-attack", "spawn_shiny"));
    else
        current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-ice-crystal-attack", "spawn"));
	dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
	dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();
}
void IceShard::MoveProjectile()
{
    dest_rect.y += speed;
}

void IceShard::Update()
{
    if (state == "spawn")
    {
		collision_shape.circle.r = 0;
        collision_shape.circle.x = 0;
        collision_shape.circle.y = 0;
        if (current_animation->IsFinished())
            state = "main";
    }
    
    if (state == "main")
    {
        if (current_animation->GetName() != "proj-ice-crystal-attack-main")
        {
            if (shiny)
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-ice-crystal-attack", "main_shiny"));
            else
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-ice-crystal-attack", "main"));
        }
        MoveProjectile();
        collision_shape.circle.x = dest_rect.x + dest_rect.w / 2;
        collision_shape.circle.y = dest_rect.y + dest_rect.h / 2;
		collision_shape.circle.r = dest_rect.w / 4;
    }
    
    if (state == "impact")
    {
        collision_shape.circle.r = 0;
        if (current_animation->GetName() != "proj-ice-crystal-attack-impact" and current_animation->GetName() != "proj-ice-crystal-attack-impact_shiny")
        {
            if (shiny)
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-ice-crystal-attack", "impact_shiny"));
            else
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-ice-crystal-attack", "impact"));
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
		Collisions::DrawCircle(renderer, collision_shape.circle);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

// LIGHTNING BALL

LightningBall::LightningBall(AnimationManager& animation_manager, const SDL_Rect& dest_rect_arg, float projectile_speed, int PIXEL_SCALE, float damage, int player_x, int player_y, bool shiny)
    : Projectile(animation_manager, { (dest_rect_arg.x + dest_rect_arg.w / 2) - (32 * PIXEL_SCALE / 2), dest_rect_arg.y, 32 * PIXEL_SCALE, 32 * PIXEL_SCALE }, projectile_speed, damage, false, false, shiny)
{
    sound_effect_impact = "lightning_ball_impact";
    sound_effect_noise = 80;
	collision_shape.type = ColliderType::CIRCLE;

    if (shiny)
        current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-cloud-attack", "main_shiny"));
    else
        current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-cloud-attack", "main"));
	dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
	dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();
    
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
    collision_shape.circle.x = static_cast<int>(position_x) + (dest_rect.w / 2);
    collision_shape.circle.y = static_cast<int>(position_y) + (dest_rect.h / 2);
	collision_shape.circle.r = dest_rect.w / 5;
}

void LightningBall::Update()
{
    if (state == "main")
    {
        if (current_animation->GetName() != "proj-storm-cloud-attack-main" and current_animation->GetName() != "proj-storm-cloud-attack-main_shiny")
        {
            if (shiny)
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-cloud-attack", "main_shiny"));
            else
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-cloud-attack", "main"));
        }
        MoveProjectile();
    }

    if (state == "impact")
    {
        if (current_animation->GetName() != "proj-storm-cloud-attack-impact" and current_animation->GetName() != "proj-storm-cloud-attack-impact_shiny")
        {
            if (shiny)
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-cloud-attack", "impact_shiny"));
            else
                current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-cloud-attack", "impact"));
        }
        collision_shape.circle.r = 0;
        collision_shape.circle.x = 0;
        collision_shape.circle.y = 0;
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
		Collisions::DrawCircle(renderer, collision_shape.circle);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

// Lightning Strike ===============================================

LightningStrike::LightningStrike(AnimationManager& animation_manager, const SDL_Rect& dest_rect_arg, int PIXEL_SCALE, float damage, bool right_flag_arg, bool shiny)
    : Projectile(animation_manager, dest_rect_arg, 0, damage, false, false, shiny)
{
    sound_effect_impact = "lightning_strike_impact";
    sound_effect_noise = 80;
    right_flag = right_flag_arg;

    collision_shape.type = ColliderType::RECT;


    if (right_flag)
    {
        if (shiny)
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-right_shiny"));
        else
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-right"));
    }
    else
    {
        if (shiny)
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-left_shiny"));
        else
            current_animation = std::make_unique<Animation>(*animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-left"));
    }
	dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
	dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale(); 
}
void LightningStrike::MoveProjectile()
{
    return;
}

void LightningStrike::Update()
{
   /* if (current_animation->IsFinished())
        state = "delete";*/

    if (state == "main")
    {
        float perc_thinner = 0.27;
        int new_height = static_cast<int>(dest_rect.h * perc_thinner);
        int new_y_pos = dest_rect.y + (dest_rect.h - new_height) / 2;  // center it vertically
        
        collision_shape.rect.x = dest_rect.x;
        collision_shape.rect.w = dest_rect.w;
		collision_shape.rect.y = new_y_pos;
        collision_shape.rect.h = new_height;
    }

    if (state == "impact")
        collision_shape.rect = { 0,0,0,0 };

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
        if (0 != SDL_RenderDrawRect(renderer, &collision_shape.rect))
        {
            std::cout << "[*] Error rendering storm genie collision box ...\n";
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}