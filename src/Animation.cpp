#include "Animation.hpp"
#include <SDL.h>
#include <iostream>
#include <string>
#include <algorithm>

Animation::Animation()
	: texture(nullptr),
	frames(),
	frame_time_ms(0),
	loop_flag(false),
	current_frame(0),
	last_update_time(0),
	finished(false),
	output_name("BLANK"),
	scale(1.0){}

Animation::Animation(SDL_Texture* texture,
					 const std::vector<SDL_Rect>& frames,
					 int frame_time_ms,
					 bool loop_flag,
					 std::string output_name,
					 float scale)
	  : texture(texture),
		frames(frames),
		frame_time_ms(frame_time_ms),
		loop_flag(loop_flag),
		current_frame(0),
		last_update_time(SDL_GetTicks()),
		finished(false),
		output_name(output_name),
		scale(scale){}

Animation::~Animation() {}

void Animation::Update() {
	
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
	const SDL_Rect& dest_rect,
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
	
	SDL_Rect scaled_dest_rect = dest_rect;
	scaled_dest_rect.w = static_cast<int>(scaled_dest_rect.w);
	scaled_dest_rect.h = static_cast<int>(scaled_dest_rect.h);

	if (SDL_RenderCopy(renderer, texture, &frames[current_frame], &scaled_dest_rect) != 0)
	{
		std::cerr << "SDL_RenderCopyEx Error: " << SDL_GetError() << std::endl;

	}
}
void Animation::DrawPartial(SDL_Renderer* renderer,
	const SDL_Rect& dest_rect,
	float percent,
	float frame_width,
	SDL_RendererFlip flip)
{
	if (!texture || frames.empty())
		return;

	// Clamp percent
	percent = std::clamp(percent, 0.0f, 1.0f);
	if (percent <= 0.0f)
		return;

	// Quantize percent to the nearest visible pixel
	// For a 48px-wide bar, each pixel step is 1/48 ≈ 0.0208333
	const float pixel_step = 1.0f / frame_width;
	percent = std::round(percent / pixel_step) * pixel_step;

	SDL_Rect full_src = frames[current_frame];

	int partial_src_w = static_cast<int>(std::round(full_src.w * percent));
	int partial_dest_w = static_cast<int>(std::round(dest_rect.w * percent));

	SDL_Rect partial_src = full_src;
	partial_src.w = partial_src_w;

	SDL_Rect partial_dest = dest_rect;
	partial_dest.w = partial_dest_w;

	// Fix left edge so bar fills left → right
	partial_dest.x = dest_rect.x;

	if (SDL_RenderCopyEx(renderer, texture, &partial_src, &partial_dest, 0.0, nullptr, flip) != 0)
		std::cerr << "SDL_RenderCopyEx Error: " << SDL_GetError() << std::endl;
}


bool Animation::IsFrameDone()
{
	
	
	Uint32 current_time = SDL_GetTicks();
	//std::cout << "Last Update Time: " << last_update_time << " Frame Time (ms): " << frame_time_ms << " Current Time" << current_frame << std::endl;
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
	last_update_time = SDL_GetTicks();
	finished = false;
}

bool Animation::IsFinished() const 
{
	return finished;
}