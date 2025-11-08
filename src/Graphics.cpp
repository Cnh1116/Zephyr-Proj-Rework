#include <iostream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <sstream>
#include <random>
#include <filesystem>

#include "Graphics.hpp"
#include "Projectiles.hpp"
#include "ItemManager.hpp"
#include "Enemy.hpp"




// Constructor
Graphics::Graphics(const char* title, int log_width, int log_height, int width, int height, int scale)
    : window(nullptr), renderer(nullptr), is_shown(false) , clouds1_animation_index(0)
{
	logical_width = log_width;
    logical_height = log_height;
    screen_width = width;
    screen_height = height;
    pixel_scale = scale;

    
    // BACKGROUND STUFF

    clouds1L_dest = { GenRandomNumber(-1200, -870), 0, 324, 576}; //FILE SIZE * arbitrary scale factor
    clouds1R_dest = { GenRandomNumber(1050, 1400), -45, 324, 576 };
    clouds2L_dest = { GenRandomNumber(-1100, -870), 0, 324, 576 };
    clouds2R_dest = { GenRandomNumber(450, 600), -45, 324, 576 };
    //clouds3L_dest
    //clouds3R_dest = 
    //clouds2_dest = { -450, 700, 576 * 4, 324 * 4 }; //FILE SIZE * arbitrary scale factor
    //clouds3_dest = { screen_height - (324 * 5 / 2), 300, 576 * 5, 324 * 5}; //FILE SIZE * arbitrary scale factor

    cloud1R_speed = 5;
    cloud1L_speed = 6;
    cloud2R_speed = 3;
    cloud2L_speed = 2;
    cloud3R_speed = 2;
    cloud3L_speed = 2;
    
    if (init(title, width, height)) 
    {
        is_shown = true;
    }

    LoadTextures();

    font_1 = TTF_OpenFont("assets/fonts/raster-forge-font/RasterForgeRegular-JpBgm.ttf",50);
    if (!font_1) 
    {
        std::cout << "[!] font 1 not initialized.";
    }
    font_2 = TTF_OpenFont("assets/fonts/raster-forge-font/RasterForgeRegular-JpBgm.ttf", 25);
    if (!font_2)
    {
        std::cout << "[!] font 2 not initialized.";
    }

}

// Destructor
Graphics::~Graphics() 
{
    std::cout << "[*] Cleaning up window./n";

    for (auto& pair : texture_map) 
    {
        SDL_DestroyTexture(pair.second); 
    }
    texture_map.clear();

    if (renderer) { SDL_DestroyRenderer(renderer); }
    if (window) { SDL_DestroyWindow(window); }
    TTF_CloseFont(font_1);
    TTF_Quit();
}

void Graphics::LoadTextures()
{
    // [ LIST OF TEXTURES ]
    // Background
     
    std::filesystem::path cwd = std::filesystem::current_path();
    std::cout << "Current Working Directory: " << cwd << std::endl;

	std::filesystem::path two_back = std::filesystem::absolute("../..");
	std::cout << "Two Back Directory: " << two_back << std::endl;

    

    texture_map["light_blue_background"] = GetTexture("assets/sprites/background-sprites/sunset-background.png");
    texture_map["clouds1_texture"] =  GetTexture("assets/sprites/background-sprites/clouds1.png");
    texture_map["clouds2_texture"] = GetTexture("assets/sprites/background-sprites/clouds2.png");
    texture_map["clouds3_texture"] = GetTexture("assets/sprites/background-sprites/clouds3.png");

    // Items
    texture_map["item_cloud"] = GetTexture("assets/sprites/item-sprites/item-cloud.png");
    texture_map["glass_toucan"] = GetTexture("assets/sprites/item-sprites/glass-toucan.png");
    texture_map["garnet_shield"] = GetTexture("assets/sprites/item-sprites/garnet-shield.png");

    
}

void Graphics::HideWindow()
{
    std::cout << "[*] HideWindow() Called";
    is_shown = false;
    SDL_HideWindow(window);  
}

void Graphics::ShowWindow()
{
    std::cout << "[*] ShowWindow() Called";
    is_shown = true;
    SDL_ShowWindow(window);
}


void Graphics::DeactivateWindow()
{
    is_shown = false;
}

