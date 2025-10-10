#include "Animation.hpp"
#include <SDL.h>
#include <iostream>
#include <string>

Animation::Animation()
	: texture(nullptr),
	frames(),
	frame_time_ms(0),
	loop_flag(false),
	current_frame(0),
	last_update_time(0),
	finished(false),
	output_name("BLANK") {}

Animation::Animation(SDL_Texture* texture,
					 const std::vector<SDL_Rect>& frames,
					 int frame_time_ms,
					 bool loop_flag,
					 std::string output_name)
	  : texture(texture),
		frames(frames),
		frame_time_ms(frame_time_ms),
		loop_flag(loop_flag),
		current_frame(0),
		last_update_time(0),
		finished(false),
		output_name(output_name){}

Animation::~Animation() {}

void Animation::Update(Uint32 time_delta) {
	
	if (finished || frames.empty()) 
	{
		return;
	}


	if (IsFrameDone())
	{
		last_update_time = SDL_GetTicks();

		if (loop_flag)
		{
			if (current_frame == frames.size() - 1)
				current_frame = 0;
			else
				current_frame++;
		}
		else
		{
			if (current_frame < frames.size() - 1 )
			{
				current_frame++;
			}
			else
			{
				finished = true;
			}
		}
	}
}

void Animation::Draw(SDL_Renderer* renderer,
	const SDL_Rect& destRect,
	SDL_RendererFlip flip)
{
	if (!texture || frames.empty())
	{
		std::cerr << "Error: No texture or frames to draw.";
		return;
	}
	int w, h;
	if (SDL_QueryTexture(texture, nullptr, nullptr, &w, &h) != 0)
	{
		std::cerr << "SDL_QueryTexture Error: " << SDL_GetError() << std::endl;
		exit(1);
	}
	
	const SDL_Rect& srcRect = frames[current_frame];
	if (SDL_RenderCopy(renderer, texture, &srcRect, &destRect) != 0)
	{
		std::cerr << "SDL_RenderCopyEx Error: " << SDL_GetError() << std::endl;

	}
}

bool Animation::IsFrameDone()
{
	Uint32 current_time = SDL_GetTicks();

	if ((current_time - last_update_time) >= frame_time_ms)
	{
		return(true);
	}

	else
	{
		//std::cout << "[*] Frame is not done\n";
		return(false);
	}
}

void Animation::OutputInformation()
{
	std::cout << "Name: " << output_name << " Finished?: " << finished << " Loop?: " << loop_flag << " Current Frame: #" << current_frame + 1 << "/" << frames.size() << "\n";
	std::cout << "Source Frame Cords X: " << frames[current_frame].x << " Y: " << frames[current_frame].y << " W: " << frames[current_frame].w << " H: " << frames[current_frame].h << "\n";
}

void Animation::Reset() {
	current_frame = 0;
	last_update_time = 0;
	finished = false;
}

bool Animation::IsFinished() const 
{
	return finished;
}