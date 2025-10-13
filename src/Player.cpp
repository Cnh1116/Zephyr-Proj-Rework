#include <iostream>
#include <random>

#include "Player.hpp"
#include "Graphics.hpp"
#include "AnimationManager.hpp"
#include "Collisions.hpp"
#include "Sound.hpp"

Player::Player(int PIXEL_SCALE, AnimationManager& animation_manager_arg)
    : animation_manager(animation_manager_arg)
{
    std::cout << "[*] I am a new player\n";
    image_scale = PIXEL_SCALE;
    

    current_animation = animation_manager_arg.Get("zephyr", "main");

    state = "main";
    bool invincible = false;

    player_dest_rect = {0, 0, BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
    player_coll_rect = { 0, 0, static_cast<int>(BASE_SPRITE_SIZE * 2.75), BASE_SPRITE_SIZE };


    // --- Tunable parameters ---
    float velocity = 1.0f;
    // --- Tunable parameters ---
    const float stiffness = 2.0f;     // increase after normalization
    const float damping = 0.7f;
    const float hover_amp = 1.5f;
    const float hover_freq = 0.07f;
    // In your class:
    float posX = 0.0f; // store precise position



    
    secondary_fire.source_rect = {32, 32, 32, 32}; 
    secondary_fire.hud_dest_rect  = {player_dest_rect.x, player_dest_rect.y, BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
    secondary_fire.hud_coll_rect = { player_dest_rect.x, player_dest_rect.y, BASE_SPRITE_SIZE, BASE_SPRITE_SIZE };
    secondary_fire.marker_active = false;
    secondary_fire.ready = true;
}


void Player::Update(float dx, float dy, int SCREEN_WIDTH, int SCREEN_HEIGHT, long loop_flag, Uint32 tick, SoundManager& sound_manager)
{
    
    Move(dx, dy, SCREEN_WIDTH, SCREEN_HEIGHT);
	//std::cout << "[*] Player STATE: " << state << std::endl;
    
    if (!secondary_fire.marker_active) secondary_fire.marker_col_rect = { 0,0,0,0 };
    
    
    if (state != "shield" && shield.shield_ready == false && IsShieldReady())
    {
        shield.shield_ready = true;
		std::cout << "[*] SHIELD IS READY !\n";

		
        animation_manager.Get("zephyr", "shield")->Reset();
        animation_manager.Get("overlays", "shield_ready")->Reset();
        overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "shield_ready")));

		sound_manager.PlaySound("jade_drum", 55);
    }
    
    if (state == "main")
    {
        current_animation = animation_manager.Get("zephyr", "main");
        shield.coll_rect = { 0,0,0,0 };
        current_speed = base_speed;
    }

    if (state == "iframes")
    {
        current_speed = base_speed * 0.8;
        if (IsIframesDone())
        {
            state = "main";
            int prev_frame_index = animation_manager.Get("zephyr", "main")->GetCurrentFrameIndex();
			current_animation = animation_manager.Get("zephyr", "main");
            current_animation->SetCurrentFrameIndex(prev_frame_index);
        }
        else if (current_animation != animation_manager.Get("zephyr", "iframes")) 
        {
            animation_manager.Get("zephyr", "iframes")->Reset();
            current_animation = animation_manager.Get("zephyr", "iframes");
        }
    }

    if (state == "shield")
    {
        animation_manager.Get("zephyr","shield")->Update();

        shield.shield_ready = false;
        current_speed = base_speed;
        int shield_width = 125;
        int shield_height = 125;
        shield.coll_rect = { player_dest_rect.x + (player_dest_rect.w/2) - shield_width/2, player_dest_rect.y + (player_dest_rect.h / 2) - (shield_height / 2), shield_width, shield_height };
        shield.dest_rect = { player_dest_rect.x, player_dest_rect.y, player_dest_rect.w, player_dest_rect.h };

        if (animation_manager.Get("zephyr", "shield")->IsFinished())
        {
            state = "main";
			
        }
    }

    if (state == "dash")
    {
        accel = dash_accel;
        current_speed = dash_speed; // IF SPEED TIME IS UP STATE + MAIN
        if (IsDashDone())
        {
            accel = base_accel;
            state = "main";
        }
    }

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

