#include <iostream>
#include <random>

#include "Player.hpp"
#include "Graphics.hpp"
#include "AnimationManager.hpp"
#include "Collisions.hpp"
#include "Sound.hpp"

Player::Player(int PIXEL_SCALE, AnimationManager* animation_manager)
{
    
    std::cout << "[*] I am a new player\n";
    image_scale = PIXEL_SCALE;
    
    animations["main"] = *animation_manager->Get("zephyr", "main");
    animations["iframes"] = *animation_manager->Get("zephyr", "iframes");
    animations["shield"] = *animation_manager->Get("zephyr", "shield");
    animations["secondary_fire_hud"] = *animation_manager->Get("zephyr", "secondary_fire_hud");
    animations["secondary_fire_marker"] = *animation_manager->Get("zephyr", "secondary_fire_marker");
    animations["shield_ready_effects"] = *animation_manager->Get("overlays", "shield_ready");
    animations["heal"] = *animation_manager->Get("overlays", "heal");
    current_animation = &animations["main"];
    
    state = "main";
    bool invincible = false;

    player_dest_rect = {0, 0, BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
    player_coll_rect = { 0, 0, BASE_SPRITE_SIZE*3, BASE_SPRITE_SIZE };

    // BASE STATS
    base_speed = 5.0;
    dash_speed = 10.2;
    base_damage = 10;
    base_health = 150;
    crit_percent = 1.0; // Percentage

    
    secondary_fire.source_rect = {32, 32, 32, 32}; 
    secondary_fire.hud_dest_rect  = {player_dest_rect.x, player_dest_rect.y, BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
    secondary_fire.hud_coll_rect = { player_dest_rect.x, player_dest_rect.y, BASE_SPRITE_SIZE, BASE_SPRITE_SIZE };
    secondary_fire.marker_active = false;
    secondary_fire.ready = true;
}


void Player::Update(int x_pos, int y_pos, int SCREEN_WIDTH, int SCREEN_HEIGHT, long loop_flag, Uint32 tick, SoundManager& sound_manager)
{
    SetPosition(x_pos, y_pos, SCREEN_WIDTH, SCREEN_HEIGHT);
	std::cout << "[*] Player STATE: " << state << std::endl;
    
    if (!secondary_fire.marker_active) secondary_fire.marker_col_rect = { 0,0,0,0 };
    
    
    if (state != "shield" && shield.shield_ready == false && IsShieldReady())
    {
        shield.shield_ready = true;
		std::cout << "[*] SHIELD IS READY !\n";

        animations["shield_ready_effects"].OutputInformation();
        animations["shield_ready_effects"].Reset();
        animations["shield"].Reset();
        overlay_animations.push_back(std::make_unique<Animation>(animations["shield_ready_effects"]));

		sound_manager.PlaySound("jade_drum", 55);
    }
    
    if (state == "main")
    {
        current_animation = &animations["main"];
        shield.coll_rect = { 0,0,0,0 };
        current_speed = base_speed;
    }

    if (state == "iframes")
    {
        if (IsIframesDone())
        {
            state = "main";
			current_animation = &animations["main"];
            last_iframes_start = SDL_GetTicks();
        }
        current_animation = &animations["iframes"];
    }

    if (state == "shield")
    {
        animations["shield"].Update();

        shield.shield_ready = false;
        current_speed = base_speed;
        int shield_width = 125;
        int shield_height = 125;
        shield.coll_rect = { player_dest_rect.x + (player_dest_rect.w/2) - shield_width/2, player_dest_rect.y + (player_dest_rect.h / 2) - (shield_height / 2), shield_width, shield_height };
        shield.dest_rect = { player_dest_rect.x, player_dest_rect.y, player_dest_rect.w, player_dest_rect.h };

        if (animations["shield"].IsFinished())
        {
            state = "main";
			
        }
    }

    if (state == "dash")
    {
        current_speed = dash_speed; // IF SPEED TIME IS UP STATE + MAIN
        if (IsDashDone())
        {
            state = "main";
        }
    }
    // Set animatino state.
    //if  idle_animation_index == idle_animation vector.size(), idle_animation_index == 0; else idle_animation_index ++

    // ITEM STATS UPDATE
    crit_percent = player_items.num_glass_toucans * 2 + 1;

    for (auto it = overlay_animations.begin(); it != overlay_animations.end();)
    {
        // Update the animation
        (*it)->Update();

        // Check if the animation is finished and not looping
        if ((*it)->IsFinished() && !(*it)->IsLooping())
        {
            // Erase the animation and update the iterator
            std::cout << "[*] Removing overlay animation\n";
            it = overlay_animations.erase(it);
        }
        else
        {
            // Move to the next animation
            ++it;
        }
    }

    

	current_animation->Update();


}

void Player::SetSecondaryFireMarkerActive(bool flag)
{
    secondary_fire.marker_active = flag;
}

bool Player::IsSecondaryFireMarkerActive()
{
    return secondary_fire.marker_active;
}

bool Player::IsFireSecondaryReady()
{
    Uint32 current_time = SDL_GetTicks();

    if ( (current_time - secondary_fire.last_fire_time) >= secondary_fire.cooldown_time_ms )
    {
        std::cout << "[*] SECONDARY FIRING !\n";
        secondary_fire.last_fire_time = current_time;
        return(true);
    }

    else
    {
        //std::cout << "[*] Secondary Fire on cooldown\n";
        return(false);
    }
}

void Player::ShootPrimaryFire(std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, ItemManager* item_manager)
{
    if (this->IsFirePrimaryReady())
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 100);

        // CRIT
        if (distrib(gen) <= this->GetCrit())
        {
            sound_manager.PlaySound("player_crit", 45);
            sound_manager.PlaySound("player_primary_fire", 55);
            game_projectiles.emplace_back(new PrimaryFire(player_dest_rect, 5.0, base_damage, 2, true));
        }
        // NORMAL
        else
        {
            sound_manager.PlaySound("player_primary_fire", 55);
            game_projectiles.emplace_back(new PrimaryFire(player_dest_rect, 5.0, base_damage, 2, false));
        }
    }
}

