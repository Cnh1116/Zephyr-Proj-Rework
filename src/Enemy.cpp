
#include <SDL.h>
#include <random>
#include <iostream>
#include "Enemy.hpp"
#include "Player.hpp"
#include "AnimationManager.hpp"

Enemy::Enemy(const SDL_Rect& dest_rect, const SDL_Rect& coll_rect, float move_speed, int health_arg, float crit, float start_damage)
{
	enemy_dest_rect = dest_rect;
	enemy_coll_rect = coll_rect;

	movement_speed = move_speed;
	base_health = health_arg;
	base_damage = start_damage;

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
IceCrystal::IceCrystal(AnimationManager* animation_manager, const SDL_Rect& dest_rect)
	: Enemy(dest_rect, dest_rect, 1.0, 100, 0, 35)
{
	animations["main"] = *animation_manager->Get("enemy-ice-crystal", "main");
	animations["death"] = *animation_manager->Get("enemy-ice-crystal", "death");
	animations["heal"] = *animation_manager->Get("overlays", "heal");
	animations["death"].Reset();
	state = "main";
	current_animation = &animations["main"];
	fire_cooldown_ms = 300;
}

void IceCrystal::Update(Player *player)
{

	if (state == "main")
	{
		
		if (enemy_dest_rect.x == 0 || enemy_dest_rect.x + enemy_dest_rect.w == 2000) //Screen width
		{
			movement_speed *= -1;
		}

		current_animation = &animations["main"];
		Move(player);
	}



	if (state == "death")
	{
		if (current_animation->GetCurrentFrameIndex() == 1)
		{
			std::cout << "[*] Ice crystal dying, adding heal overlay~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
			overlay_animations.push_back(std::make_unique<Animation>(animations["heal"]));
		}
		current_animation = &animations["death"];
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
	
	if ((player->GetDstRect()->x + (player->GetDstRect()->w / 2)) < (enemy_dest_rect.x + (enemy_dest_rect.w / 2)))
		enemy_dest_rect.x += (-1 * movement_speed);
	else
		enemy_dest_rect.x += movement_speed;

	enemy_coll_rect.x = enemy_dest_rect.x + (enemy_dest_rect.w / 2) - (enemy_coll_rect.w / 2);
	enemy_coll_rect.y = enemy_dest_rect.y + (enemy_dest_rect.h / 2) - (enemy_coll_rect.h / 2);
	enemy_coll_rect.w = enemy_dest_rect.w / 2;
	enemy_coll_rect.h = enemy_dest_rect.h / 2;
}

bool IceCrystal::IsReadyToAttack()
{

	if (state == "main")
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

}

void IceCrystal::Draw(SDL_Renderer* renderer, bool collision_box_flag)
{
	std::cout << "[*] Drawing Ice Crystal\n";

	current_animation->Draw(renderer, enemy_dest_rect, SDL_FLIP_NONE);

	for (auto& animation : overlay_animations)
	{
		std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
		SDL_Rect* current_frame = animation->GetCurrentFrame();
		SDL_Rect temp = { (enemy_dest_rect.x + enemy_dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2,
							(enemy_dest_rect.y + enemy_dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
							current_frame->w,
							current_frame->h };
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
StormCloud::StormCloud(AnimationManager* animation_manager, int screen_width, int screen_height, int player_x, int player_y)
	: Enemy({ -32,-32,48 * 4 ,32 * 4 }, { -32,-32,48 * 4,32 * 4 }, 4.7, 30, 0, 35)
{
	

	animations["main"] = *animation_manager->Get("enemy-storm-cloud", "main");
	animations["attack"] = *animation_manager->Get("enemy-storm-cloud", "attack");
	animations["death"] = *animation_manager->Get("enemy-storm-genie", "death");
	animations["heal"] = *animation_manager->Get("overlays", "heal");
	current_animation = &animations["main"];
	
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

void StormCloud::Update(Player* player)
{
	float threshhold = 85;
	std::cout << "STATE: " << state <<  "Shot Fired: " << shot_fired << "=============================================" << std::endl;

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
		if ((SDL_GetTicks() - start_of_wait_state) > time_to_wait_ms)
		{
			if (first_time_waiting)
			{
				state = "shoot";

				
				first_time_waiting = false;
			}
			else
				state = "retreat";
		}
	}

	if (state == "shoot")
	{
		if (!shot_fired)
			current_animation = &animations["attack"];

		std::cout << "[*] Storm cloud shooting\n";
		if (shot_fired)
		{
			direction_x *= -1;
			direction_y *= -1;
			start_of_wait_state = SDL_GetTicks();
			state = "wait"; //WILL BE LEAVE SCREEN
		}
	}

	if (state == "retreat")
	{
		Move(player);
	}

	if (state == "death")
	{
		if (current_animation->GetCurrentFrameIndex() == 1)
		{
			overlay_animations.push_back(std::make_unique<Animation>(animations["heal"]));
		}
		enemy_coll_rect = { 0,0,0,0 };
		if (current_animation->IsFinished())
		{
			state = "delete";
		}

		current_animation = &animations["death"];
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
	std::cout << "[*] Drawing Storm Cloud\n";

	current_animation->Draw(renderer, enemy_dest_rect, SDL_FLIP_NONE);

	for (auto& animation : overlay_animations)
	{
		std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
		SDL_Rect* current_frame = animation->GetCurrentFrame();
		SDL_Rect temp = { (enemy_dest_rect.x + enemy_dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2,
							(enemy_dest_rect.y + enemy_dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
							current_frame->w,
							current_frame->h };
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
StormGenie::StormGenie(AnimationManager* animation_manager, const SDL_Rect& dest_rect)
	: Enemy(dest_rect, dest_rect, 2, 100, 0, 10)
{
	animations["spawn"] = *animation_manager->Get("enemy-storm-genie", "spawn");
	animations["main"] = *animation_manager->Get("enemy-storm-genie", "main");
	animations["attack"] = *animation_manager->Get("enemy-storm-genie", "attack");
	animations["death"] = *animation_manager->Get("enemy-storm-genie", "death");
	animations["heal"] = *animation_manager->Get("overlays", "heal");
	current_animation = &animations["spawn"];
	
	fire_cooldown_ms = 4000;
	state = "spawn";
}

void StormGenie::Update(Player* player)
{
	if (state == "spawn")
	{
		current_animation = &animations["spawn"];

		if (current_animation->IsFinished())
		{
			animations["main"].Reset();
			current_animation = &animations["main"];
			state = "main";
		}
	}

	if (state == "main")
	{
		if (enemy_dest_rect.y == 0 || enemy_dest_rect.y + enemy_dest_rect.y == 2000) //Screen width
		{
			movement_speed *= -1;
		}

		Move(player);
		
		// READY TO ATTACK
		Uint32 current_time = SDL_GetTicks();
		int vertical_difference = enemy_dest_rect.y - player->GetDstRect()->y;

		if ((current_time - last_fire_time) >= fire_cooldown_ms && abs(vertical_difference) <  100)
		{
			animations["attack"].Reset();
			current_animation = &animations["attack"];
			last_fire_time = current_time;
			state = "attacking";
		}
		// END READY TO ATTACK

	}

	if (state == "attacking")
	{

		if (current_animation->IsFinished())
		{
			animations["main"].Reset();
			current_animation = &animations["main"];

			state = "main";
			// MAKE GENIE GO UP for some Time, then go back towards the player, like the Mario3 angry sun
		}
	}

	if (state == "death")
	{
		current_animation = &animations["death"];
		enemy_coll_rect = { 0,0,0,0 };

		if (current_animation->GetCurrentFrameIndex() == 1)
		{
			overlay_animations.push_back(std::make_unique<Animation>(animations["heal"]));
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

void StormGenie::Move(Player* player)
{

	int vertical_difference = enemy_dest_rect.y - player->GetDstRect()->y;

	// Only move if the difference is great enough.
	if (abs(vertical_difference) > 30)
	{
		if (vertical_difference > 0)
		{
			enemy_dest_rect.y += movement_speed;
		}

		else
		{
			enemy_dest_rect.y -= movement_speed;
		}
	}

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
	std::cout << "[*] Drawing Storm Genie\n";

	current_animation->Draw(renderer, enemy_dest_rect, SDL_FLIP_NONE);
	for (auto& animation : overlay_animations)
	{
		std::cout << "DRAWING OVERLAY ! ----------------------------------------" << std::endl;
		SDL_Rect* current_frame = animation->GetCurrentFrame();
		SDL_Rect temp = { (enemy_dest_rect.x + enemy_dest_rect.w / 2) - current_frame->w * animation->GetScale() / 2,
							(enemy_dest_rect.y + enemy_dest_rect.h / 2) - current_frame->h * animation->GetScale() / 2,
							current_frame->w,
							current_frame->h };
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


