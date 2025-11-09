
#include <SDL.h>
#include <random>
#include <iostream>
#include "Enemy.hpp"
#include "Player.hpp"
#include "AnimationManager.hpp"
#include <Projectiles.hpp>
#include "Sound.hpp"

Enemy::Enemy(AnimationManager& animation_manager, const SDL_Rect& dest_rect, const Collider& coll_shape, float move_speed, int health_arg, float crit, float start_damage)
	: animation_manager(animation_manager)
{
	enemy_dest_rect = dest_rect;
	enemy_coll_shape = coll_shape;

	movement_speed = move_speed;
	base_health = health_arg;
	base_damage = start_damage;

	points = 0;

	invincible = false;
	state = "main";
	

	// IS SHINY ?
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0f, 100.0f);
	float roll = dist(gen);
	shiny = roll < SHINY_CHANCE;

	if (shiny)
		this->AddOverlayAnimation(new Animation(*animation_manager.Get("overlays", "shiny")));
	
}
void Enemy::AddOverlayAnimation(Animation* animation)
{
	overlay_animations.emplace_back(std::make_unique<Animation>(*animation));
}

// Setters and Getters
 Collider* Enemy::GetCollShape()
{
	return &enemy_coll_shape;
}
SDL_Rect* Enemy::GetDstRect()
{
	return &enemy_dest_rect;
}

void Enemy::UpdateState(std::string new_state)
{
	state = new_state;
}

int Enemy::GetPoints()
{
	return points;
}

std::string Enemy::GetState()
{
	return(state);
}


int Enemy::GetHealth()
{
	return base_health;
}
void Enemy::ChangeHealth(int health_diff)
{
	base_health += health_diff;
}

bool Enemy::IsDoneAttacking()
{
	Uint32 current_time = SDL_GetTicks();
	if ((current_time - last_fire_time) >= fire_cooldown_ms)
	{
		return(true);
	}

	else
	{
		//std::cout << "[*] Frame is not done\n";
		return(false);
	}
}





// ICE CRYSTAL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
IceCrystal::IceCrystal(AnimationManager& animation_manager, const SDL_Rect& dest_rect)
	: Enemy(animation_manager, dest_rect, Collider(0,0,0), 1.0, 100, 0, 35)
{
	
	
	state = "idle";
	if (shiny)
		current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-ice-crystal", "main_shiny"));
	else
		current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-ice-crystal", "main"));

	enemy_dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
	enemy_dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();

	enemy_coll_shape.type = ColliderType::CIRCLE;
	enemy_coll_shape.circle.x = enemy_dest_rect.x + (enemy_dest_rect.w / 2);
	enemy_coll_shape.circle.y = enemy_dest_rect.y + (enemy_dest_rect.h / 2);
	enemy_coll_shape.circle.r = 5;
	fire_cooldown_ms = 300;
	points = 10;
	
}

void IceCrystal::Update(Player *player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, int screen_width, int screen_height)
{
	
	//std::cout << "ICE STATE: " << "---------------------------------------------------" << state << std::endl;
	
	if (state == "idle")
	{
		if (shiny and !shiny_sound_played)
		{
			sound_manager.PlaySound("shiny", 50);
			shiny_sound_played = true;
		}

		velocity = 0.0f;
		
		int lateral_diff = abs(enemy_dest_rect.x - player->GetDstRect()->x);
		if (lateral_diff > distance_to_swing)
		{
			do_swing = true;
			step_angle = BASE_STEP_ANGLE;
		}

		if (lateral_diff <= player_distance_threshold)
		{
			state = "shoot";
			num_proj_shot = 0;
			last_fire_time = SDL_GetTicks();
		}
		else
		{
			old_target_x = target_x;
			std::cout << "[*] Player X: " << player->GetDstRect()->x << " Enemy X: " << enemy_dest_rect.x << std::endl;
			target_x = static_cast<float>(player->GetDstRect()->x);
			direction = (target_x - enemy_dest_rect.x > 0) ? 1 : -1;
			state = "move";
		}
	}

	if (state == "wait")
	{
		if (this->WaitDone())
			state = "idle";
	}
	
	if (state == "move")
	{
		Move(player, screen_width, screen_height);

		if (fabs(enemy_dest_rect.x - target_x) <= player_distance_threshold)
		{
			state = "shoot";
			num_proj_shot = 0;
			last_fire_time = SDL_GetTicks();
		}
	}

	if (state == "shoot")
	{
		if (num_proj_shot >= max_proj_shot)
		{
			num_proj_shot = 0;
			state = "wait";
			last_wait_time = SDL_GetTicks();
		}

		else if (BulletReady())
		{
			Attack(game_projectiles, player);
			num_proj_shot++;
		}
	}



	if (state == "death")
	{
		swing_angle = 0.0;
		enemy_coll_shape.circle.r = 0;
		enemy_coll_shape.circle.x = 0;
		enemy_coll_shape.circle.y = 0;
		if (!added_death_animation && current_animation->GetCurrentFrameIndex() == 5)
		{
			if (!shiny)
				overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "ice_burst"), Animation::Order::BACK));
			else
				overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "ice_burst_shiny"), Animation::Order::BACK));
			added_death_animation = true;
		}
		if (current_animation->GetName() != "enemy-ice-crystal-death" and current_animation->GetName() != "enemy-ice-crystal-death_shiny")
		{
			if (shiny)
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-ice-crystal", "death_shiny"));
			else
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-ice-crystal", "death"));
		}

		if (current_animation->IsFinished())
		{
			current_animation->Reset();
			state = "delete";
			
		}

	}
	current_animation->Update();
	for (auto& animation : overlay_animations)
	{
		animation->Update();
	}

	UpdateSwing();
}

