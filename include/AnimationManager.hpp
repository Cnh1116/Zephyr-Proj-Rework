#ifndef ANIMATIONMANAGER_HPP
#define ANIMATIONMANAGER_HPP

#include <unordered_map>
#include <vector>
#include <SDL.h>

#include "nlohmann/json.hpp"
#include "Animation.hpp"

class AnimationManager
{
	public:
		AnimationManager(SDL_Renderer* renderer);
		~AnimationManager();

		bool LoadFromFile(const std::string& animation_json_file);

		Animation* Get(const std::string& category, const std::string& name); // NEED TO ADD ONE MORE LEVEL TO THIS SCHEME: Enemies and Projectiles have 3 levels of depth

	private:
		SDL_Renderer* renderer;


		struct AnimationData {
			std::string name;
			std::string texture_path;
			int frame_width;
			int frame_height;
			int frame_count;
			int frame_time_ms;
			bool loop_flag;
			bool vertical;
			std::string output_name;
			float scale;
		};

		std::unordered_map<std::string, std::unordered_map<std::string, Animation*>> animations;
		Animation* CreateAnimation(const AnimationData& data);

};

#endif