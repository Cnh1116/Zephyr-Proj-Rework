
#include <SDL.h>
#include <random>
#include <iostream>
#include "Enemy.hpp"
#include "Player.hpp"
#include "AnimationManager.hpp"
#include <Projectiles.hpp>

Enemy::Enemy(AnimationManager& animation_manager, const SDL_Rect& dest_rect, const SDL_Rect& coll_rect, float move_speed, int health_arg, float crit, float start_damage)
	: animation_manager(animation_manager)
{
	enemy_dest_rect = dest_rect;
	enemy_coll_rect = coll_rect;

	movement_speed = move_speed;
	base_health = health_arg;
	base_damage = start_damage;

	points = 0;

	invincible = false;
	state = "main";
}

// Setters and Getters
SDL_Rect* Enemy::GetCollRect()
{
	return &enemy_coll_rect;
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
	: Enemy(animation_manager, dest_rect, dest_rect, 1.0, 100, 0, 35)
{
	
	
	state = "idle";
	current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-ice-crystal", "main"));
	enemy_coll_rect = { enemy_dest_rect.x + (enemy_dest_rect.w / 2) - (enemy_dest_rect.w / 4),
						enemy_dest_rect.y + (enemy_dest_rect.h / 2) - (enemy_dest_rect.h / 4),
						enemy_dest_rect.w / 2,
						enemy_dest_rect.h / 2 };
	fire_cooldown_ms = 300;
	
}

void IceCrystal::Update(Player *player, std::vector<Projectile*>& game_projectiles)
{
	int player_distance_threshold = 40;
	//std::cout << "ICE STATE: " << "---------------------------------------------------" << state << std::endl;
	
	// --- IDLE / SPAWN ---
	if (state == "idle")
	{
		int lateral_diff = abs(enemy_dest_rect.x - player->GetDstRect()->x);

		if (lateral_diff <= player_distance_threshold)
		{
			// Player is close → start shooting immediately
			state = "shoot";
			num_proj_shot = 0;
			last_fire_time = SDL_GetTicks();
		}
		else
		{
			// Player far → start moving toward their X (snapshot)
			target_x = static_cast<float>(player->GetDstRect()->x);
			state = "move";
		}
	}

	if (state == "wait" && this->WaitDone())
	{
		state = "idle";
	}
	
	if (state == "move")
	{
		Move(player); // Move should move toward target_x (not player->x directly!)

		// Check if we’ve reached our stored target position
		if (fabs(enemy_dest_rect.x - target_x) <= player_distance_threshold)
		{
			state = "shoot";
			num_proj_shot = 0;
			last_fire_time = SDL_GetTicks();
		}
	}

	if (state == "shoot")
	{
		
		// Once done shooting, go idle again
		if (num_proj_shot >= max_proj_shot)
		{
			num_proj_shot = 0;
			state = "wait";
			last_wait_time = SDL_GetTicks();
		}
		// Stay stationary and fire at intervals
		else if (BulletReady())
		{
			Attack(game_projectiles, player);
			num_proj_shot++;
		}
	}



	if (state == "death")
	{
		if (!added_death_animation)
		{
			overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "heal")));
			added_death_animation = true;
		}
		if (current_animation->GetName() != "enemy-ice-crystal-death")
		{
			current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-ice-crystal", "death"));
		}

		enemy_coll_rect = { 0,0,0,0 };
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
	
	
}