bool Player::IsFirePrimaryReady()
{
    Uint32 current_time = SDL_GetTicks();

    if ( (current_time - primary_fire.last_fire_time) >= primary_fire.cooldown_time_ms )
    {
        std::cout << "[*] PRIMARY FIRING !\n";
        primary_fire.last_fire_time = current_time;
        return(true);
    }

    else
    {
        //std::cout << "[*] Primary Fire on cooldown\n";
        return(false);
    }
}

bool Player::IsShieldReady()
{
    Uint32 current_time = SDL_GetTicks();

    if ((current_time - shield.last_time_used) >= shield.shield_cooldown_ms)
    {
        std::cout << "[*] SHIELD IS SHIELDING !\n";
        return(true);
    }

    else
    {
        std::cout << "[*] Shield on cooldown\n";
        return(false);
    }
}

void Player::SetImageScale(int scale)
{
   image_scale = scale;
}

void Player::SetPosition(int x, int y, int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    
    // Update position with boundary checks.
    if (player_dest_rect.x + x >= 0 && player_dest_rect.x + x <= SCREEN_WIDTH - player_dest_rect.w) 
    {
        player_dest_rect.x += x ;  // Move along x-axis within bounds
        player_coll_rect.x = player_dest_rect.x + (player_dest_rect.w / 2) - (player_coll_rect.w/2);
        secondary_fire.hud_dest_rect.x = player_dest_rect.x;
        secondary_fire.hud_coll_rect.x = player_dest_rect.x + (player_dest_rect.w / 2) - (secondary_fire.hud_coll_rect.w / 2);
    }
    if (player_dest_rect.y + y >= 0 && player_dest_rect.y + y <= SCREEN_HEIGHT - player_dest_rect.h) 
    {
        player_dest_rect.y += y ;  // Move along y-axis within bounds
        player_coll_rect.y = player_dest_rect.y + (player_dest_rect.h / 2) - (player_coll_rect.h/2);
        secondary_fire.hud_dest_rect.y = player_dest_rect.y - (256 + 64);
        secondary_fire.hud_coll_rect.y = player_dest_rect.y - (256 + 64) + (secondary_fire.hud_dest_rect.h / 2) - (secondary_fire.hud_coll_rect.h / 2);
    }

    if (secondary_fire.marker_active)
    {
        secondary_fire.marker_dest_rect.y += 1;
    }
}

void Player::AddItem(std::string item_name)
{
    if (item_name == "glass_toucan")
        player_items.num_glass_toucans++;
    if (item_name == "garnet_shield")
        player_items.num_garnet_shields++;
}

bool Player::IsIframesDone()
{
    Uint32 current_time = SDL_GetTicks();

    if ((current_time - last_iframes_start) >= iframes_duration)
    {
        return(true);
    }

    else
    {
        return(false);
    }
}

// Getters and Setters:
SDL_Rect* Player::GetCollRect()
{
    return &player_coll_rect;
}

SDL_Rect* Player::GetDstRect()  
{
    return &player_dest_rect;
}

float Player::GetSpeed()
{
    return current_speed;
}

void Player::SetSpeed(float speed) //NOT NEEDED?? SPEED IS MANAGED IN STATE MACHINE
{
    current_speed = speed;
}

void Player::ShootSecondaryFire(std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, ItemManager* item_manager)
{
    if (this->IsFireSecondaryReady())
    {
        this->SetSecondaryFireMarkerActive(true);
        this->SetSecondaryFireMarkerPosition();
        game_projectiles.emplace_back(new SecondaryFire(player_dest_rect, secondary_fire.speed, 4));

        //for (int i = 0; i < item_manager->GetItemList()->size(); i++)
        //{
        //    if (Collisions::RectRectCollision(&secondary_fire.hud_coll_rect, &item_manager->GetItemList()->at(i).item_dest_rect, false))
        //    {
        //        std::cout << "[*] Player shot an item!\n";
        //        this->AddItem(item_manager->GetItemList()->at(i).name);

        //        sound_manager.PlaySound("item_collection_sound", 55);
        //    }
        //}

        sound_manager.PlaySound("player_secondary_fire", 55);
    }
}

