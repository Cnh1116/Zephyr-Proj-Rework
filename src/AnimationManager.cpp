#include "AnimationManager.hpp"
#include <fstream>
#include <SDL_Image.h>
#include <iostream>

using json = nlohmann::json;

AnimationManager::AnimationManager(SDL_Renderer* renderer)
    : renderer(renderer) {}

AnimationManager::~AnimationManager() {
    for (auto& cat : animations) {
        for (auto& entry : cat.second)
            delete entry.second;
    }
}

bool AnimationManager::LoadFromFile(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open animation JSON: " << jsonPath << "\n";
        return false;
    }

    json data;
    file >> data;

    for (auto& category : data.items()) {
        std::string categoryName = category.key();

        for (auto& anim : category.value().items()) {
            AnimationData a;
            a.name = anim.key();

            auto& d = anim.value();
            a.texture_path = d["texture"].get<std::string>();
            a.frame_width = d["frame_width"].get<int>();
            a.frame_height = d["frame_height"].get<int>();
            a.frame_count = d["frame_count"].get<int>();
            a.frame_time_ms = d["frame_time_ms"].get<int>();
            a.loop_flag = d.value("loop_flag", true);
            a.vertical = d.value("vertical", false);

            animations[categoryName][a.name] = CreateAnimation(a);
        }
    }

    std::cout << "Loaded animations from " << jsonPath << "\n";
    return true;
}

Animation* AnimationManager::CreateAnimation(const AnimationData& data) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, data.texture_path.c_str());
    if (!texture) {
        std::cerr << "Failed to load texture: " << data.texture_path
            << " SDL_Error: " << SDL_GetError() << "\n";
        return nullptr;
    }

    std::vector<SDL_Rect> frames;
    for (int i = 0; i < data.frame_count; ++i) {
        SDL_Rect rect;
        if (data.vertical) {
            rect = { 0, i * data.frame_height, data.frame_width, data.frame_height };
        }
        else {
            rect = { i * data.frame_width, 0, data.frame_width, data.frame_height };
        }
        frames.push_back(rect);
    }

    return new Animation(texture, frames, data.frame_time_ms, data.loop_flag);
}

Animation* AnimationManager::Get(const std::string& category, const std::string& name) {
    auto catIt = animations.find(category);
    if (catIt == animations.end()) return nullptr;

    auto animIt = catIt->second.find(name);
    if (animIt == catIt->second.end()) return nullptr;

    return animIt->second;
}