void IceCrystal::Move(Player* player, int screen_width, int screen_height)
{

	float diff = target_x - posX;
	float distance = fabsf(diff);
	float direction = (diff > 0 ? 1.0f : -1.0f);
	dead_zone = diff * 0.25;

	

	// Slow-down zone begins this distance away from target
	float slow_range = 100.0f;    // tune this like "braking radius"

	if (distance > dead_zone)
	{
		// accelerate toward target
		velocity += direction * acceleration;
	}
	else
	{
		// ---------------------------------------------------------
		// Scale deceleration by how close we are
		// ---------------------------------------------------------
		float proximity = 1.0f - (distance / slow_range);
		if (proximity < 0.0f) proximity = 0.0f;
		if (proximity > 1.0f) proximity = 1.0f;

		float scaled_deccel = deceleration * proximity;

		// Apply braking gradually
		if (velocity > 0.0f)
		{
			velocity -= scaled_deccel;
			if (velocity < 0.0f) velocity = 0.0f;
		}
		else if (velocity < 0.0f)
		{
			velocity += scaled_deccel;
			if (velocity > 0.0f) velocity = 0.0f;
		}
	}

	// clamp speed
	if (velocity > max_speed) velocity = max_speed;
	if (velocity < -max_speed) velocity = -max_speed;

	// apply movement
	posX += velocity;
	enemy_dest_rect.x = static_cast<int>(posX);


	enemy_dest_rect.x = static_cast<int>(posX);
	enemy_coll_shape.circle.x = enemy_dest_rect.x + (enemy_dest_rect.w / 2);
	enemy_coll_shape.circle.y = enemy_dest_rect.y + (enemy_dest_rect.h / 2);
	enemy_coll_shape.circle.r = enemy_dest_rect.w / 5;
}

bool IceCrystal::IsReadyToAttack()
{
	Uint32 current_time = SDL_GetTicks();

	if ((current_time - last_fire_time) >= fire_cooldown_ms)
	{
		last_fire_time = current_time;
		return(true);
	}

	else
	{

		return(false);
	}
}

bool IceCrystal::BulletReady()
{
	Uint32 current_time = SDL_GetTicks();

	if ((current_time - last_bullet_time) >= bullet_rate)
	{
		last_bullet_time = current_time;
		return(true);
	}

	else
	{
		return(false);
	}
}

bool IceCrystal::WaitDone()
{
	Uint32 current_time = SDL_GetTicks();

	if ((current_time - last_wait_time) >= wait_duration)
	{
		last_wait_time = current_time;
		return(true);
	}

	else
	{
		return(false);
	}
}


