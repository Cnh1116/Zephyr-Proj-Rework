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
#include "OverlayTextManager.hpp"


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
    void RenderDebugText(Player* player, Uint32 game_timer);
    void RenderPlayerUI(Player* player);

    // Textures
    void LoadTextures();
    SDL_Texture* GetTexture(const char* png_path);

    // FONT
    void RenderText(const std::string& text, const SDL_Rect& rect, SDL_Color color);
    TTF_Font* GetFont(int font_id);

    // Main Render Functions
    void RenderGameItems(Player* player,
                        std::vector<Projectile*> &game_projectiles, 
                        ItemManager& item_manager, 
                        std::vector<Enemy*>& enemies, 
                        OverlayTextManager& overlay_text_manager, 
                        bool render_coll_boxes,
                        Uint32 game_timer);
    void BackgroundUpdate(Uint32 loop_flag);

    // Menu Render Functions
	void RenderPauseMenu(int current_option);

	SDL_Texture* GetCurrentScreenTexture();


    int GetScreenHeight();
    int GetScreenWidth();

    SDL_Renderer* GetRenderer();

    int GenRandomNumber(int low, int high);


    

private:
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Background Stuff
    SDL_Rect clouds1L_dest, clouds1R_dest,  clouds2L_dest, clouds2R_dest, clouds3L_dest, clouds3R_dest;
    int clouds1_animation_index;

    int screen_width;
    int screen_height;
    int pixel_scale;

    int cloud1R_speed;
    int cloud1L_speed;
    int cloud2R_speed;
    int cloud2L_speed;
    int cloud3R_speed;
    int cloud3L_speed;

    bool is_shown;

    std::map<std::string, SDL_Texture*> texture_map;
    TTF_Font* font_1;
    TTF_Font* font_2;
    
    
    // Initialize SDL, create window and renderer
    bool init(const char* title, int width, int height);

    bool IsFrameDone(Uint32 frame_time_ms_ms, Uint32 last_frame_time_ms);

    

    
};

#endif