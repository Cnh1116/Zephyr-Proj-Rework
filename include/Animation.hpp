#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <SDL.h>
#include <vector>
#include <string>

class Animation
{
public:

	Animation();
	Animation(	SDL_Texture* texture,
				const std::vector<SDL_Rect>& frames,
				int frame_time_ms,
				bool loop_flag,
				std::string output_name,
				float scale);

	~Animation();

	
	// Every Frame Updates ?
	void Update();
	void Draw(	SDL_Renderer* renderer,
				const SDL_Rect& dest_rect,
				SDL_RendererFlip flip = SDL_FLIP_NONE);

	// Utilities
	void Reset();
	bool IsFinished() const;
	void OutputInformation();
	bool IsFrameDone();
	
	// Set loop_flag_flag false ? Player iframes?

	// GETTERS
	SDL_Texture* GetTexture() const { return texture; }
	SDL_Rect* GetCurrentFrame() { return &frames[current_frame]; }
	int GetCurrentFrameIndex() const { return current_frame; }
	std::string GetName() const { return output_name; }
	bool IsLooping() const { return loop_flag; }
	float GetScale() const { return scale; }

private:
	SDL_Texture* texture;
	std::vector<SDL_Rect> frames;
	int frame_time_ms;
	bool loop_flag;
	float scale = 1.0f;
	int current_frame;
	bool finished;
	std::string output_name;

	Uint32 last_update_time;
};
#endif