void IceCrystal::Attack(std::vector<Projectile*>& game_projectiles, Player* player)
{
	std::string texture_key = shiny ? "spawn_shiny" : "spawn";
	int proj_w = animation_manager.Get("proj-ice-crystal-attack", texture_key)->GetFrameWidth() * animation_manager.Get("proj-ice-crystal-attack", texture_key)->GetScale();
	int proj_h = animation_manager.Get("proj-ice-crystal-attack", texture_key)->GetFrameHeight() * animation_manager.Get("proj-ice-crystal-attack", texture_key)->GetScale();

	// Base center position
	int enemy_center_x = enemy_dest_rect.x + enemy_dest_rect.w / 2;
	std::cout << "[*] VELOCITY " << velocity << std::endl;
	

	// Apply offset based on how many have been shot in the burst
	int final_x = enemy_center_x + (proj_w/2);

	// Vertical placement
	int final_y = enemy_dest_rect.y + (enemy_dest_rect.h * 0.75);

	// Build destination rect
	SDL_Rect ice_shard_dest = { final_x, final_y, proj_w, proj_h };

	game_projectiles.emplace_back(
		new IceShard(animation_manager, ice_shard_dest, 1.3, 3, base_damage, shiny)
	);
}

void IceCrystal::UpdateSwing()
{	
	if (!do_swing)
		return;

	// Determine direction if not already set
	if (!swinging_initialized)
	{
		direction = (target_x - enemy_dest_rect.x >= 0) ? 1 : -1;
		swinging_initialized = true;
	}

	// Apply swing
	if (!going_down)
	{
		// Accelerate toward max angle
		swing_angle += step_angle * direction;

		// Overshoot check
		if (abs(swing_angle) >= max_swing_angle)
		{
			swing_angle = max_swing_angle * direction; // clamp to max
			going_down = true;
		}

		// Smooth acceleration: step increases slightly each frame
		step_angle *= 1.05; // small acceleration factor
		if (step_angle > BASE_STEP_ANGLE * 1.35)
			step_angle = BASE_STEP_ANGLE * 1.35; // clamp max step
	}
	else
	{
		// Decelerate swing back toward zero
		swing_angle -= step_angle * direction;

		// Stop swing when near zero
		if ((direction > 0 && swing_angle <= 0.0) ||
			(direction < 0 && swing_angle >= 0.0))
		{
			swing_angle = 0.0;
			going_down = false;
			do_swing = false;
			swinging_initialized = false;
		}

		// Smooth decay: reduce step each frame
		step_angle *= scale_factor;
		if (step_angle < 0.5) // clamp minimal step to avoid tiny oscillations
			step_angle = 0.5;
	}

	// Debug print
	std::cout << "[*] Swing Angle: " << swing_angle
		<< " Step Angle: " << step_angle
		<< " Direction: " << direction << std::endl;
}

void IceCrystal::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{
	// DRAW BACK OVERLAYS
	for (auto& animation : overlay_animations)
	{
		if (animation->GetOrder() == Animation::Order::BACK)
		{
			//std::cout << "DRAWING OVERLAY !" << std::endl;
			SDL_Rect* current_frame = animation->GetCurrentFrame();
			SDL_Rect temp = { enemy_dest_rect.x + (enemy_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
								enemy_dest_rect.y + (enemy_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
								current_frame->w * animation->GetScale(),
								current_frame->h * animation->GetScale() };
			animation->Draw(renderer, temp, SDL_FLIP_NONE);
		}
	}


	current_animation->DrawRotated(renderer, enemy_dest_rect, SDL_FLIP_NONE, swing_angle);
	
	if (collision_box_flag)
	{
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		Collisions::DrawCircle(renderer, enemy_coll_shape.circle);
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderDrawRect(renderer, &enemy_dest_rect);
	}
	
	// DRAW FRONT OVERLAYS
	for (auto& animation : overlay_animations)
	{
		if (animation->GetOrder() == Animation::Order::FRONT)
		{
			//std::cout << "DRAWING OVERLAY !" << std::endl;
			SDL_Rect* current_frame = animation->GetCurrentFrame();
			SDL_Rect temp = { enemy_dest_rect.x + (enemy_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
								enemy_dest_rect.y + (enemy_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
								current_frame->w * animation->GetScale(),
								current_frame->h * animation->GetScale() };
			animation->Draw(renderer, temp, SDL_FLIP_NONE);
		}
	}
}
// STORM CLOUD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StormCloud::StormCloud(AnimationManager& animation_manager, int screen_width, int screen_height, int player_x, int player_y)
	: Enemy(animation_manager, { -32,-32,48 * 4 ,32 * 4 }, Collider(0, 0, 0), 2.9, 30, 0, 35)
{
	if (shiny)
		current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "main_shiny"));
	else
		current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "main"));
	enemy_dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
	enemy_dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();


	
	points = 10;

	std::cout << "[*] Goal x and y: " << goal_x << " " << goal_y << std::endl;
	
	enemy_coll_shape.type = ColliderType::CIRCLE;
	enemy_coll_shape.circle.x = enemy_dest_rect.x + (enemy_dest_rect.w / 2);
	enemy_coll_shape.circle.y = enemy_dest_rect.y + (enemy_dest_rect.h / 2);
	enemy_coll_shape.circle.r = enemy_dest_rect.w / 6;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> location_zone(0,1000);
	std::uniform_int_distribution<> within_land_position_y(screen_height * 0.25, screen_height * 0.40);
	std::uniform_int_distribution<> within_screen_x(0, screen_width);
	std::uniform_int_distribution<> within_land_position_x(screen_width * 0.25, screen_width * 0.75);


	enemy_dest_rect = { within_screen_x(gen), -32, static_cast<int>(current_animation->GetFrameWidth() * current_animation->GetScale()), static_cast<int>(current_animation->GetFrameHeight() * current_animation->GetScale()) };
	goal_x = within_land_position_x(gen);
	goal_y = within_land_position_y(gen);

	// Calculate the direction vector
	float delta_x = goal_x - enemy_dest_rect.x + (enemy_dest_rect.w / 2);
	float delta_y = goal_y - enemy_dest_rect.y + (enemy_dest_rect.h/2);
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

	position_x = enemy_dest_rect.x;
	position_y = enemy_dest_rect.y;

	std::cout << "[*] Direction x: " << direction_x << "Direction y: " << direction_y << std::endl;
	
}

