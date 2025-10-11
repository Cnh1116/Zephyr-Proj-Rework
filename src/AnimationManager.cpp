#include "AnimationManager.hpp"
#include <fstream>
#include <SDL_Image.h>
#include <iostream>
#include <filesystem>
#include <string>

using json = nlohmann::json;

AnimationManager::AnimationManager(SDL_Renderer* renderer)
    : renderer(renderer) {
    LoadFromFile("..\\..\\configs\\animations.json");
}

AnimationManager::~AnimationManager() {
    for (auto& cat : animations) {
        for (auto& entry : cat.second)
            delete entry.second;
    }
}

bool AnimationManager::LoadFromFile(const std::string& jsonPath) {

    std::cout << "Current working directory: " << std::filesystem::current_path() << "\n";
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open animation JSON file: " << jsonPath << "\n";
        exit(1);
        return false;
    }

    json data;
    file >> data;

    for (auto& category : data.items()) {  
       std::string categoryName = category.key();  
       std::cout << "Category: " << categoryName << "\n";  

       for (auto& anim : category.value().items()) {  
           AnimationData a;  
           a.name = anim.key();  
           std::cout << "  Animation Name: " << a.name << "\n";  

           auto& d = anim.value();  
           a.texture_path = d["texture"].get<std::string>();  
           a.frame_width = d["frame_width"].get<int>();  
           a.frame_height = d["frame_height"].get<int>();  
           a.frame_count = d["frames"].get<int>();  
           a.frame_time_ms = d["frame_time_ms"].get<int>();  
           a.loop_flag = d.value("loop_flag", true);  
           std::string layout = d.value("layout", "horizontal"); // Default to "horizontal"
           a.vertical = (layout == "vertical");
           //a.vertical = d.value("vertical", false);  
		   a.output_name = categoryName + "-" + a.name;
           a.scale = d.value("scale", 1.0f);


           std::cout << "    Texture Path: " << a.texture_path << "\n";  
           std::cout << "    Frame Width: " << a.frame_width << "\n";  
           std::cout << "    Frame Height: " << a.frame_height << "\n";  
           std::cout << "    Frame Count: " << a.frame_count << "\n";  
           std::cout << "    Frame Time (ms): " << a.frame_time_ms << "\n";  
           std::cout << "    Loop Flag: " << a.loop_flag << "\n";  
           std::cout << "    Vertical: " << (a.vertical ? "true" : "false") << "\n";  


           animations[categoryName][a.name] = CreateAnimation(a);  
       }  
    }

    std::cout << "Loaded animations from " << jsonPath << "\n";
    return true;
}

Animation* AnimationManager::CreateAnimation(const AnimationData& data) 
{
   
     std::string base_path = std::filesystem::current_path().string();
     std::cout << "Current working directory: " << base_path  << "\n";
     std::string asset_path = base_path + "\\" + data.texture_path;

    SDL_Texture* texture = IMG_LoadTexture(renderer, asset_path.c_str());
    if (!texture) {
        std::cerr << "Failed to load texture: " << asset_path
            << " SDL_Error: " << SDL_GetError() << "\n";
        exit(1);
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
    std::cout << "Created animation: " << data.output_name << " LOOP FLAG: " << data.loop_flag << std::endl;
    return new Animation(texture, frames, data.frame_time_ms, data.loop_flag, data.output_name, data.scale);
}

Animation* AnimationManager::Get(const std::string& category, const std::string& name) 
{
    auto catIt = animations.find(category);
    if (catIt == animations.end())
    {
        std::cerr << "Category '" << category << "' not found.\n";
        exit(1);
    }

    auto animIt = catIt->second.find(name);
    if (animIt == catIt->second.end()) 
    {
        std::cerr << "Category '" << category << "' not found.\n";
        exit(1);
    }

    return animIt->second;
}