// Initialize SDL, create window and renderer
bool Graphics::init(const char* title, int width, int height) 
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) 
    {
        std::cerr << "[!] SDL Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() != 0)
    {
        std::cerr << "[!] TTF Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              screen_width, screen_height,
                              SDL_WINDOW_SHOWN);

	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) 
    {
        std::cerr << "[!] Window creation error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) 
    {
        std::cerr << "[!] Renderer creation error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_RenderSetLogicalSize(renderer, logical_width, logical_height);

    return true;
}

SDL_Texture* Graphics::GetCurrentScreenTexture()
{
    // Get actual output size of renderer
    int w, h;
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0)
    {
        std::cerr << "SDL_GetRendererOutputSize failed: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    // Allocate buffer for all pixels
    std::vector<Uint32> pixels(w * h);

    // Read from the **backbuffer**
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA8888,
        pixels.data(), w * sizeof(Uint32)) != 0)
    {
        std::cerr << "SDL_RenderReadPixels failed: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    // Create texture from pixel buffer
    SDL_Texture* screen_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STATIC,
        w, h);
    if (!screen_texture)
    {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    if (SDL_UpdateTexture(screen_texture, NULL, pixels.data(), w * sizeof(Uint32)) != 0)
    {
        std::cerr << "SDL_UpdateTexture failed: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(screen_texture);
        return nullptr;
    }

    return screen_texture;
}


SDL_Texture* Graphics::GetTexture(const char* png_path)
{
    SDL_Surface* temp_surface = IMG_Load(png_path);
    SDL_Texture* retreived_texture = SDL_CreateTextureFromSurface(renderer, temp_surface);
    if (NULL == retreived_texture)
    {
        std::cout << "[!] Failed to load texture" << png_path << std::endl;;
        exit(1);
    }
    SDL_FreeSurface(temp_surface);

    return retreived_texture;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
void Graphics::RenderGameItems(Player* player, 
                                std::vector<Projectile*> &game_projectiles, 
                                ItemManager& item_manager, 
                                std::vector<Enemy*>& enemies, 
                                OverlayTextManager& overlay_text_manager, 
                                bool render_coll_boxes,
                                Uint32 game_timer)
{
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // BACKGROUD //
    if ( 0 != SDL_RenderCopy(renderer, texture_map["light_blue_background"], NULL, NULL) ) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Background failed to render.\n";
    }

    // cloud 1
    if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds1_texture"], NULL, &clouds1L_dest, 0.0, NULL, SDL_FLIP_NONE)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds1L failed to render.\n";
    }

    if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds1_texture"], NULL, &clouds1R_dest, 0.0, NULL, SDL_FLIP_HORIZONTAL)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds1R failed to render.\n";
    }

    // cloud 2
    if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds3_texture"], NULL, &clouds2L_dest, 0.0, NULL, SDL_FLIP_NONE)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds2L failed to render.\n";
    }

    if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds3_texture"], NULL, &clouds2R_dest, 0.0, NULL, SDL_FLIP_HORIZONTAL)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds2R failed to render.\n";
    }



    // ITEMS //
    item_manager.DrawItems(renderer, render_coll_boxes, screen_width, screen_height);



    // ENEMIES //
    for (int i = 0; i < enemies.size(); i++)
    {
        if (enemies.at(i)->GetDstRect()->x >= 0 &&
            enemies.at(i)->GetDstRect()->x <= logical_width + enemies.at(i)->GetDstRect()->w &&
            enemies.at(i)->GetDstRect()->y + enemies.at(i)->GetDstRect()->h >= 0 &&
            enemies.at(i)->GetDstRect()->y <= logical_height)
        {
			enemies.at(i)->Draw(renderer, render_coll_boxes);
        }
        else
        {
            enemies.at(i)->UpdateState("delete");
        }
    }
   

    // PROJECTILES //
    for (int i = 0; i < game_projectiles.size(); i++)
    {
        SDL_Rect* r = game_projectiles.at(i)->GetDstRect();

        // Check if completely outside logical screen
        bool out_of_bounds = (r->x + r->w < 0) || (r->x > logical_width) ||
            (r->y + r->h < 0) || (r->y > logical_height);

        if (!out_of_bounds || dynamic_cast<LightningStrike*>(game_projectiles.at(i))) {
            game_projectiles.at(i)->Draw(renderer, render_coll_boxes);
        }
        else {
            game_projectiles.at(i)->UpdateState("delete");
        }
        
    }

	// PLAYER //
    player->Draw(renderer, render_coll_boxes, logical_width, logical_height);

    //  UI //
    if (render_coll_boxes)
        RenderDebugText(player, game_timer);

	RenderPlayerUI(player);

    // OVERLAY TEXT //
	overlay_text_manager.RenderOverlays();

    // SHOW IT //
    SDL_RenderPresent(renderer);
}