void StormCloud::Update(Player* player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, int screen_width, int screen_height)
{
	float threshhold = 85;
	//std::cout << "STATE: " << state <<  "Shot Fired: " << shot_fired << "=============================================" << std::endl;

	if (state == "main")
	{
		if (enemy_dest_rect.x > 0 and enemy_dest_rect.x < 1920 and enemy_dest_rect.y > 0 and enemy_dest_rect.y < 1080 and shiny and !shiny_sound_played)
		{
			sound_manager.PlaySound("shiny", 75);
			shiny_sound_played = true;
		}
		
		Move(player, screen_width, screen_height);
		if ((std::abs(enemy_dest_rect.x - goal_x) < threshhold) && (std::abs(enemy_dest_rect.y - goal_y) < threshhold))
		{
			state = "wait";
			start_of_wait_state = SDL_GetTicks();
		}
	}

	if (state == "wait")
	{
		Uint32 elapsed = SDL_GetTicks() - start_of_wait_state;
		if (elapsed > time_to_wait_ms)
		{
			if (!shot_fired)
			{
				state = "shoot";
				shot_fired = false;
				if (shiny)
					current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "attack_shiny"));
				else
					current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "attack"));
			}
			else
			{
				state = "retreat";
			}
		}
	}

	if (state == "shoot")
	{
		if (!shot_fired)
		{
			if (current_animation->GetName() != "enemy-storm-cloud-attack" and current_animation->GetName() != "enemy-storm-cloud-attack_shiny")
			{
				if (shiny)
					current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "attack_shiny"));
				else
					current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "attack"));
			}
			Attack(game_projectiles, player);
			
		}

		if (current_animation->IsFinished())
		{
			shot_fired = true;
		}

		if (shot_fired)
		{
			direction_x *= -1;
			direction_y *= -1;
			start_of_wait_state = SDL_GetTicks();
			state = "wait"; 
		}
	}

	if (state == "retreat")
	{
		Move(player, screen_width, screen_height);
	}

	if (state == "death")
	{
		enemy_coll_shape.circle.r = 0;
		enemy_coll_shape.circle.x = 0;
		enemy_coll_shape.circle.y = 0;

		if (current_animation->GetName() != "enemy-storm-cloud-death" and current_animation->GetName() != "enemy-storm-cloud-death_shiny")
		{
			
			if (shiny)
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "death_shiny"));
			else
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "death"));			
		}

		if (!added_death_animation)
		{
			overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "lightning_burst")));
			added_death_animation = true;
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



