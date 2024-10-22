#include "Player.hpp"
#include <iostream>
#include "Graphics.hpp"

Player::Player(Graphics* graphics_manager, int PIXEL_SCALE)
{
    
    std::cout << "[*] I am a new player\n";
    image_scale = PIXEL_SCALE;
    player_dest_rect = {0, 0, BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
    player_coll_rect = { 0, 0, BASE_SPRITE_SIZE*3, BASE_SPRITE_SIZE };

    base_speed = 5.0;
    dash_speed = 10.2;
    base_damage = 10;
    base_health = 150;
    crit_percent = 1.0; // Percentage

    state = "main";
    current_frames = main_frames;
    bool invincible = false;

   
    
    secondary_fire.source_rect = {32, 32, 32, 32}; 
    secondary_fire.hud_dest_rect  = {player_dest_rect.x, player_dest_rect.y, BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
    //secondary_fire.hud_coll_rect = { (player_dest_rect.x + (player_dest_rect.w / 2) - (32 / 2)), player_dest_rect.y, 32, 32 };
    secondary_fire.hud_coll_rect = { player_dest_rect.x, player_dest_rect.y, BASE_SPRITE_SIZE, BASE_SPRITE_SIZE };
    secondary_fire.marker_active = false;

}


void Player::Update(int x_pos, int y_pos, int SCREEN_WIDTH, int SCREEN_HEIGHT, long loop)
{
    SetPosition(x_pos, y_pos, SCREEN_WIDTH, SCREEN_HEIGHT);

    if (!secondary_fire.marker_active)
        secondary_fire.marker_col_rect = { 0,0,0,0 };
    
    
    if (state == "main")
    {
        shield.coll_rect = { 0,0,0,0 };
        current_speed = base_speed;
    }

    if (state == "shield")
    {
        current_speed = base_speed;
        int shield_width = 125;
        int shield_height = 125;
        shield.coll_rect = { player_dest_rect.x + (player_dest_rect.w/2) - shield_width/2, player_dest_rect.y + (player_dest_rect.h / 2) - (shield_height / 2), shield_width, shield_height };
        shield.dest_rect = { player_dest_rect.x, player_dest_rect.y, player_dest_rect.w, player_dest_rect.h };

        if (shield.shield_frame_index >= shield.shield_frames.size() - 1)
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
        shield.last_time_used = current_time;
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

float Player::GetBaseDamage()
{
    return(base_damage);
}

int Player::GetHealth()
{
    return base_health;
}
float Player::GetCrit()
{
    return crit_percent;
}

std::string Player::GetTextureKey()
{
    return current_texture_key;
}
Uint32 Player::GetFrameTime()
{
    return frame_time_ms;
}
Uint32 Player::GetLastFrameStart()
{
    return last_frame_time;
}
SDL_Rect* Player::GetFrame()
{
    return &current_frames.at(current_frame_index);
}

std::string Player::GetPlayerState()
{
    return state;
}

void Player::SetShieldLastFrameTime(Uint32 current_time)
{
    shield.last_shield_frame_time = current_time;
}

void Player::UpdatePlayerState(std::string new_state)
{
    state = new_state;
    current_frame_index = 0;
    shield.shield_frame_index = 0;
}

SDL_Rect* Player::GetShieldFrame()
{
    return &shield.shield_frames[shield.shield_frame_index];
}
SDL_Rect* Player::GetShieldColl()
{
    return &shield.coll_rect;
}
SDL_Rect* Player::GetShieldDstRect()
{
    return &shield.dest_rect;
}
int Player::GetShieldNumFrames()
{
    return shield.shield_frames.size();
}

Uint32 Player::GetShieldFrameTime()
{
    return shield.shield_frame_time_ms;
}
Uint32 Player::GetLastShieldFrameStart()
{
    return shield.last_shield_frame_time;
}
Uint32 Player::GetShieldLastTimeUsed()
{
    return shield.last_time_used;
}
Uint32 Player::GetShieldCooldown()
{
    return shield.shield_cooldown_ms;
}

void Player::AdvanceShieldFrame()
{
    shield.shield_frame_index++;
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

int Player::NumOfFrames()
{
    return current_frames.size();
}

void Player::SetLastFrameTime(Uint32 current_time)
{
    last_frame_time = current_time;
}

int Player::GetFrameIndex()
{
    return current_frame_index;
}

void Player::AdvanceFrame()
{
    current_frame_index++;
}

void Player::SetFrameIndex(int index)
{
    current_frame_index = 0;
}

void Player::ChangeHealth(float health_modifier)
{
    base_health += health_modifier;
}