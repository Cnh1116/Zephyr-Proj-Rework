#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <map>
#include <memory>
#include <SDL_ttf.h>

#include "Projectiles.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "Enemy.hpp"


class Player;


class Graphics 
{
public:
    // Constructor
    Graphics(const char* title, int width, int height, int pixel_scale);
    ~Graphics();

    //Window Manipulation
    void HideWindow();
    void ShowWindow();
    void DeactivateWindow();

    // Text
    void RenderPlayerText(Player* player);

    // Textures
    void LoadTextures();
    SDL_Texture* GetTexture(const char* png_path);

    void RenderText(const std::string& text, const SDL_Rect& rect, SDL_Color color);
    void RenderGameItems(Player* player, std::vector<Projectile*> &game_projectiles, std::vector<ItemManager::item>* item_list, std::vector<Enemy*>& enemies);
    void BackgroundUpdate(Uint32 loop);


    int GetScreenHeight();
    int GetScreenWidth();

    SDL_Renderer* GetRenderer();


    

private:
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Background Stuff
    SDL_Rect clouds1_src, clouds1_dest;
    int clouds1_animation_index;

    int screen_width;
    int screen_height;
    int pixel_scale;
    
    bool is_shown;

    std::map<std::string, SDL_Texture*> texture_map;
    TTF_Font* font_1;
    
    
    // Initialize SDL, create window and renderer
    bool init(const char* title, int width, int height);

    bool IsFrameDone(Uint32 frame_time_ms, Uint32 last_frame_time);

    

    
};

#endif