void IceCrystal::Move(Player* player)
{
	
	float diff = target_x - posX;

	// --- Dead zone ---
	const float dead_zone = 40.0f;  // pixels of tolerance before moving

	// --- Screen-normalized difference ---
	float screen_height = static_cast<float>(1080);
	float norm_diff = diff / screen_height;

	
	float hover_offset;

	// --- Springy motion ---
	if (fabs(diff) > dead_zone)
	{
		// Only chase player if far enough away
		velocity = velocity * damping + norm_diff * stiffness;

		// --- Optional hover motion ---
		//hover_offset = sin(SDL_GetTicks() * hover_freq) * hover_amp;
	}
	else
	{
		// Within tolerance → slow down gradually
		velocity *= 0.0f;  // optional: damp to zero smoothly
		hover_offset = 0.0f;
	}



	// --- Apply velocity ---
	posX += velocity;//+ hover_offset;

	// --- Update render position ---
	enemy_dest_rect.x = static_cast<int>(posX);

	int coll_box_width = enemy_dest_rect.w / 3;
	int coll_box_hight = enemy_dest_rect.h / 3;
	
	enemy_coll_rect.x = enemy_dest_rect.x + (enemy_dest_rect.w / 2) - (coll_box_width / 2);
	enemy_coll_rect.y = enemy_dest_rect.y + (enemy_dest_rect.h / 2) - (coll_box_hight / 2);
	enemy_coll_rect.w = coll_box_width;
	enemy_coll_rect.h = coll_box_hight;
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
	const SDL_Rect ice_shard_dest = { (enemy_dest_rect.x + (enemy_dest_rect.w / 2) - (animation_manager.Get("proj-ice-crystal-attack", "spawn")->GetFrameWidth()) / 2),
										enemy_dest_rect.y + (enemy_dest_rect.h * 0.8),
											animation_manager.Get("proj-ice-crystal-attack", "spawn")->GetFrameWidth() + (num_proj_shot * 30) ,
											animation_manager.Get("proj-ice-crystal-attack", "spawn")->GetFrameHeight() };
	
	game_projectiles.emplace_back(new IceShard(animation_manager, ice_shard_dest, 5.0, 3, base_damage));
}

void IceCrystal::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{
	current_animation->Draw(renderer, enemy_dest_rect, SDL_FLIP_NONE);

	for (auto& animation : overlay_animations)
	{
		std::cout << "DRAWING OVERLAY !" << std::endl;
		SDL_Rect* current_frame = animation->GetCurrentFrame();
		SDL_Rect temp = {	enemy_dest_rect.x + (enemy_dest_rect.w - current_frame->w * animation->GetScale()) / 2,
							enemy_dest_rect.y + (enemy_dest_rect.h - current_frame->h * animation->GetScale()) / 2,
							current_frame->w * animation->GetScale(),
							current_frame->h * animation->GetScale() };
		animation->Draw(renderer, temp, SDL_FLIP_NONE);
	}

	if (collision_box_flag)
	{
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		if (0 != SDL_RenderDrawRect(renderer, &enemy_coll_rect))
		{
			std::cout << "[*] Error rendering ice crystal collision box ...\n";
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	}

}

// STORM CLOUD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StormCloud::StormCloud(AnimationManager& animation_manager, int screen_width, int screen_height, int player_x, int player_y)
	: Enemy(animation_manager, { -32,-32,48 * 4 ,32 * 4 }, { -32,-32,48 * 4,32 * 4 }, 4.7, 30, 0, 35)
{

	current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "main"));
	
	std::cout << "[*] Goal x and y: " << goal_x << " " << goal_y << std::endl;
	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> location_zone(0,1000);
	std::uniform_int_distribution<> within_land_position_y(screen_height * 0.25, screen_height * 0.40);
	std::uniform_int_distribution<> within_screen_x(0, screen_width);
	std::uniform_int_distribution<> within_land_position_x(screen_width * 0.25, screen_width * 0.75);


	enemy_dest_rect = { within_screen_x(gen), -32, 48 * 3, 32 * 3 };
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

void StormCloud::Update(Player* player, std::vector<Projectile*>& game_projectiles)
{
	float threshhold = 85;
	//std::cout << "STATE: " << state <<  "Shot Fired: " << shot_fired << "=============================================" << std::endl;

	if (state == "main")
	{
		Move(player);
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
			// Transition to shoot only if not already shot
			if (!shot_fired)
			{
				state = "shoot";
				shot_fired = false;           // reset flag for shooting
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "attack"));
			}
			else
			{
				state = "retreat"; // move away after shooting
			}
		}
	}

	if (state == "shoot")
	{
		// Only set attack animation once
		if (!shot_fired)
		{
			if (current_animation->GetName() != "enemy-storm-cloud-attack")
			{
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "attack"));
			}
			Attack(game_projectiles, player);
			
		}

		// Example: check if animation finished to fire
		if (current_animation->IsFinished())
		{
			shot_fired = true;
		}

		if (shot_fired)
		{
			direction_x *= -1;
			direction_y *= -1;
			start_of_wait_state = SDL_GetTicks();
			state = "wait"; // leave screen / retreat
		}
	}

	if (state == "retreat")
	{
		Move(player);
	}

	if (state == "death")
	{
		// Only set death animation once
		if (current_animation->GetName() != "enemy-storm-cloud-death")
		{
			current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "death"));
			enemy_coll_rect = { 0, 0, 0, 0 };
		}

		// Add overlay once
		if (!added_death_animation)
		{
			overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "heal")));
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



