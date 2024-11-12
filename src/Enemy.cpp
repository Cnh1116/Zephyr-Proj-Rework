
#include <SDL.h>
#include <random>
#include <iostream>

#include "Enemy.hpp"
#include "Player.hpp"

Enemy::Enemy(const SDL_Rect& dest_rect, const SDL_Rect& coll_rect, std::vector<SDL_Rect> spawn_frames_arg, std::vector<SDL_Rect> main_frames_arg, std::vector<SDL_Rect> attack_frames_arg, std::vector<SDL_Rect> death_frames_arg,  float move_speed, int health_arg, float crit, float start_damage)
{
	enemy_dest_rect = dest_rect;
	enemy_coll_rect = coll_rect;
	spawn_frames = spawn_frames_arg;
	main_frames = main_frames_arg;
	attack_frames = attack_frames_arg;
	death_frames = death_frames_arg;
	last_frame_time_ms = 0;
	
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
	return last_frame_time_ms;
}
Uint32 Enemy::GetFrameTime()
{
	return frame_time_ms_ms;
}

void Enemy::SetLastFrameTime(Uint32 current_time) 
{
	last_frame_time_ms = current_time;
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
	: Enemy(dest_rect, dest_rect, 
		{ {0,0,64,64}, {64,0,64,64}, {128,0,64,64}, {192,0,64,64}, {256,0,64,64}, {320,0,64,64}, {384,0,64,64}, {448,0,64,64}, {512,0,64,64}, {576,0,64,64}, {640,0,64,64}, {704,0,64,64} }, // Spawn frame
		{ {0,0,64,64}, {64,0,64,64}, {128,0,64,64}, {192,0,64,64}, {256,0,64,64}, {320,0,64,64}, {384,0,64,64}, {448,0,64,64}, {512,0,64,64}, {576,0,64,64}, {640,0,64,64}, {704,0,64,64}}, // Main frames
		{ {0,0,64,64}, {64,0,64,64}, {128,0,64,64}, {192,0,64,64}, {256,0,64,64}, {320,0,64,64}, {384,0,64,64}, {448,0,64,64}, {512,0,64,64}, {576,0,64,64}, {640,0,64,64}, {704,0,64,64} }, // Attack Frames
		{{ 0,0,64,64 }, { 64,0,64,64 }, { 128,0,64,64 }, { 192,0,64,64 }, { 256,0,64,64 }, { 320,0,64,64 }, { 384,0,64,64 }, { 448,0,64,64 }, { 512,0,64,64 }, { 576,0,64,64 }, { 640,0,64,64 }, { 704,0,64,64 }}, // Death frames 
		1.0, 100, 0, 35)
{
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
	: Enemy({ -32,-32,48 * 4 ,32 * 4 }, { -32,-32,48 * 4,32 * 4 }, 
		{ {0,0,48,32} }, // Spawn Frames
		{ {0,0,48,32} }, // Main Frames
		{ {0, 0, 48, 32}, {48, 0, 48, 32}, {96, 0, 48, 32}, {144, 0, 48, 32 }, {192, 0, 48, 32 }, {240, 0, 48, 32}, {288, 0, 48, 32} }, // Attack Frames
		{ {{0, 0, 48, 32}, {48, 0, 48, 32}, {96, 0, 48, 32}, {144, 0, 48, 32 }, {192, 0, 48, 32 }, {240, 0, 48, 32}, {288, 0, 48, 32}} }, // Death Frames
		4.7, 30, 0, 35)
{
	


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

	if (state == "main")
	{
		Move(player);
		if ((std::abs(enemy_dest_rect.x - goal_x) < threshhold) && (std::abs(enemy_dest_rect.y - goal_y) < threshhold))
		{
			state = "wait";
			start_of_wait_state = SDL_GetTicks();
		}


		current_texture_key = "storm_cloud_attack"; //Still image of the first frame for the main image
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
		if (!shot_fired)
			current_frames = attack_frames;

		std::cout << "[*] Storm cloud shooting\n";
		if (shot_fired)
		{
			current_frame_index = 0;
			current_frames = main_frames;
			direction_x *= -1;
			direction_y *= -1;
			start_of_wait_state = SDL_GetTicks();
			state = "wait"; //WILL BE LEAVE SCREEN
		}
	}

	if (state == "retreat")
	{
		current_frame_index = 0;
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

		current_texture_key = "storm_cloud_death";
		current_frames = death_frames;
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
	if (state == "shoot" && !shot_fired && current_frame_index == 3)
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

// STORM GENIE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StormGenie::StormGenie(const SDL_Rect& dest_rect)
	: Enemy(dest_rect, dest_rect,
		{ {0,0,64,64}, {64,0,64,64}, {128,0,64,64}, {192,0,64,64}, {256,0,64,64}, {320,0,64,64}, {384,0,64,64}, {448,0,64,64}, {512,0,64,64}, {576,0,64,64}, {640,0,64,64}, {704,0,64,64}, {768,0,64,64}, {832,0,64,64}, {896,0,64,64}, {960,0,64,64} }, // Spawn frame
		{ {1024,0,64,64}, {1088,0,64,64}, {1152,0,64,64}, {1216,0,64,64} }, // Main frames
		{ {1280,0,64,64}, {1344,0,64,64}, {1408,0,64,64}, {1472,0,64,64}, {1536,0,64,64}, {1536,0,64,64}, {1536,0,64,64}, {1536,0,64,64}, {1536,0,64,64}, {1600,0,64,64} }, // Attack Frames
		{ {0, 0, 48, 32}, {48, 0, 48, 32}, {96, 0, 48, 32}, {144, 0, 48, 32 }, {192, 0, 48, 32 }, {240, 0, 48, 32}, {288, 0, 48, 32} }, // Death frames 
		2, 100, 0, 10)
{
	fire_cooldown_ms = 4000;
	frame_time_ms_ms = 120;
	state = "spawn";
	current_texture_key = "storm_genie";
	current_frame_index = 0;
}

void StormGenie::Update(Player* player)
{
	if (state == "spawn")
	{
		current_frames = spawn_frames;
		if (current_frame_index == spawn_frames.size() - 1)
		{
			state = "main";
			current_frame_index = 0;
		}
	}

	if (state == "main")
	{

		if (enemy_dest_rect.y == 0 || enemy_dest_rect.y + enemy_dest_rect.y == 2000) //Screen width
		{
			movement_speed *= -1;
		}
		current_frames = main_frames;
		Move(player);
		
		Uint32 current_time = SDL_GetTicks();
		int vertical_difference = enemy_dest_rect.y - player->GetDstRect()->y;

		if ((current_time - last_fire_time) >= fire_cooldown_ms && abs(vertical_difference) <  100)
		{
			last_fire_time = current_time;
			state = "attacking";
		}

	}

	if (state == "attacking")
	{
		current_frames = attack_frames;
		if (current_frame_index == attack_frames.size() - 1)
		{
			current_frame_index = 0;
			state = "main";
			// MAKE GENIE GO UP for some Time, then go back towards the player, like the Mario3 angry sun
		}
	}

	if (state == "death")
	{
		enemy_coll_rect = { 0,0,0,0 };
		current_texture_key = "storm_cloud_death";
		if (current_frame_index >= death_frames.size() - 1)
		{
			state = "delete";
		}

		
		current_frames = death_frames;


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

	if (state == "attacking" && current_frame_index == 4)
		return true;
	else
		return false;

}