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

    health_bar_animation = animation_manager_arg.Get("ui", "health_bar");
    shield_bar_animation = animation_manager_arg.Get("ui", "shield_bar");
    dash_bar_animation = animation_manager_arg.Get("ui", "dash_bar");
    slash_bar_animation = animation_manager_arg.Get("ui", "slash_bar");
    health_bar_base_animation = animation_manager_arg.Get("ui", "health_bar_base");
    shield_ready_animation = animation_manager_arg.Get("ui", "shield_ready");

    state = "main";
    bool invincible = false;


    player_dest_rect = {0, 0,  BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
    player_coll_shape.circle.r = BASE_SPRITE_SIZE * image_scale / 4;

	player_coll_shape.type = ColliderType::CIRCLE;
    player_coll_shape.circle.x = player_dest_rect.x + player_dest_rect.w / 2;
    player_coll_shape.circle.y = player_dest_rect.y + player_dest_rect.h / 2;


    // --- Tunable parameters ---
    float velocity = 1.0f;
    // --- Tunable parameters ---
    const float stiffness = 2.0f;     // increase after normalization
    const float damping = 0.7f;
    const float hover_amp = 1.5f;
    const float hover_freq = 0.07f;
    // In your class:
    float posX = 0.0f; // store precise position




    // SECONDARY FIRE COLL
    secondary_fire.source_rect = {32, 32, 32, 32}; 
    secondary_fire.hud_dest_rect  = {player_dest_rect.x, player_dest_rect.y, BASE_SPRITE_SIZE * image_scale, BASE_SPRITE_SIZE * image_scale};
    secondary_fire.hud_coll_rect.type = ColliderType::RECT;
    secondary_fire.marker_active = false;
    secondary_fire.ready = true;
    shield.shield_ready = true;
}


void Player::Update(int SCREEN_WIDTH, int SCREEN_HEIGHT, long loop_flag, Uint32 tick, SoundManager& sound_manager)
{
    
    Move(SCREEN_WIDTH, SCREEN_HEIGHT);
	//std::cout << "[*] Player STATE: " << state << std::endl;
    
    if (!secondary_fire.marker_active) secondary_fire.marker_col_rect.rect = { 0,0,0,0 };
    
    
    if (state != "shield" && shield.shield_ready == false && IsShieldReady())
    {
        shield.shield_ready = true;
		std::cout << "[*] SHIELD IS READY !\n";

		
        animation_manager.Get("zephyr", "shield")->Reset();
        animation_manager.Get("overlays", "shield_ready")->Reset();
        overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "shield_ready")));

		sound_manager.PlaySound("shield_ready", 80);
    }
    
    if (state == "main")
    {
        current_animation = animation_manager.Get("zephyr", "main");
        shield.coll_shape.circle.r = 0;

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

    if (state == "slash" and slashing_attack.slash_projectile->current_animation->IsFinished())
    {
				state = "main";						
    }

    if (state == "shield")
    {
        shield.coll_shape.circle.r = shield.dest_rect.w / 2;
        animation_manager.Get("zephyr","shield")->Update();

        shield.shield_ready = false;
        current_speed = base_speed;
        int shield_width = 125;
        int shield_height = 125;
        shield.coll_shape.circle.x = player_dest_rect.x + (player_dest_rect.w / 2);
        shield.coll_shape.circle.y = player_dest_rect.y + (player_dest_rect.h / 2);
        
        shield.dest_rect = { player_dest_rect.x, player_dest_rect.y, player_dest_rect.w, player_dest_rect.h };

        if (SDL_GetTicks() - this->GetShieldLastTimeUsed() >= shield.shield_duration_ms)
        {
            state = "main";
			
        }
    }

    if (state == "dash")
    {
		if (!dash_overlay_added)
		{
            dash_overlay_added = true;
            overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "dash"), Animation::Order::BACK));
            dash_overlay_dest_rect = player_dest_rect;
            
            double angle = 0.0;
            if (vx != 0 || vy != 0)
            {
                angle = atan2(vy, vx) * 180.0 / M_PI;
            }
			dash_overlay_angle = static_cast<float>(angle);
		}
        
        accel = dash_accel;
        current_speed = dash_speed; // IF SPEED TIME IS UP STATE + MAIN
        if (IsDashDone())
        {
            accel = base_accel;
            state = "main";
            dash_overlay_added = false;
        }
    }

    

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

    
    if (this->IsShieldReady()) { shield.shield_ready = true; }
	current_animation->Update();

    if (secondary_fire.marker_active)
        secondary_fire.marker_col_rect.rect.y += 1;


}