void StormCloud::Move(Player* player)
{
	

	
	position_x += direction_x * movement_speed;
	position_y += direction_y * movement_speed;

	enemy_dest_rect.x = static_cast<int>(position_x) + (enemy_dest_rect.w / 2) - (enemy_dest_rect.w / 2);
	enemy_dest_rect.y = static_cast<int>(position_y) + (enemy_dest_rect.h / 2) - (enemy_dest_rect.h / 2);
	enemy_coll_rect.x = static_cast<int>(position_x) + (enemy_dest_rect.w / 2) - (enemy_coll_rect.w / 2);
	enemy_coll_rect.y = static_cast<int>(position_y) + (enemy_dest_rect.h / 2) - (enemy_coll_rect.h / 2);
	enemy_coll_rect.w = enemy_dest_rect.w / 2;
	enemy_coll_rect.h = enemy_dest_rect.h / 2;
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
	current_animation->Draw(renderer, enemy_dest_rect, SDL_FLIP_NONE);

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
	
		SDL_Rect rect;
		int size = 5;
		rect.x = this->GetGoalX() - size / 2; // Center the point
		rect.y = this->GetGoalY() - size / 2; // Center the point
		rect.w = size;         // Width of the rectangle
		rect.h = size;
		// Now it's safe to call the special function
		if (collision_box_flag)
		{
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			if (0 != SDL_RenderFillRect(renderer, &rect))
			{
				std::cout << "[*] Error rendering point storm cloud goal point ...\n";
			}
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			SDL_RenderDrawRect(renderer, &enemy_coll_rect);

		}
	
}

void StormCloud::Attack(std::vector<Projectile*>& game_projectiles, Player* player)
{
	if (this->IsReadyToAttack())
	{
		game_projectiles.emplace_back(new LightningBall(animation_manager, enemy_dest_rect, 7.5, 3, base_damage, (player->GetCollRect()->x + (player->GetCollRect()->w / 2)), (player->GetCollRect()->y + (player->GetCollRect()->h / 2))));
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
	: Enemy(animation_manager, dest_rect, dest_rect, 2, 100, 0, 10)
{
	
	current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "spawn"));
	shot_fired = false;
	spawned_lightning = false;
	posY = static_cast<float>(enemy_dest_rect.y);
	
	fire_cooldown_ms = 4000;
	state = "spawn";
}

void StormGenie::Update(Player* player, std::vector<Projectile*>& game_projectiles)
{
	if (state == "spawn")
	{
		enemy_coll_rect = { 0, 0, 0, 0 };
		if (current_animation->IsFinished())
		{
			current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "main"));
			float width_Scale = 0.4;
			state = "main";
		}
	}

	if (state == "main")
	{
		if (enemy_dest_rect.y < 5 || enemy_dest_rect.y  > 1020) // SCREEN HEIGHT
		{
			movement_speed *= -1;
		}

		Move(player);
		
		// READY TO ATTACK
		Uint32 current_time = SDL_GetTicks();
		int vertical_difference = enemy_dest_rect.y - player->GetDstRect()->y;

		if ((current_time - last_fire_time) >= fire_cooldown_ms && abs(vertical_difference) <  100)
		{
			if (!shot_fired)
			{
				animation_manager.Get("enemy-storm-genie", "attack")->Reset();
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "attack"));
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
		}
		
		if (IsDoneAttacking())
		{
			shot_fired = false;
			spawned_lightning = false;

		
			left_lightning_bolt->UpdateState("delete");
			right_lightning_bolt->UpdateState("delete");
			

			if (!spawned_lightning)
			{
				current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "main"));
				last_fire_time = SDL_GetTicks();
				state = "main";
			}
			// MAKE GENIE GO UP for some Time, then go back towards the player, like the Mario3 angry sun
		}
	}

	if (state == "death")
	{
		if (left_lightning_bolt != nullptr)
		{
			left_lightning_bolt->UpdateState("delete");
		}
		if (right_lightning_bolt!= nullptr)
		{
			right_lightning_bolt->UpdateState("delete");
		}
		
		// Only set the death animation once
		if (current_animation->GetName() != "enemy-storm-genie-death")
		{
			current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-genie", "death"));
			enemy_coll_rect = { 0,0,0,0 };  // disable collision
		}

		// Add heal overlay on first death frame
		if (!death_animation_played)
		{
			death_animation_played = true;
			overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "heal")));
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