SDL_Rect* Player::GetSecondaryFirePosition()
{
    return &secondary_fire.hud_dest_rect;
}

float Player::GetSecondaryFireSpeed()
{
    return secondary_fire.speed;
}

void Player::SetSecondaryFireMarkerPosition()
{
    secondary_fire.marker_dest_rect = {secondary_fire.hud_dest_rect.x, secondary_fire.hud_dest_rect.y, BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
    secondary_fire.marker_col_rect = {secondary_fire.hud_dest_rect.x, secondary_fire.hud_dest_rect.y - (secondary_fire.hud_dest_rect.h / 2), BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
}

SDL_Rect* Player::GetSecondaryFireMarkerCollision()
{
    return &secondary_fire.marker_col_rect;
}

SDL_Rect* Player::GetSecondaryFireMarkerPosition()
{
    return &secondary_fire.marker_dest_rect;
}

SDL_Rect* Player::GetSecondaryFireHudColl()
{
    return &secondary_fire.hud_coll_rect;
}


// STAT GETTERS ==================================
float Player::GetBaseDamage()
{
    return(base_damage);
}

bool Player::CanParryHeal()
{
    if (SDL_GetTicks() - last_parry_heal_time >= parry_heal_buffer) 
    {
        last_parry_heal_time = SDL_GetTicks();
        return true;
    }
    return false;
}

int Player::GetHealth()
{
    return base_health;
}
float Player::GetCrit()
{
    return crit_percent;
}


std::string Player::GetPlayerState()
{
    return state;
}


void Player::SetShieldLastTimeUsed(Uint32 last_time_used)
{
    shield.last_time_used = last_time_used;
}

void Player::UpdatePlayerState(std::string new_state)
{
    state = new_state;
}


SDL_Rect* Player::GetShieldColl()
{
    return &shield.coll_rect;
}
SDL_Rect* Player::GetShieldDstRect()
{
    return &shield.dest_rect;
}


Uint32 Player::GetShieldLastTimeUsed()
{
    return shield.last_time_used;
}
Uint32 Player::GetShieldCooldown()
{
    return shield.shield_cooldown_ms;
}

float Player::GetDashSpeed()
{
    return dash_speed;
}
bool Player::IsDashReady()
{
    Uint32 current_time = SDL_GetTicks();

    if ((current_time - last_dash_time) >= dash_cooldown_ms)
    {
        std::cout << "[*] Dash is ready to use !\n";
        last_dash_time = current_time;
        return(true);
    }

    else
    {
        //std::cout << "[*] Secondary Fire on cooldown\n";
        return(false);
    }
}
bool Player::IsDashDone()
{
    Uint32 current_time = SDL_GetTicks();

    if ((current_time - last_dash_time) >= dash_duration)
    {
        std::cout << "[*] Dash is done !\n";
        last_dash_time = current_time;
        return(true);
    }

    else
    {
        //std::cout << "[*] Secondary Fire on cooldown\n";
        return(false);
    }
}

void Player::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{
    
    
    current_animation->Draw(renderer, player_dest_rect, SDL_FLIP_NONE);
    current_animation->OutputInformation();

	animations["secondary_fire_hud"].Draw(renderer, secondary_fire.hud_dest_rect, SDL_FLIP_NONE);

    if (secondary_fire.marker_active)
    {
        animations["secondary_fire_marker"].Draw(renderer, secondary_fire.marker_dest_rect, SDL_FLIP_NONE);
    }

    if (state == "shield")
    {
		animations["shield"].Draw(renderer, player_dest_rect, SDL_FLIP_NONE);
    }
    
    
    for (auto& animation : overlay_animations) 
    {  
		std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
        SDL_Rect* current_frame = animation->GetCurrentFrame();
        SDL_Rect temp = {   (player_dest_rect.x + player_dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2, 
                            (player_dest_rect.y + player_dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
                            current_frame->w,
                            current_frame->h };
        animation->Draw(renderer, temp, SDL_FLIP_NONE);
    }

    if (collision_box_flag)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &player_coll_rect);
        
        SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
        SDL_RenderDrawRect(renderer, &shield.coll_rect);
        SDL_RenderDrawRect(renderer, &secondary_fire.hud_coll_rect);
       
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &secondary_fire.marker_col_rect);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &player_dest_rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
}

int Player::GetNumItem(std::string item_name)
{
    if (item_name == "glass_toucan")
        return player_items.num_glass_toucans;
    if (item_name == "garnet_shield")
        return player_items.num_garnet_shields;
}


void Player::Heal(int recovery_amount, SoundManager& sound_manager)
{

    std::cout << "[*] Player healed for " << recovery_amount << " health points.============================================================================================================\n";
    sound_manager.PlaySound("player_heal", 80);
    overlay_animations.push_back(std::make_unique<Animation>(animations["heal"]));
    base_health += recovery_amount;

}
void Player::Hurt(int damage, SoundManager& sound_manager)
{
    sound_manager.PlaySound("player_hurt", 80);
    base_health -= damage;
}