void Graphics::RenderText( const std::string& text, const SDL_Rect& rect, SDL_Color color)
{    
    SDL_Surface* surface = TTF_RenderText_Solid(font_1, text.c_str(), color);
    if (!surface) 
    {
        std::cout << "[!] Failed to create text surface: " << TTF_GetError();
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);



    if (!texture) 
    {
        std::cout << "[!] Failed to create text texture: " << SDL_GetError();
        return;
    }

    if (SDL_RenderCopy(renderer, texture, nullptr, &rect) != 0) 
    {
        std::cout << "[!] Failed to render text texture: " << SDL_GetError();
    }

    // Clean up the texture
    SDL_DestroyTexture(texture); 
}

int Graphics::GetScreenWidth()
{
    return logical_width;
}

int Graphics::GetScreenHeight()
{
    return logical_height;
}

int Graphics::GetTrueScreenWidth()
{
    return screen_width;
}

int Graphics::GetTrueScreenHeight()
{
    return screen_height;
}

bool Graphics::IsFrameDone(Uint32 frame_time_ms_ms, Uint32 last_frame_start)
{
    Uint32 current_time = SDL_GetTicks();

    if ((current_time - last_frame_start) >= frame_time_ms_ms)
    {
        return(true);
    }

    else
    {
        //std::cout << "[*] Frame is not done\n";
        return(false);
    }
}

void Graphics::BackgroundUpdate(Uint32 loop_flag)
{
    // CLOUD 1
    
    if (clouds1L_dest.y >= (screen_height)) 
    { 
        std::cout << "[*] loop_flaging cloud 1L back to the top\n";
        clouds1L_dest.x = GenRandomNumber(static_cast<int>(-clouds1L_dest.w * 0.85), static_cast<int>(-clouds1L_dest.w * 0.45)); //INSTEAD OF ALWAYS AT ORIGIN, 
        clouds1L_dest.y = 0 - (clouds1L_dest.h / 2);
        cloud1L_speed = GenRandomNumber(4, 8);
    }

    if (clouds1R_dest.y >= (screen_height))
    {
        std::cout << "[*] loop_flaging cloud 1R back to the top\n";
        clouds1R_dest.x = GenRandomNumber(1050, 1400); //INSTEAD OF ALWAYS AT ORIGIN, RANDOMIZE 
        clouds1R_dest.y = 0 - (clouds1R_dest.h / 2);
        cloud1R_speed = GenRandomNumber(4, 8);
    }
    if (loop_flag % cloud1L_speed == 0)
    {
        clouds1L_dest.y += 1;
    }

    if (loop_flag % cloud1R_speed == 0)
    {
    
        clouds1R_dest.y += 1;
    }

    // CLOUD 2
    if (clouds2L_dest.y >= (screen_height))
    {
        std::cout << "[*] loop_flaging cloud 1L back to the top\n";
        clouds2L_dest.x = GenRandomNumber(-1200, -870);; //INSTEAD OF ALWAYS AT ORIGIN, 
        clouds2L_dest.y = 0 - (clouds2L_dest.h);
        cloud1L_speed = GenRandomNumber(2, 3);
    }

    if (clouds2R_dest.y >= (screen_height))
    {
        std::cout << "[*] loop_flaging cloud 1R back to the top\n";
        clouds2R_dest.x = GenRandomNumber(450, 600); //INSTEAD OF ALWAYS AT ORIGIN, RANDOMIZE 
        clouds2R_dest.y = 0 - (clouds2R_dest.h);
        cloud1R_speed = GenRandomNumber(1, 3);
    }

    if (loop_flag % cloud2L_speed == 0)
    {
        clouds2L_dest.y += 1;
    }

    if (loop_flag % cloud2R_speed == 0)
    {

        clouds2R_dest.y += 1;
    }

    
}