void Player::Move(float input_dx, float input_dy, int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    // Compute desired velocities
    float target_vx = input_dx * current_speed;
    float target_vy = input_dy * current_speed;

    // Smoothly move current velocity toward the target velocity
    vx += (target_vx - vx) * accel;
    vy += (target_vy - vy) * accel;

    // If there is no directional intent, apply deceleration to come to rest
    if (fabsf(input_dx) < 0.0001f) {
        vx *= (1.0f - decel);
        if (fabsf(vx) < 0.05f) vx = 0.0f;
    }
    if (fabsf(input_dy) < 0.0001f) {
        vy *= (1.0f - decel);
        if (fabsf(vy) < 0.05f) vy = 0.0f;
    }

    // --- Update position ---
    SetPosition((int)vx, (int)vy, SCREEN_WIDTH, SCREEN_HEIGHT);
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
            game_projectiles.emplace_back(new PrimaryFire(animation_manager, player_dest_rect, 5.0, base_damage, 2, true));
        }
        // NORMAL
        else
        {
            sound_manager.PlaySound("player_primary_fire", 55);
            game_projectiles.emplace_back(new PrimaryFire(animation_manager, player_dest_rect, 5.0, base_damage, 2, false));
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

void Player::SetPosition(float dx, float dy, int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    
    // Update internal float positions
    posX += dx;
    posY += dy;

    // Clamp within bounds
    if (posX < 0) posX = 0;
    if (posX > SCREEN_WIDTH - player_dest_rect.w)
        posX = SCREEN_WIDTH - player_dest_rect.w;

    if (posY < 0) posY = 0;
    if (posY > SCREEN_HEIGHT - player_dest_rect.h)
        posY = SCREEN_HEIGHT - player_dest_rect.h;

    // Update rectangles based on new position
    player_dest_rect.x = static_cast<int>(posX);
    player_dest_rect.y = static_cast<int>(posY);

    player_coll_rect.x = player_dest_rect.x + (player_dest_rect.w / 2) - (player_coll_rect.w / 2);
    player_coll_rect.y = player_dest_rect.y + (player_dest_rect.h / 2) - (player_coll_rect.h / 2);

    secondary_fire.hud_dest_rect.x = player_dest_rect.x;
    secondary_fire.hud_dest_rect.y = player_dest_rect.y - (256 + 64);

    secondary_fire.hud_coll_rect.x = player_dest_rect.x + (player_dest_rect.w / 2) - (secondary_fire.hud_coll_rect.w / 2);
    secondary_fire.hud_coll_rect.y = player_dest_rect.y - (256 + 64) + (secondary_fire.hud_dest_rect.h / 2) - (secondary_fire.hud_coll_rect.h / 2);


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
        game_projectiles.emplace_back(new SecondaryFire(animation_manager, player_dest_rect, secondary_fire.speed, 4));

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

int Player::GetMaxHealth()
{
    return max_health;
}

int Player::GetCurrentHealth()
{
    return current_health;
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
    if (new_state == "iframes")
    {
		int prev_frame_index = current_animation->GetCurrentFrameIndex();
        last_iframes_start = SDL_GetTicks();
		current_animation = animation_manager.Get("zephyr", "iframes");
		current_animation->SetCurrentFrameIndex(prev_frame_index);
    }
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

	animation_manager.Get("zephyr", "secondary_fire_hud")->Draw(renderer, secondary_fire.hud_dest_rect, SDL_FLIP_NONE);

    if (secondary_fire.marker_active)
    {
        animation_manager.Get("zephyr", "secondary_fire_marker")->Draw(renderer, secondary_fire.marker_dest_rect, SDL_FLIP_NONE);
    }

    if (state == "shield")
    {
		animation_manager.Get("zephyr", "shield")->Draw(renderer, player_dest_rect, SDL_FLIP_NONE);
    }
    
    
    for (auto& animation : overlay_animations) 
    {  
		std::cout << "DRAWING OVERLAY !" << std::endl;
        SDL_Rect* current_frame = animation->GetCurrentFrame();
        SDL_Rect temp = {   player_dest_rect.x + (player_dest_rect.w - current_frame->w *  animation->GetScale()) / 2, 
                            player_dest_rect.y + (player_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
                            current_frame->w * animation->GetScale(),
                            current_frame->h * animation->GetScale() };
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

    sound_manager.PlaySound("player_heal", 80);
    overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "heal")));
    current_health += recovery_amount;

}
void Player::Hurt(int damage, SoundManager& sound_manager)
{
    std::cout << "[*] Player hurt for " << damage << " damage points.\n";
    sound_manager.PlaySound("player_hurt", 100);
    current_health -= damage;
}