void Player::Move(int SCREEN_WIDTH, int SCREEN_HEIGHT)
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

void Player::DoSlash(std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, bool left_flag)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 100);

    bool critical = false;
    // CRIT
	if (distrib(gen) <= this->GetCrit())
	{
		critical = true;
        
	}
    
    
    if (IsSlashReady())
    {
		if (critical)
            sound_manager.PlaySound("player_slash_crit", 55);
        state = "slash";
        slashing_attack.last_fire_time = SDL_GetTicks();
        slashing_attack.slash_projectile = new Slash(animation_manager, player_dest_rect, slashing_attack.damage, 1, critical, left_flag);
        


		game_projectiles.emplace_back(slashing_attack.slash_projectile);
		sound_manager.PlaySound("player_slash", 55);
    }
}

bool Player::IsSlashReady()
{
    Uint32 current_time = SDL_GetTicks();

    if ((current_time - slashing_attack.last_fire_time) >= slashing_attack.cooldown_time_ms)
    {
        std::cout << "[*] SLASH FIRING !\n";
        return(true);
    }

    else
    {
        //std::cout << "[*] Secondary Fire on cooldown\n";
        return(false);
    }
}

void Player::DoShield(SoundManager& sound_manager, Projectile* projectile, bool render_coll_boxes, OverlayTextManager* overlay_text_manager)
{
    sound_manager.PlaySound("player_shield_hit", 45);
    projectile->UpdateState("impact");
    sound_manager.PlaySound(projectile->GetSoundEffectImpact(), 25);

    if (this->GetNumItem("garnet_shield") > 0 && this->CanParryHeal())
    {
        int heal_amount = this->GetNumItem("garnet_shield") * 5;
        this->Heal(heal_amount, sound_manager);
        if (render_coll_boxes)
        {
            overlay_text_manager->AddMessage(std::to_string(heal_amount),
                { 44, 214, 58, 255 }, //green color
                this->GetDstRect(),
                350); //ms
        }
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
            sound_manager.PlaySound("player_crit", 40);
            sound_manager.PlaySound("player_primary_fire", 30);
            game_projectiles.emplace_back(new PrimaryFire(animation_manager, player_dest_rect, primary_fire.bullet_speed, base_damage, 2, true));
        }
        // NORMAL
        else
        {
            sound_manager.PlaySound("player_primary_fire", 30);
            game_projectiles.emplace_back(new PrimaryFire(animation_manager, player_dest_rect, primary_fire.bullet_speed, base_damage, 2, false));
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

    player_coll_shape.circle.x = player_dest_rect.x + (player_dest_rect.w / 2);
    player_coll_shape.circle.y = player_dest_rect.y + (player_dest_rect.h / 2);

    secondary_fire.hud_dest_rect.x = player_dest_rect.x;
    secondary_fire.hud_dest_rect.y = player_dest_rect.y - (80);

	int new_hud_width = secondary_fire.hud_dest_rect.w / 4;
    int new_hud_height = secondary_fire.hud_dest_rect.h / 4;
    secondary_fire.hud_coll_rect.rect = {secondary_fire.hud_dest_rect.x + secondary_fire.hud_dest_rect.w / 2 - new_hud_width / 2,
                                        secondary_fire.hud_dest_rect.y + secondary_fire.hud_dest_rect.h / 2 - new_hud_height / 2 ,
                                        new_hud_width,
                                        new_hud_height};


    if (secondary_fire.marker_active)
    {
        secondary_fire.marker_pos_y += secondary_fire.item_speed;

        // Move marker rect
        secondary_fire.marker_dest_rect.y = static_cast<int>(secondary_fire.marker_pos_y);

        // Keep collision rect centered on marker
        secondary_fire.marker_col_rect.rect.y =
            static_cast<int>(secondary_fire.marker_pos_y + (BASE_SPRITE_SIZE * image_scale) / 2 - (secondary_fire.marker_col_rect.rect.h / 2));
    }
}

void Player::AddItem(std::string item_name)
{
    if (item_name == "glass_toucan")
    {
        crit_percent += 2.0f;
        player_items.num_glass_toucans++;
    }
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
Collider* Player::GetCollShape()
{
    return &player_coll_shape;
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
    
    // Get center of HUD
    int hud_center_x = secondary_fire.hud_dest_rect.x + secondary_fire.hud_dest_rect.w / 2;
    int hud_center_y = secondary_fire.hud_dest_rect.y + secondary_fire.hud_dest_rect.h / 2;

    // Set marker rect so it's centered
    secondary_fire.marker_dest_rect = {
        hud_center_x - (BASE_SPRITE_SIZE * image_scale) / 2,
        hud_center_y - (BASE_SPRITE_SIZE * image_scale) / 2,
        BASE_SPRITE_SIZE * image_scale,
        BASE_SPRITE_SIZE * image_scale
    };

    // Marker collision rect
    int hud_coll_w = BASE_SPRITE_SIZE / 8;
    int hud_coll_h = BASE_SPRITE_SIZE / 8;
    secondary_fire.marker_col_rect.rect = {
        hud_center_x - hud_coll_w / 2,
        hud_center_y - hud_coll_h / 2,
        hud_coll_w,
        hud_coll_h
    };

    // Store posY as the top-left of the marker for movement
    secondary_fire.marker_pos_y = secondary_fire.marker_dest_rect.y;
    
}

Collider* Player::GetSecondaryFireMarkerCollision()
{
    return &secondary_fire.marker_col_rect;
}

SDL_Rect* Player::GetSecondaryFireMarkerPosition()
{
    return &secondary_fire.marker_dest_rect;
}

Collider* Player::GetSecondaryFireHudColl()
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
    if (new_state == "dash")
    {
		last_dash_time = SDL_GetTicks();
    }
    if (new_state == "shield")
    {
		shield.last_time_used = SDL_GetTicks();

    }
}


Collider* Player::GetShieldColl()
{
    return &shield.coll_shape;
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
        return(true);
    }

    else
    {
        //std::cout << "[*] Secondary Fire on cooldown\n";
        return(false);
    }
}