void StormGenie::Move(Player* player)
{
	float targetY = static_cast<float>(player->GetDstRect()->y);
	float diff = targetY - posY;

	// --- Dead zone ---
	const float dead_zone = 40.0f;  // pixels of tolerance before moving

	// --- Screen-normalized difference ---
	float screen_height = static_cast<float>(1080);
	float norm_diff = diff / screen_height;

	// --- Tunable parameters ---
	const float stiffness = 2.0f;     // increase after normalization
	const float damping = 0.7f;
	const float hover_amp = 1.5f;
	const float hover_freq = 0.07f;
	float hover_offset;

	// --- Springy motion ---
	if (fabs(diff) > dead_zone)
	{
		// Only chase player if far enough away
		velocity = velocity * damping + norm_diff * stiffness;
		
		// --- Optional hover motion ---
		//hover_offset = sin(SDL_GetTicks() * hover_freq) * hover_amp;
	}
	else
	{
		// Within tolerance → slow down gradually
		velocity *= 0.0f;  // optional: damp to zero smoothly
		hover_offset = 0.0f;
	}

	

	// --- Apply velocity ---
	posY += velocity;//+ hover_offset;

	// --- Update render position ---
	enemy_dest_rect.y = static_cast<int>(posY);



	

	enemy_coll_rect.x = enemy_dest_rect.x + (enemy_dest_rect.w / 2) - (enemy_coll_rect.w / 2);
	enemy_coll_rect.y = enemy_dest_rect.y + (enemy_dest_rect.h / 2) - (enemy_coll_rect.h / 2);
	enemy_coll_rect.w = enemy_dest_rect.w / 2;
	enemy_coll_rect.h = enemy_dest_rect.h / 2;
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
	current_animation->Draw(renderer, enemy_dest_rect, SDL_FLIP_NONE);
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
		if (0 != SDL_RenderDrawRect(renderer, &enemy_coll_rect))
		{
			std::cout << "[*] Error rendering storm genie collision box ...\n";
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	}
}

void StormGenie::Attack(std::vector<Projectile*>& game_projectiles, Player* player)
{
	if (this->IsReadyToAttack())
	{
		int bolt_height_diff = -30;
		int width_adjustment = 5;
		float width_scaling = 1;
		
		const SDL_Rect left_bolt_dst = { (enemy_dest_rect.x - (animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-left")->GetFrameWidth() * width_scaling) + width_adjustment),
											enemy_dest_rect.y + bolt_height_diff,
											animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-left")->GetFrameWidth() * width_scaling ,
											animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-left")->GetFrameHeight() };
		
		const SDL_Rect right_bolt_dst = {	(enemy_dest_rect.x + (enemy_dest_rect.w) - width_adjustment),
											enemy_dest_rect.y + bolt_height_diff,
											animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-right")->GetFrameWidth() * width_scaling ,
											animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-right")->GetFrameHeight() };
		
		animation_manager.Get("proj-storm-genie-attack", "storm-genie-attack-left")->GetFrameWidth();
		

		
		right_lightning_bolt = new LightningStrike(animation_manager, right_bolt_dst, 3, base_damage, true);
		left_lightning_bolt = new LightningStrike(animation_manager, left_bolt_dst, 3, base_damage, false);
 
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


