
#include <SDL.h>
#include <random>
#include <iostream>

#include "Enemy.hpp"
#include "Player.hpp"

Enemy::Enemy(const SDL_Rect& dest_rect, const SDL_Rect& coll_rect, std::vector<SDL_Rect> main_frames_arg, std::vector<SDL_Rect> death_frames_arg,  float move_speed, int health_arg, float crit, float start_damage)
{
	enemy_dest_rect = dest_rect;
	enemy_coll_rect = coll_rect;
	main_frames = main_frames_arg;
	death_frames = death_frames_arg;
	last_frame_time = 0;
	
	movement_speed = move_speed;
	base_health = health_arg;
	base_damage = start_damage;

	invincible = false;
	state = "main";
}

void Enemy::AdvanceFrame()
{
	current_frame_index++;
}

std::string Enemy::GetState()
{
	return state;
}

std::string Enemy::GetTextureKey()
{
	return (current_texture_key);
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

SDL_Rect* Enemy::GetFrame()
{
	return &current_frames[current_frame_index];
}

int Enemy::NumOfFrames()
{
	return current_frames.size();
}

void Enemy::UpdateState(std::string new_state)
{
	state = new_state;
	current_frame_index = 0;
}

Uint32 Enemy::GetLastFrameStart()
{
	return last_frame_time;
}
Uint32 Enemy::GetFrameTime()
{
	return frame_time_ms;
}

void Enemy::SetLastFrameTime(Uint32 current_time) 
{
	last_frame_time = current_time;
}

void Enemy::SetFrameIndex(int index)
{
	current_frame_index = index;
}

int Enemy::GetFrameIndex()
{
	return current_frame_index;
}

int Enemy::GetHealth()
{
	return base_health;
}
void Enemy::ChangeHealth(int health_diff)
{
	base_health += health_diff;
}





// ICE CRYSTAL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
IceCrystal::IceCrystal(const SDL_Rect& dest_rect)
	: Enemy(dest_rect, dest_rect, { {0,0,64,64}, {64,0,64,64}, {128,0,64,64}, {192,0,64,64}, {256,0,64,64}, {320,0,64,64}, {384,0,64,64}, {448,0,64,64}, {512,0,64,64}, {576,0,64,64}, {640,0,64,64}, {704,0,64,64}},
		{{ 0,0,64,64 }, { 64,0,64,64 }, { 128,0,64,64 }, { 192,0,64,64 }, { 256,0,64,64 }, { 320,0,64,64 }, { 384,0,64,64 }, { 448,0,64,64 }, { 512,0,64,64 }, { 576,0,64,64 }, { 640,0,64,64 }, { 704,0,64,64 }}, 1.0, 100, 0, 35)
{}

void IceCrystal::Update(Player *player)
{

	if (state == "main")
	{
		
		if (enemy_dest_rect.x == 0 || enemy_dest_rect.x + enemy_dest_rect.w == 2000) //Screen width
		{
			movement_speed *= -1;
		}

		current_texture_key = "purple_crystal_main";
		current_frames = main_frames;
		Move(player);
	}



	if (state == "death")
	{
		enemy_coll_rect = { 0,0,0,0 };
		if (current_frame_index >= death_frames.size() - 1)
		{
			state = "delete";
		}
		
		current_texture_key = "purple_crystal_death";
		current_frames = death_frames;


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

// STORM CLOUD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StormCloud::StormCloud(int screen_width, int screen_height, int player_x, int player_y)
	: Enemy({-32,-32,32,32}, { -32,-32,32,32 }, { {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}, {128,0,32,32}, {160,0,32,32}, {192,0,32,32}, {224,0,32,32}, {256,0,32,32}, {288,0,32,32}, {320,0,32,32}, {352,0,32,32}, {384,0,32,32}, {416,0,32,32}, {448,0,32,32} },
		{ {0,0,32,32}, {32,0,32,32}, {64,0,32,32}, {96,0,32,32}, {128,0,32,32}, {160,0,32,32}, {192,0,32,32}, {224,0,32,32}, {256,0,32,32}, {288,0,32,32}, {320,0,32,32}, {352,0,32,32}, {384,0,32,32}, {416,0,32,32}, {448,0,32,32} }, 4.0, 35, 0, 35)
{
	


	std::cout << "[*] Goal x and y: " << goal_x << " " << goal_y << std::endl;
	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> location_zone(0,1000);
	std::uniform_int_distribution<> within_screen_y(0, screen_height * 0.55);
	std::uniform_int_distribution<> within_screen_x(0, screen_width);

	if (location_zone(gen) % 3 == 0) //LEFT
	{
		enemy_dest_rect = { -32, within_screen_y(gen), 32 * 2, 32 * 2 };
		goal_x = player_x - 100;
		goal_y = player_y;
	}
	
	else if (location_zone(gen) % 3 == 1) //RIGHT
	{
		enemy_dest_rect = { screen_width + 32, within_screen_y(gen), 32 * 2, 32 * 2 };
		goal_x = player_x + 100;
		goal_y = player_y;
	}
	else //UP
	{
		enemy_dest_rect = { within_screen_x(gen), -32, 32 * 2, 32 * 2 };
		goal_x = player_x;
		goal_y = player_y - 100;
	}

	// Calculate the direction vector
	float delta_x = player_x - enemy_dest_rect.x + (enemy_dest_rect.w / 2);
	float delta_y = player_y - enemy_dest_rect.y + (enemy_dest_rect.h/2);
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
	float threshhold = 325;

	if (state == "main")
	{
		std::cout << "[*] Main\n";
		Move(player);
		if ((std::abs(enemy_dest_rect.x - goal_x) < threshhold) && (std::abs(enemy_dest_rect.y - goal_y) < threshhold))
		{
			state = "wait";
			start_of_wait_state = SDL_GetTicks();
		}


		current_texture_key = "storm_cloud_main";
		current_frames = main_frames;
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
		enemy_coll_rect = { 0,0,0,0 };
		if (current_frame_index >= death_frames.size() - 1)
		{
			state = "delete";
		}

		current_texture_key = "storm_cloud_main";
		current_frames = death_frames;
	}
}

void StormCloud::Move(Player* player)
{
	std::cout << "[*] Speed: " << movement_speed << " " << direction_x << " " << direction_y << std::endl;

	
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
	if (state == "shoot" && !shot_fired)
	{
		shot_fired = true;
		return true;
	}
	else
		return false;
}

int StormCloud::GetGoalX()
{
	return goal_x;
}

int StormCloud::GetGoalY()
{
	return goal_y;
}