void StormCloud::Move(Player* player, int screen_width, int screen_height)
{
	position_x += direction_x * movement_speed;
	position_y += direction_y * movement_speed;

	enemy_dest_rect.x = static_cast<int>(position_x) + (enemy_dest_rect.w / 2) - (enemy_dest_rect.w / 2);
	enemy_dest_rect.y = static_cast<int>(position_y) + (enemy_dest_rect.h / 2) - (enemy_dest_rect.h / 2);
	enemy_coll_shape.circle.x = static_cast<int>(position_x) + (enemy_dest_rect.w / 2);
	enemy_coll_shape.circle.y = static_cast<int>(position_y) + (enemy_dest_rect.h / 2);
}

bool StormCloud::IsReadyToAttack()
{
	if (state == "shoot" && !shot_fired && current_animation->GetCurrentFrameIndex() == 4)
	{
		shot_fired = true;
		return true;
	}
	else
		return false;
}

void StormCloud::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{
	

	// DRAW BACK OVERLAYS
	for (auto& animation : overlay_animations)
	{
		if (animation->GetOrder() == Animation::Order::BACK)
		{
			//std::cout << "DRAWING OVERLAY !" << std::endl;
			SDL_Rect* current_frame = animation->GetCurrentFrame();
			SDL_Rect temp = { enemy_dest_rect.x + (enemy_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
								enemy_dest_rect.y + (enemy_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
								current_frame->w * animation->GetScale(),
								current_frame->h * animation->GetScale() };
			animation->Draw(renderer, temp, SDL_FLIP_NONE);
		}
	}
	current_animation->Draw(renderer, enemy_dest_rect, SDL_FLIP_NONE);

	// DRAW FRONT OVERLAYS
	for (auto& animation : overlay_animations)
	{
		if (animation->GetOrder() == Animation::Order::FRONT)
		{
			//std::cout << "DRAWING OVERLAY !" << std::endl;
			SDL_Rect* current_frame = animation->GetCurrentFrame();
			SDL_Rect temp = { enemy_dest_rect.x + (enemy_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
								enemy_dest_rect.y + (enemy_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
								current_frame->w * animation->GetScale(),
								current_frame->h * animation->GetScale() };
			animation->Draw(renderer, temp, SDL_FLIP_NONE);
		}
	}
	
	SDL_Rect rect;
	int size = 5;
	rect.x = this->GetGoalX() - size / 2;
	rect.y = this->GetGoalY() - size / 2;
	rect.w = size;
	rect.h = size;
		
	if (collision_box_flag)
	{
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		if (0 != SDL_RenderFillRect(renderer, &rect))
		{
			std::cout << "[*] Error rendering point storm cloud goal point ...\n";
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		Collisions::DrawCircle(renderer, enemy_coll_shape.circle);

	}

	
}

void StormCloud::Attack(std::vector<Projectile*>& game_projectiles, Player* player)
{
	if (this->IsReadyToAttack())
	{
		game_projectiles.emplace_back(new LightningBall(animation_manager, 
														enemy_dest_rect, 
														3.0, 
														3, 
														base_damage, 
														(player->GetDstRect()->x + (player->GetDstRect()->w / 2)),
														(player->GetDstRect()->y + (player->GetDstRect()->h / 2)), shiny));
	}
}

int StormCloud::GetGoalX()
{
	return goal_x;
}

int StormCloud::GetGoalY()
{
	return goal_y;
}

// STORM GENIE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StormGenie::StormGenie(AnimationManager& animation_manager, const SDL_Rect& dest_rect)
	: Enemy(animation_manager, dest_rect, Collider(0,0,0), 2, 100, 0, 10)
{
	if (shiny)
		current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "spawn_shiny"));
	else
		current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "spawn"));

	enemy_dest_rect.w = current_animation->GetFrameWidth() * current_animation->GetScale();
	enemy_dest_rect.h = current_animation->GetFrameHeight() * current_animation->GetScale();


	shot_fired = false;
	spawned_lightning = false;
	posY = static_cast<float>(enemy_dest_rect.y);
	points = 10;
	fire_cooldown_ms = 4000;
	state = "spawn";
	enemy_coll_shape.type = ColliderType::CIRCLE;
	enemy_coll_shape.circle.x = enemy_dest_rect.x + (enemy_dest_rect.w / 2);
	enemy_coll_shape.circle.y = enemy_dest_rect.y + (enemy_dest_rect.h / 2);
	enemy_coll_shape.circle.r = 5;
}

void StormGenie::Update(Player* player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager, int screen_width, int screen_height)
{
	if (state == "spawn")
	{
		enemy_coll_shape.circle.r = 0;
		if (current_animation->IsFinished())
		{
			if (shiny)
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "main_shiny"));
			else
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "main"));
			state = "main";
		}
		if (shiny and current_animation->GetCurrentFrameIndex() == 4 and !shiny_sound_played)
		{
			sound_manager.PlaySound("shiny", 100);
			shiny_sound_played = true;
		}
	}

	if (state == "main")
	{
		enemy_coll_shape.circle.r = enemy_dest_rect.w / 4;
		if (enemy_dest_rect.y < 5 || enemy_dest_rect.y  > 1020) // SCREEN HEIGHT
		{
			movement_speed *= -1;
		}

		Move(player, screen_width, screen_height);
		
		// READY TO ATTACK
		Uint32 current_time = SDL_GetTicks();
		int vertical_difference = enemy_dest_rect.y - player->GetDstRect()->y;

		if ((current_time - last_fire_time) >= fire_cooldown_ms && abs(vertical_difference) <  35)
		{
			if (!shot_fired)
			{
				if (shiny)
				{
					animation_manager.Get("enemy-storm-genie", "attack_shiny")->Reset();
					current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "attack_shiny"));
				}
				else
				{
					animation_manager.Get("enemy-storm-genie", "attack")->Reset();
					current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "attack"));
				}
				last_fire_time = current_time;
				state = "attacking";
			}
		}
		// END READY TO ATTACK

	}

	if (state == "attacking")
	{

		if (!spawned_lightning && current_animation->GetCurrentFrameIndex() == 4)
		{
			Attack(game_projectiles, player);
			spawned_lightning = true;
			last_fire_time = SDL_GetTicks();
			lightning_strike_channel = sound_manager.PlaySoundTracking("lightning_strike", 45);
		}

		if (shiny)
		{
			if (current_animation->GetName() == "enemy-storm-genie-attack_shiny" && current_animation->IsFinished())
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "attacking_shiny"));
		}
		else
		{
			if (current_animation->GetName() == "enemy-storm-genie-attack" && current_animation->IsFinished())
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "attacking"));
		}
		
		if (IsDoneAttacking())
		{
			shot_fired = false;
			spawned_lightning = false;

		
			left_lightning_bolt->UpdateState("delete");
			right_lightning_bolt->UpdateState("delete");

			sound_manager.StopSoundChannel(lightning_strike_channel);
			

			if (!spawned_lightning)
			{
				if (shiny)
					current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "main_shiny"));
				else
					current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "main"));
				last_fire_time = SDL_GetTicks();
				state = "main";
			}
			// MAKE GENIE GO UP for some Time, then go back towards the player, like the Mario3 angry sun
		}
	}

	if (state == "death")
	{
		enemy_coll_shape.circle.r = 0;
		enemy_coll_shape.circle.x = 0;
		enemy_coll_shape.circle.y = 0;
		if (left_lightning_bolt != nullptr)
		{
			left_lightning_bolt->UpdateState("delete");
		}
		if (right_lightning_bolt!= nullptr)
		{
			right_lightning_bolt->UpdateState("delete");
		}

		sound_manager.StopSoundChannel(lightning_strike_channel); // NEED TO CHECK IF NOT EMPTY ?
		
		// Only set the death animation once
		if (current_animation->GetName() != "enemy-storm-genie-death" and current_animation->GetName() != "enemy-storm-genie-death_shiny")
		{
			if (shiny)
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "death_shiny"));
			else
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "death"));
		}

		// Add heal overlay on first death frame
		if (!death_animation_played)
		{
			death_animation_played = true;
			overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "lightning_burst")));
		}

		// If death animation finished, mark for deletion
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

