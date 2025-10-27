
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
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> shiny_chance(0, 999);
	std::cout << "[*] Updating Enemies since size is 0\n";
	shiny = shiny_chance(gen) < 6;
	
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

	enemy_coll_shape.type = ColliderType::CIRCLE;
	enemy_coll_shape.circle.x = enemy_dest_rect.x + (enemy_dest_rect.w / 2);
	enemy_coll_shape.circle.y = enemy_dest_rect.y + (enemy_dest_rect.h / 2);
	enemy_coll_shape.circle.r = 5;
	fire_cooldown_ms = 300;
	points = 10;
	
}

void IceCrystal::Update(Player *player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager)
{
	int player_distance_threshold = 40;
	//std::cout << "ICE STATE: " << "---------------------------------------------------" << state << std::endl;
	
	if (state == "idle")
	{
		if (shiny and !shiny_sound_played)
		{
			sound_manager.PlaySound("shiny", 50);
			shiny_sound_played = true;
		}
		
		int lateral_diff = abs(enemy_dest_rect.x - player->GetDstRect()->x);

		if (lateral_diff <= player_distance_threshold)
		{
			state = "shoot";
			num_proj_shot = 0;
			last_fire_time = SDL_GetTicks();
		}
		else
		{
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
		Move(player);

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
		enemy_coll_shape.circle.r = 0;
		enemy_coll_shape.circle.x = 0;
		enemy_coll_shape.circle.y = 0;
		if (!added_death_animation && current_animation->GetCurrentFrameIndex() == 5)
		{
			overlay_animations.push_back(std::make_unique<Animation>(*animation_manager.Get("overlays", "ice_burst")));
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
	
	
}

void IceCrystal::Move(Player* player)
{
	
	float diff = target_x - posX;
	float screen_height = static_cast<float>(1080);
	float norm_diff = diff / screen_height;
	float hover_offset;

	if (fabs(diff) > dead_zone)
		velocity = velocity * damping + norm_diff * stiffness;

	else
	{
		velocity *= 0.0f;
		hover_offset = 0.0f;
	}

	posX += velocity;

	enemy_dest_rect.x = static_cast<int>(posX);

	int coll_box_width = enemy_dest_rect.w / 3;
	int coll_box_hight = enemy_dest_rect.h / 3;
	
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
	std::string texture_key;
	if (shiny)
		texture_key = "spawn_shiny";
	else
		texture_key = "spawn";
	const SDL_Rect ice_shard_dest = { (enemy_dest_rect.x + (enemy_dest_rect.w / 2) - (animation_manager.Get("proj-ice-crystal-attack", texture_key)->GetFrameWidth()) / 2),
										enemy_dest_rect.y + (enemy_dest_rect.h * 0.8),
											animation_manager.Get("proj-ice-crystal-attack", texture_key)->GetFrameWidth() + (num_proj_shot * 30) ,
											animation_manager.Get("proj-ice-crystal-attack", texture_key)->GetFrameHeight() };
	
	game_projectiles.emplace_back(new IceShard(animation_manager, ice_shard_dest, 5.0, 3, base_damage, shiny));
}

void IceCrystal::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{

	current_animation->Draw(renderer, enemy_dest_rect, SDL_FLIP_NONE);
	if (collision_box_flag)
	{
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		Collisions::DrawCircle(renderer, enemy_coll_shape.circle);
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
}
// STORM CLOUD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StormCloud::StormCloud(AnimationManager& animation_manager, int screen_width, int screen_height, int player_x, int player_y)
	: Enemy(animation_manager, { -32,-32,48 * 4 ,32 * 4 }, Collider(0, 0, 0), 4.7, 30, 0, 35)
{
	if (shiny)
		current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "main_shiny"));
	else
		current_animation = std::make_unique<Animation>(*animation_manager.Get("enemy-storm-cloud", "main"));
	
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

void StormCloud::Update(Player* player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager)
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
		Move(player);
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



void StormCloud::Move(Player* player)
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
														7.5, 
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

void StormGenie::Update(Player* player, std::vector<Projectile*>& game_projectiles, SoundManager& sound_manager)
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

		Move(player);
		
		// READY TO ATTACK
		Uint32 current_time = SDL_GetTicks();
		int vertical_difference = enemy_dest_rect.y - player->GetDstRect()->y;

		if ((current_time - last_fire_time) >= fire_cooldown_ms && abs(vertical_difference) <  100)
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

void StormGenie::Move(Player* player)
{
	float targetY = static_cast<float>(player->GetDstRect()->y);
	float diff = targetY - posY;


	const float dead_zone = 40.0f;

	
	float screen_height = static_cast<float>(1080);
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
		Collisions::DrawCircle(renderer, enemy_coll_shape.circle);
	}
}

void StormGenie::Attack(std::vector<Projectile*>& game_projectiles, Player* player)
{
	if (this->IsReadyToAttack())
	{
		int bolt_height_diff = -30;
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