void Graphics::RenderDebugText(Player* player, Uint32 game_timer)
{
    // Weird logic to make the crit string be 2 decimal places.
    float crit_percent = player->GetCrit();
    std::stringstream string_stream;
    string_stream.precision(2);
    string_stream << std::fixed << crit_percent;
    std::string crit_string = string_stream.str();

    std::string player_pos = std::string("X: ") + std::to_string(player->GetDstRect()->x) + std::string(" Y: ") + std::to_string(player->GetDstRect()->y);
    std::string player_damage = std::string("Base Damage ") + std::to_string(static_cast<int>(player->GetBaseDamage()));
    std::string player_crit = std::string("Crit Chance ") + crit_string + "%";
    std::string player_health = std::string("Health ") + std::to_string(static_cast<int>(player->GetCurrentHealth())) + " / " + std::to_string(static_cast<int>(player->GetMaxHealth()));
    std::string player_speed = std::string("Speed ") + std::to_string(static_cast<int>(player->GetSpeed()));
    std::string state = std::string("State: ") + player->GetPlayerState();
    
    int total_seconds = static_cast<int>(game_timer);
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;

    char time_str[6]; // "MM:SS" + null terminator
    snprintf(time_str, sizeof(time_str), "%02d:%02d", minutes, seconds);
    std::string formatted_time = time_str;
    std::string game_timer_str = std::string("Time: ") + time_str;

    RenderText(player_pos, { logical_width - static_cast<int>(player_pos.length()) * 6, 0, static_cast<int>(player_pos.length()) * 6, 15 }, { 0,0,0,0 });
    RenderText(player_damage, { logical_width - static_cast<int>(player_damage.length()) * 6, 15, static_cast<int>(player_damage.length()) * 6, 15 }, { 0,0,0,0 });
    RenderText(player_crit, { logical_width - static_cast<int>(player_crit.length()) * 6, 30, static_cast<int>(player_crit.length()) * 6, 15 }, { 0,0,0,0 });
    RenderText(player_health, { logical_width - static_cast<int>(player_health.length()) * 6, 45, static_cast<int>(player_health.length()) * 6, 15 }, { 0,0,0,0 });
    RenderText(player_speed, { logical_width - static_cast<int>(player_speed.length()) * 6, 60, static_cast<int>(player_speed.length()) * 6, 15 }, { 0,0,0,0 });
    RenderText(state, { logical_width - static_cast<int>(state.length()) * 6, 75, static_cast<int>(state.length()) * 6, 15 }, { 0,0,0,0 });
    RenderText(game_timer_str, { logical_width - static_cast<int>(game_timer_str.length()) * 6, 90, static_cast<int>(game_timer_str.length()) * 6, 15 }, { 0,0,0,0 });

}

void Graphics::RenderPlayerUI(Player* player)
{
    // HEALTH BAR
    

    //POINTS
    std::string points = std::string("Points:") + std::to_string(player->GetPoints());
    RenderText(points, { logical_width / 2 - static_cast<int>(points.length()) * 6 / 2, 0, static_cast<int>(points.length()) * 6, 15 }, { 0,0,0,0 });
}

SDL_Renderer* Graphics::GetRenderer()
{
    return renderer;
}

int Graphics::GenRandomNumber(int low, int high)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(low, high);
    return (distrib(gen));
}

TTF_Font* Graphics::GetFont(int font_id)
{
    switch (font_id)
    {
    case 1: return font_1;
		break;
    case 2: return font_2;
	default: return font_1;
		break;
    }
}


// Might be moved to Pause State Render function instead
void Graphics::RenderPauseMenu(int current_option)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 10); // Semi-transparent black
	SDL_Rect dim_rect = { 0, 0, screen_width, screen_height };
	SDL_RenderFillRect(renderer, &dim_rect);

	std::string resume_text = "Resume";
	std::string restart_text = "Restart";
	std::string quit_text = "Quit";

	SDL_Color normal_color = { 255, 255, 255, 255 };
	SDL_Color highlight_color = { 20, 156, 0, 255 };

	// Calculate positions
	int option_width = 200;
	int option_height = 50;
	int start_y = screen_height / 2 - option_height * 2;

	// Render Resume
	SDL_Color color = (current_option == 0) ? highlight_color : normal_color;
	RenderText(resume_text, { screen_width / 2 - option_width / 2, start_y, option_width, option_height }, color);

	// Render Restart
	color = (current_option == 1) ? highlight_color : normal_color;
	RenderText(restart_text, { screen_width / 2 - option_width / 2, start_y + option_height + 10, option_width, option_height }, color);

	// Render Quit
	color = (current_option == 2) ? highlight_color : normal_color;
	RenderText(quit_text, { screen_width / 2 - option_width / 2, start_y + 2 * (option_height + 10), option_width, option_height }, color);

	// Present the updated renderer
	SDL_RenderPresent(renderer);
}