void Player::Draw(SDL_Renderer* renderer, bool collision_box_flag, int screen_width, int screen_height)
{
    
    
    for (auto& animation : overlay_animations)
    {
        if (animation->GetOrder() == Animation::Order::BACK)
        {
            std::cout << "DRAWING OVERLAY !" << std::endl;
            if (animation->GetName() == "overlays-dash")
            {
                animation->DrawRotated(renderer, dash_overlay_dest_rect, SDL_FLIP_NONE, dash_overlay_angle);
            }

            SDL_Rect* current_frame = animation->GetCurrentFrame();
            SDL_Rect temp = { player_dest_rect.x + (player_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
                                player_dest_rect.y + (player_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
                                current_frame->w * animation->GetScale(),
                                current_frame->h * animation->GetScale() };
            animation->Draw(renderer, temp, SDL_FLIP_NONE);
        }
    }
    if (state == "shield")
    {
        SDL_Rect temp = { player_dest_rect.x - (animation_manager.Get("zephyr", "shield")->GetFrameWidth() * animation_manager.Get("zephyr", "shield")->GetScale() - player_dest_rect.w) / 2,
                          player_dest_rect.y - (animation_manager.Get("zephyr", "shield")->GetFrameWidth() * animation_manager.Get("zephyr", "shield")->GetScale() - player_dest_rect.h) / 2,
            animation_manager.Get("zephyr", "shield")->GetFrameWidth() * animation_manager.Get("zephyr", "shield")->GetScale(),
                          animation_manager.Get("zephyr", "shield")->GetFrameWidth() * animation_manager.Get("zephyr", "shield")->GetScale() };
        animation_manager.Get("zephyr", "shield")->Draw(renderer, temp, SDL_FLIP_NONE);
    }
    current_animation->Draw(renderer, player_dest_rect, SDL_FLIP_NONE);
    current_animation->OutputInformation();

	animation_manager.Get("zephyr", "secondary_fire_hud")->Draw(renderer, secondary_fire.hud_dest_rect, SDL_FLIP_NONE);

    if (secondary_fire.marker_active)
    {
        animation_manager.Get("zephyr", "secondary_fire_marker")->Draw(renderer, secondary_fire.marker_dest_rect, SDL_FLIP_NONE);
    }

    
    
    
    for (auto& animation : overlay_animations) 
    {  
        if (animation->GetOrder() == Animation::Order::FRONT)
        {
            SDL_Rect* current_frame = animation->GetCurrentFrame();
            SDL_Rect temp = { player_dest_rect.x + (player_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
                                player_dest_rect.y + (player_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
                                current_frame->w * animation->GetScale(),
                                current_frame->h * animation->GetScale() };
            animation->Draw(renderer, temp, SDL_FLIP_NONE);
        }
    }
    
    auto clamp = [](float value, float min, float max) {
        return (value < min) ? min : (value > max ? max : value);
        };

    auto lerp = [](float a, float b, float t) {
        return a + (b - a) * t;
        };

    // Store smoothed values as static to persist between frames
    static float smooth_health = 1.0f;
    static float smooth_shield = 1.0f;
    static float smooth_dash = 1.0f;
	static float smooth_slash = 1.0f;
    const float SMOOTHING_SPEED = 0.1f; // lower = smoother, higher = snappier


    if (shield.shield_ready)
	{
        float icon_offset_x = 0;
        float icon_offset_y = -15;
        SDL_Rect shield_ready_dst = {
			secondary_fire.hud_dest_rect.x + secondary_fire.hud_dest_rect.w / 2 - (shield_ready_animation->GetFrameWidth() * shield_ready_animation->GetScale() / 2),
            (secondary_fire.hud_dest_rect.y + secondary_fire.hud_dest_rect.h / 2 - (shield_ready_animation->GetFrameHeight() * shield_ready_animation->GetScale() / 2)) + icon_offset_y,
			shield_ready_animation->GetFrameWidth() * shield_ready_animation->GetScale(),
			shield_ready_animation->GetFrameHeight() * shield_ready_animation->GetScale()
		};
		shield_ready_animation->Draw(renderer, shield_ready_dst);
	}

    float ui_offset = screen_height * 0.05;
    
    // --- HEALTH BAR ---
    float health_percent = clamp(static_cast<float>(current_health) / static_cast<float>(max_health), 0.0f, 1.0f);
    smooth_health = lerp(smooth_health, health_percent, SMOOTHING_SPEED);

    SDL_Rect health_bar_dst = {
        0,
        screen_height - health_bar_animation->GetFrameHeight() * health_bar_animation->GetScale() - 1 * ui_offset,
        health_bar_animation->GetFrameWidth() * health_bar_animation->GetScale(),
        health_bar_animation->GetFrameHeight() * health_bar_animation->GetScale()
    };

    health_bar_base_animation->Draw(renderer, health_bar_dst);
    health_bar_animation->DrawPartial(renderer, health_bar_dst, smooth_health, static_cast<float>(health_bar_animation->GetFrameWidth() / health_bar_animation->GetScale()), SDL_FLIP_NONE);

    // --- SHIELD BAR ---
    float shield_percentage = clamp(
        static_cast<float>(SDL_GetTicks() - shield.last_time_used) / static_cast<float>(shield.shield_cooldown_ms),
        0.0f, 1.0f
    );
    smooth_shield = lerp(smooth_shield, shield_percentage, SMOOTHING_SPEED);

    SDL_Rect shield_bar_dst = {
        0,
        screen_height - shield_bar_animation->GetFrameHeight() * health_bar_animation->GetScale() - 2*ui_offset,
        shield_bar_animation->GetFrameWidth(),
        shield_bar_animation->GetFrameHeight()
    };

    health_bar_base_animation->Draw(renderer, shield_bar_dst);
    shield_bar_animation->DrawPartial(renderer, shield_bar_dst, smooth_shield, static_cast<float>(shield_bar_animation->GetFrameWidth() / shield_bar_animation->GetScale()), SDL_FLIP_NONE);



    // --- DASH BAR ---
    float dash_percentage = clamp(
        static_cast<float>(SDL_GetTicks() - last_dash_time) / static_cast<float>(dash_cooldown_ms),
        0.0f, 1.0f
    );
    smooth_dash = lerp(smooth_dash, dash_percentage, SMOOTHING_SPEED);

    SDL_Rect dash_bar_dst = {
        0,
        screen_height - dash_bar_animation->GetFrameHeight() * health_bar_animation->GetScale() - 3 * ui_offset,
        dash_bar_animation->GetFrameWidth(),
        dash_bar_animation->GetFrameHeight()
    };

    health_bar_base_animation->Draw(renderer, dash_bar_dst);
    dash_bar_animation->DrawPartial(renderer, dash_bar_dst, smooth_dash, static_cast<float>(dash_bar_animation->GetFrameWidth()/dash_bar_animation->GetScale()), SDL_FLIP_NONE);

    // --- SLASH BAR ---
    float slash_percentage = clamp(
        static_cast<float>(SDL_GetTicks() - slashing_attack.last_fire_time) / static_cast<float>(slashing_attack.cooldown_time_ms),
        0.0f, 1.0f
    );
    smooth_slash = lerp(smooth_slash, slash_percentage, SMOOTHING_SPEED);

    SDL_Rect slash_bar_dst = {
        0,
        screen_height - slash_bar_animation->GetFrameHeight() * health_bar_animation->GetScale() - 4 * ui_offset,
        slash_bar_animation->GetFrameWidth(),
        slash_bar_animation->GetFrameHeight()
    };

    health_bar_base_animation->Draw(renderer, slash_bar_dst);
    slash_bar_animation->DrawPartial(renderer, slash_bar_dst, smooth_slash, static_cast<float>(slash_bar_animation->GetFrameWidth() / slash_bar_animation->GetScale()), SDL_FLIP_NONE);

    

    if (collision_box_flag)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		Collisions::DrawCircle(renderer, player_coll_shape.circle);
        
        SDL_SetRenderDrawColor(renderer, 255, 150, 56, 255);
        Collisions::DrawCircle(renderer, shield.coll_shape.circle);
        SDL_RenderDrawRect(renderer, &secondary_fire.hud_coll_rect.rect);
       
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &secondary_fire.marker_col_rect.rect);

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

void Player::AddOverlayAnimation(Animation* animation)
{
    overlay_animations.emplace_back(std::make_unique<Animation>(*animation));
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

void Player::ResetPlayer(int window_width, int window_height)
{
	current_health = max_health;
	posX = window_width / 2 - player_dest_rect.w / 2;
	posY = window_height * 3 / 4;
	player_dest_rect.x = static_cast<int>(posX);
	player_dest_rect.y = static_cast<int>(posY);

    // Reset Stats
    max_health = BASE_HEALTH;
	current_health = max_health;
	base_speed = BASE_SPEED;
	base_damage = BASE_DAMAGE;

	crit_percent = BASE_CRIT;

    // Reset ITems
    player_items.num_glass_toucans = 0;
    player_items.num_garnet_shields = 0;

	shield.last_time_used = SDL_GetTicks() - shield.shield_cooldown_ms;
	last_dash_time = SDL_GetTicks() - dash_cooldown_ms;
	slashing_attack.last_fire_time = SDL_GetTicks() - slashing_attack.cooldown_time_ms;
	primary_fire.last_fire_time = SDL_GetTicks() - primary_fire.cooldown_time_ms;
}