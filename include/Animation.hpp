#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <SDL.h>
#include <vector>

class Animation
{
public:
	Animation(	SDL_Texture* texture,
				const std::vector<SDL_Rect>& frames,
				int frame_time_ms,
				bool loop_flag);

	~Animation();

	
	// Every Frame Updates ?
	void Update(Uint32 time_delta);
	void Draw(	SDL_Renderer* renderer,
				const SDL_Rect& dest_rect,
				SDL_RendererFlip flip = SDL_FLIP_NONE);

	// Utilities
	void Reset();
	bool IsFinished() const;
	// Set loop_flag_flag false ? Player iframes?

	// GETTERS
	SDL_Texture* GetTexture() const { return texture; }
	int GetCurrentFrameIndex() const { return current_frame; }

private:
	SDL_Texture* texture;
	std::vector<SDL_Rect> frames;
	int frame_time_ms;
	bool loop_flag;

	int current_frame;
	int time_accumulator_ms;
	bool finished;
};
#endif
