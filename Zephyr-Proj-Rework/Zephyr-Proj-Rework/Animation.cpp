#include "Animation.hpp"
#include <SDL.h>

Animation::Animation(SDL_Texture* texture,
					 const std::vector<SDL_Rect>& frames,
					 int frame_time_ms_ms,
					 bool loop_flag_flag)
	  : texture(texture),
		frames(frames),
		frame_time_ms(frame_time_ms_ms),
		loop_flag(loop_flag_flag),
		current_frame(0),
		time_accumulator_ms(0),
		finished(false) {}

Animation::~Animation() {}

void Animation::Update(Uint32 deltaTime) {
	if (finished || frames.empty()) return;

	time_accumulator_ms += deltaTime;

	// Advance by however many frames worth of time have passed
	while (time_accumulator_ms >= frame_time_ms) {
		time_accumulator_ms -= frame_time_ms;
		current_frame++;

		if (current_frame >= static_cast<int>(frames.size())) {
			if (loop_flag) {
				current_frame = 0;
			}
			else {
				current_frame = static_cast<int>(frames.size()) - 1;
				finished = true;
				break;
			}
		}
	}
}

void Animation::Draw(SDL_Renderer* renderer,
	const SDL_Rect& destRect,
	SDL_RendererFlip flip)
{
	if (!texture || frames.empty()) return;

	const SDL_Rect& srcRect = frames[current_frame];
	SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0.0, nullptr, flip);
}

void Animation::Reset() {
	current_frame = 0;
	time_accumulator_ms = 0;
	finished = false;
}

bool Animation::IsFinished() const {
	return finished;
}