void StormGenie::Move(Player* player, int screen_width, int screen_height)
{
	float targetY = static_cast<float>(player->GetDstRect()->y);
	float diff = targetY - posY;


	const float dead_zone = 40.0f;

	
	
	float norm_diff = diff / screen_height;
	float hover_offset;

	if (fabs(diff) > dead_zone)
	{
		velocity = velocity * damping + norm_diff * stiffness;
	}
	else
	{
		velocity *= 0.0f;
		hover_offset = 0.0f;
	}

	posY += velocity;//+ hover_offset;

	enemy_dest_rect.y = static_cast<int>(posY);

	enemy_coll_shape.circle.x = enemy_dest_rect.x + (enemy_dest_rect.w / 2);
	enemy_coll_shape.circle.y = enemy_dest_rect.y + (enemy_dest_rect.h / 2);
}

bool StormGenie::IsReadyToAttack()
{

	if (state == "attacking" && current_animation->GetCurrentFrameIndex() >= 4)
		return true;
	else
		return false;
}

void StormGenie::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{
	// DRAW BACK OVERLAYS
	for (auto& animation : overlay_animations)
	{
		if (animation->GetOrder() == Animation::Order::BACK)
		{
			//std::cout << "DRAWING OVERLAY !" << std::endl;
			SDL_Rect* current_frame = animation->GetCurrentFrame();
			SDL_Rect temp = { enemy_dest_rect.x + (enemy_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
								enemy_dest_rect.y + (enemy_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
								current_frame->w * animation->GetScale(),
								current_frame->h * animation->GetScale() };
			animation->Draw(renderer, temp, SDL_FLIP_NONE);
		}
	}
	
	current_animation->Draw(renderer, enemy_dest_rect, SDL_FLIP_NONE);

	// DRAW FRONT OVERLAYS
	for (auto& animation : overlay_animations)
	{
		if (animation->GetOrder() == Animation::Order::FRONT)
		{
			//std::cout << "DRAWING OVERLAY !" << std::endl;
			SDL_Rect* current_frame = animation->GetCurrentFrame();
			SDL_Rect temp = { enemy_dest_rect.x + (enemy_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
								enemy_dest_rect.y + (enemy_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
								current_frame->w * animation->GetScale(),
								current_frame->h * animation->GetScale() };
			animation->Draw(renderer, temp, SDL_FLIP_NONE);
		}
	}
	for (auto& animation : overlay_animations)
	{
		std::cout << "DRAWING OVERLAY !" << std::endl;
		SDL_Rect* current_frame = animation->GetCurrentFrame();
		SDL_Rect temp = { enemy_dest_rect.x + (enemy_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
							enemy_dest_rect.y + (enemy_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
							current_frame->w * animation->GetScale(),
							current_frame->h * animation->GetScale() };
		animation->Draw(renderer, temp, SDL_FLIP_NONE);
	}

	if (collision_box_flag)
	{
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		Collisions::DrawCircle(renderer, enemy_coll_shape.circle);
	}
}

void StormGenie::Attack(std::vector<Projectile*>& game_projectiles, Player* player)
{
	if (this->IsReadyToAttack())
	{
		int bolt_height_diff = -10;
		int width_adjustment = 5;
		float width_scaling = 1;

		std::string left_attack_key;
		std::string right_attack_key;
		if (shiny)
		{
			left_attack_key = "storm-genie-attack-left_shiny";
			right_attack_key = "storm-genie-attack-right_shiny";
		}
		else
		{
			left_attack_key = "storm-genie-attack-left";
			right_attack_key = "storm-genie-attack-right";
		}

		
		const SDL_Rect left_bolt_dst = { (enemy_dest_rect.x - (animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-left")->GetFrameWidth() * width_scaling) + width_adjustment),
											enemy_dest_rect.y + bolt_height_diff,
											animation_manager.Get("proj-storm-genie-attack", left_attack_key)->GetFrameWidth() * width_scaling ,
											animation_manager.Get("proj-storm-genie-attack", right_attack_key)->GetFrameHeight() };
		
		const SDL_Rect right_bolt_dst = {	(enemy_dest_rect.x + (enemy_dest_rect.w) - width_adjustment),
											enemy_dest_rect.y + bolt_height_diff,
											animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-right")->GetFrameWidth() * width_scaling ,
											animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-right")->GetFrameHeight() };
		
		animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-left")->GetFrameWidth();
		

		
		right_lightning_bolt = new LightningStrike(animation_manager, right_bolt_dst, 3, base_damage, true, shiny);
		left_lightning_bolt = new LightningStrike(animation_manager, left_bolt_dst, 3, base_damage, false, shiny);
 
		game_projectiles.emplace_back(right_lightning_bolt);
		game_projectiles.emplace_back(left_lightning_bolt);
	}
}

bool StormGenie::IsDoneAttacking()
{
	Uint32 current_time = SDL_GetTicks();

	if ((current_time - last_fire_time) >= lightning_strike_duration)
	{
		return(true);
	}

	else
	{
		return(false);
	}
}


