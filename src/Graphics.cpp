#include <iostream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <sstream>
#include <random>

#include "Graphics.hpp"
#include "Projectiles.hpp"
#include "ItemManager.hpp"
#include "Enemy.hpp"


class ItemManager;

// Constructor
Graphics::Graphics(const char* title, int width, int height, int scale)
    : window(nullptr), renderer(nullptr), is_shown(false) , clouds1_animation_index(0)
{
    
    screen_width = width;
    screen_height = height;
    pixel_scale = scale;

    // BACKGROUND STUFF
    clouds1_dest = {screen_width - 300, 0, 324 * 5, 576 * 5}; //FILE SIZE * arbitrary scale factor
    //clouds2_dest = { -450, 700, 576 * 4, 324 * 4 }; //FILE SIZE * arbitrary scale factor
    //clouds3_dest = { screen_height - (324 * 5 / 2), 300, 576 * 5, 324 * 5}; //FILE SIZE * arbitrary scale factor

    cloud1_speed = 1;
    cloud2_speed = 3;
    cloud3_speed = 2;

    cloud1_flipped = true;
    cloud2_flipped = false;
    cloud3_flipped = true;
    
    if (init(title, width, height)) 
    {
        is_shown = true;
    }

    LoadTextures();

    font_1 = TTF_OpenFont("../../assets/fonts/bulan-ramadhan-font/BulanRamadhan-YzRYL.ttf",50);
    if (!font_1) 
    {
        std::cout << "[!] font 1 not initialized.";
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
    texture_map["light_blue_background"] = GetTexture("../../assets/sprites/background-sprites/light-blue-background.png");
    texture_map["clouds1_texture"] =  GetTexture("../../assets/sprites/background-sprites/clouds1.png");
    texture_map["clouds2_texture"] = GetTexture("../../assets/sprites/background-sprites/clouds2.png");
    texture_map["clouds3_texture"] = GetTexture("../../assets/sprites/background-sprites/clouds3.png");

    // Player
    texture_map["player_main_texture"] = GetTexture("../../assets/sprites/player-sprites/zephyr.png");
    texture_map["player_secondary_fire_hud_texture"] = GetTexture("../../assets/sprites/player-sprites/secondary_fire_hud.png");
    texture_map["player_secondary_fire_marker_texture"] =  GetTexture("../../assets/sprites/player-sprites/secondary_fire_marker.png");
    texture_map["player_shield"] = GetTexture("../../assets/sprites/player-sprites/player_shield.png");

    // Projectiles
    texture_map["primary_fire"] = GetTexture("../../assets/sprites/projectile-sprites/primary_fire.png");
    texture_map["primary_fire_impact"] = GetTexture("../../assets/sprites/projectile-sprites/primary_fire_impact.png");
    texture_map["secondary_fire"] = GetTexture("../../assets/sprites/projectile-sprites/secondary_fire.png");
    texture_map["secondary_fire_impact"] = GetTexture("../../assets/sprites/projectile-sprites/secondary_fire_impact.png");
    //texture_map["purple_crystal_projectile"] =  GetTexture("../../assets/sprites/projectile-sprites/purple_projectile.png");
    //texture_map["purple_crystal_projectile_impact"] = GetTexture("../../assets/sprites/projectile-sprites/purple_crystal_impact.png");
    texture_map["ice_shard_conjure"] = GetTexture("../../assets/sprites/projectile-sprites/Ice_Shard_Cast.png");
    texture_map["ice_shard"] = GetTexture("../../assets/sprites/projectile-sprites/Ice_shard.png");
    texture_map["ice_shard_impact"] = GetTexture("../../assets/sprites/projectile-sprites/Ice_Shard_Hit.png");

    // Items
    texture_map["item_cloud"] = GetTexture("../../assets/sprites/item-sprites/item_cloud.png");
    texture_map["glass_toucan"] = GetTexture("../../assets/sprites/item-sprites/glass_toucan.png");

    // Enemies
    texture_map["purple_crystal_main"] = GetTexture("../../assets/sprites/enemies-sprites/light_blue.png");
    texture_map["purple_crystal_death"] = GetTexture("../../assets/sprites/enemies-sprites/light_blue_destr.png");
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
                              width, height,
                              SDL_WINDOW_SHOWN);
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

    return true;
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
void Graphics::RenderGameItems(Player* player, std::vector<Projectile*> &game_projectiles, std::vector<ItemManager::item>* item_list, std::vector<Enemy*>& enemies)
{
    
    bool draw_collision_boxes = false;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    if (NULL == player->GetDstRect())
    {
        std::cerr << "[!] DstRect for the player is NULL.";
    }


    // BACKGROUD //
    if ( 0 != SDL_RenderCopy(renderer, texture_map["light_blue_background"], NULL, NULL) ) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Background failed to render.\n";
    }

    // cloud 1
    if (!cloud1_flipped)
    {
        if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds1_texture"], NULL, &clouds1_dest, 0.0, NULL, SDL_FLIP_NONE)) //Second arg NULL means use whole png.
        {
            std::cout << "[!] Clouds1 failed to render.\n";
        }
    }
    else
    {
        if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds1_texture"], NULL, &clouds1_dest, 0.0, NULL, SDL_FLIP_HORIZONTAL)) //Second arg NULL means use whole png.
        {
            std::cout << "[!] Clouds1 failed to render.\n";
        }
    }

    // cloud 3
    /*if (!cloud3_flipped)
    {
        if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds3_texture"], NULL, &clouds3_dest, 0.0, NULL, SDL_FLIP_NONE)) //Second arg NULL means use whole png.
        {
            std::cout << "[!] Clouds1 failed to render.\n";
        }
    }
    else
    {
        if (0 != SDL_RenderCopyEx(renderer, texture_map["clouds3_texture"], NULL, &clouds3_dest, 0.0, NULL, SDL_FLIP_HORIZONTAL)) //Second arg NULL means use whole png.
        {
            std::cout << "[!] Clouds1 failed to render.\n";
        }
    }

    /*if (0 != SDL_RenderCopy(renderer, texture_map["clouds3_texture"], NULL, &clouds3_dest)) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Clouds3 failed to render.\n";
    }*/

    // ITEMS //
    for (int i = 0; i < (*item_list).size(); i++)
    {  
        if ((*item_list).at(i).item_dest_rect.x >= 0 && 
        (*item_list).at(i).item_dest_rect.x  <= screen_width - (*item_list).at(i).item_dest_rect.w &&
        (*item_list).at(i).item_dest_rect.y + (*item_list).at(i).item_dest_rect.h >= 0 && 
        (*item_list).at(i).item_dest_rect.y  <= screen_height) 
        {
            if ( 0 != SDL_RenderCopy(renderer, texture_map["item_cloud"], NULL, &(*item_list).at(i).item_cloud_dest_rect)) //Second arg NULL means use whole png.
                {
                    std::cout << "[!] Item failed to render.\n";
                }
            
            if ( 0 != SDL_RenderCopy(renderer, texture_map["glass_toucan"], NULL, &(*item_list).at(i).item_dest_rect)) //Second arg NULL means use whole png.
                {
                    std::cout << "[!] Item failed to render.\n";
                }

            if (draw_collision_boxes)
            {
         
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawRect(renderer, &(*item_list).at(i).item_cloud_dest_rect);
                SDL_RenderDrawRect(renderer, &(*item_list).at(i).item_dest_rect);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &(*item_list).at(i).item_cloud_coll_rect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
            }
        }
        else
        {
            //std::cout << "[*] Need to get rid of this proj\n";
            (*item_list).erase((*item_list).begin() + i);
        }
        
    }


    // ENEMIES //
    for (int i = 0; i < enemies.size(); i++)
    {
        if (enemies.at(i)->GetDstRect()->x >= 0 &&
            enemies.at(i)->GetDstRect()->x <= screen_width + enemies.at(i)->GetDstRect()->w &&
            enemies.at(i)->GetDstRect()->y + enemies.at(i)->GetDstRect()->h >= 0 &&
            enemies.at(i)->GetDstRect()->y <= screen_height)
        {

            if (0 != SDL_RenderCopy(renderer, texture_map[enemies.at(i)->GetTextureKey()], enemies.at(i)->GetFrame(), enemies.at(i)->GetDstRect())) //Second arg NULL means use whole png.
            {
                std::cout << "[!] " << "TextureKey: " << enemies.at(i)->GetTextureKey() << std::endl;
                std::cout << "[!] " << "TextureKey: " << enemies.at(i)->GetFrame()->x << enemies.at(i)->GetFrame()->y << enemies.at(i)->GetFrame()->w << enemies.at(i)->GetFrame()->h << std::endl;
                std::cout << "[!] " << "TextureKey: " << enemies.at(i)->GetDstRect()->x << enemies.at(i)->GetDstRect()->y << enemies.at(i)->GetDstRect()->w << enemies.at(i)->GetDstRect()->h << std::endl;
                std::cout << "[!] Enemy failed to render.\n"; 
            }

            if (draw_collision_boxes)
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawRect(renderer, enemies.at(i)->GetCollRect());
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
        }
        else
        {
            enemies.at(i)->UpdateState("delete");
        }
    }
   

    // PROJECTILES //
    for (int i = 0; i < game_projectiles.size(); i++)
    {
        
        if (game_projectiles.at(i)->GetDstRect()->x >= 0 && 
        game_projectiles.at(i)->GetDstRect()->x  <= screen_width - game_projectiles.at(i)->GetDstRect()->w &&
        game_projectiles.at(i)->GetDstRect()->y + game_projectiles.at(i)->GetDstRect()->h >= 0 && 
        game_projectiles.at(i)->GetDstRect()->y  <= screen_height) 
        {   

            if (0 != SDL_RenderCopy(renderer, texture_map[game_projectiles.at(i)->GetTextureKey()], game_projectiles.at(i)->GetFrame(), game_projectiles.at(i)->GetDstRect())) //Second arg NULL means use whole png.
                {
                    std::cout << "[!] Proj failed to render.\n";
                    std::cout << "[!] " << game_projectiles.at(i)->GetFrame()->x << game_projectiles.at(i)->GetFrame()->y << game_projectiles.at(i)->GetFrame()->w << game_projectiles.at(i)->GetFrame()->h << std::endl;
                    std::cout << "[!] Texture Key: " << game_projectiles.at(i)->GetTextureKey() << std::endl;
                }
            if (draw_collision_boxes)
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawRect(renderer, game_projectiles.at(i)->GetCollisionRect());
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawRect(renderer, game_projectiles.at(i)->GetDstRect());
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
        }
        // If they're not in the screen delete them
        else
        {
            game_projectiles.at(i)->UpdateState("delete");
        }
        
    }

    // PLAYER //
                    // Main Player Render
    if ( 0 != SDL_RenderCopy(renderer, texture_map["player_main_texture"], player->GetFrame(), player->GetDstRect())) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Player failed to render.\n";
    }
                    // Render the secondary fire hud
    if ( 0 != SDL_RenderCopy(renderer, texture_map["player_secondary_fire_hud_texture"], NULL, player->GetSecondaryFirePosition())) //Second arg NULL means use whole png.
    {
        std::cout << "[!] Secondary Fire HUD failed to render.\n";
    }
                    // If sec. fire marker is active, render it
    if (player->IsSecondaryFireMarkerActive())
    {
        SDL_RenderCopy(renderer, texture_map["player_secondary_fire_marker_texture"], NULL, player->GetSecondaryFireMarkerPosition());
    }
                    // If shield is active render it
    if (player->GetPlayerState() == "shield")
    {
        SDL_RenderCopy(renderer, texture_map["player_shield"], player->GetShieldFrame(), player->GetShieldDstRect());
    }

    if (draw_collision_boxes)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
        SDL_RenderDrawRect(renderer, player->GetCollRect());
        SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
        SDL_RenderDrawRect(renderer, player->GetShieldColl());
        SDL_RenderDrawRect(renderer, player->GetSecondaryFireHudColl());
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawRect(renderer, player->GetSecondaryFireMarkerCollision());

        //SDL_RenderDrawRect(renderer, player->GetSecondaryFirePosition());
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); 
        SDL_RenderDrawRect(renderer, player->GetDstRect());

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
    }

    // PLAYER UI //
    RenderPlayerText(player);



    SDL_RenderPresent(renderer);

    // INCREMENT FRAMES //
    // Projectiles
    for (int i = 0; i < game_projectiles.size(); i++)
    {
        
        if (IsFrameDone(game_projectiles.at(i)->frame_time_ms, game_projectiles.at(i)->last_frame_time))
        {
            game_projectiles.at(i)->last_frame_time = SDL_GetTicks();
            if (game_projectiles.at(i)->animation_replayable)
            {
                if (game_projectiles.at(i)->current_frame_index == game_projectiles.at(i)->NumOfFrames() - 1)
                    game_projectiles.at(i)->current_frame_index = 0;
                else
                    game_projectiles.at(i)->AdvanceFrame();
            }

            else
            {
                if (game_projectiles.at(i)->current_frame_index < game_projectiles.at(i)->NumOfFrames() - 1)
                    game_projectiles.at(i)->AdvanceFrame();
            }
        }
    }
    //Items
    for (int i = 0; i < (*item_list).size(); i++)
    {

        //if ((*item_list).at(i).item_dest_rect.x >= 0 &&
    }

    //player
    if (IsFrameDone(player->GetFrameTime(), player->GetLastFrameStart()))
    {
        player->SetLastFrameTime(SDL_GetTicks());
        /*if (game_projectiles.at(i)->animation_replayable)
        {
            if (game_projectiles.at(i)->current_frame_index == game_projectiles.at(i)->NumOfFrames() - 1)
                game_projectiles.at(i)->current_frame_index = 0;
            else
                game_projectiles.at(i)->AdvanceFrame();
        }*/

        //else
        //{
        if (player->GetFrameIndex() < player->NumOfFrames() - 1)
            player->AdvanceFrame();
        else
            player->SetFrameIndex(0);
        //std::cout << "[*] Player frame index: " << player->GetFrameIndex() << std::endl;
        //}
    }
    if ( player->GetPlayerState() == "shield" && IsFrameDone(player->GetShieldFrameTime(), player->GetLastShieldFrameStart()))
    {
        std::cout << "[*] Shield Frame is done !\n";
        player->AdvanceShieldFrame();
        player->SetShieldLastFrameTime(SDL_GetTicks());
    }

    // ENEMEY INCREMENT FRAME
    for (int i = 0; i < enemies.size(); i++)
    {
        if (IsFrameDone(enemies.at(i)->GetFrameTime(), enemies.at(i)->GetLastFrameStart()))
        {
            enemies.at(i)->SetLastFrameTime(SDL_GetTicks());

            if (enemies.at(i)->GetFrameIndex() < enemies.at(i)->NumOfFrames() - 1)
                enemies.at(i)->AdvanceFrame();
            else
                enemies.at(i)->SetFrameIndex(0);
        }
    }

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
    return screen_width;
}

int Graphics::GetScreenHeight()
{
    return screen_height;
}

bool Graphics::IsFrameDone(Uint32 frame_time_ms, Uint32 last_frame_start)
{
    Uint32 current_time = SDL_GetTicks();

    if ((current_time - last_frame_start) >= frame_time_ms)
    {
        return(true);
    }

    else
    {
        //std::cout << "[*] Frame is not done\n";
        return(false);
    }
}

void Graphics::BackgroundUpdate(Uint32 loop)
{
    // CLOUD 1
    if (clouds1_dest.y >= (screen_height)) 
    { 
        if (GenRandomNumber(0, 1000) % 2 == 0)
        {
            cloud1_flipped = false;
            std::cout << "[*] Cloud 1 normal\n";
            clouds1_dest.x = GenRandomNumber(-1200, -870);; //INSTEAD OF ALWAYS AT ORIGIN, RANDOMIZE 
        }
        else
        {
            cloud1_flipped = true;
            std::cout << "[*] Cloud 1 FLIPPED\n";
            clouds1_dest.x = GenRandomNumber(screen_width - 750, screen_width - 400); //INSTEAD OF ALWAYS AT ORIGIN, RANDOMIZE 
        }
        
        clouds1_dest.y = 0 - clouds1_dest.h;
        cloud1_speed = GenRandomNumber(1, 1);
    }

    if (loop % cloud1_speed == 0)
    {
        clouds1_dest.y += 1;
    }

    // CLOUD 3
    /*if (clouds3_dest.y >= (screen_height + 128))
    {
        if (GenRandomNumber(0, 1000) % 2 == 0)
        {
            cloud1_flipped = false; 
            std::cout << "[*] Cloud 3 normal\n";
            clouds3_dest.x = GenRandomNumber(-500, -300); //INSTEAD OF ALWAYS AT ORIGIN, RANDOMIZE 
        }
        else
        {
            cloud1_flipped = true;
            std::cout << "[*] Cloud 3 FLIPPED\n";
            clouds3_dest.x = GenRandomNumber(screen_width + 500, screen_width + 700); //INSTEAD OF ALWAYS AT ORIGIN, RANDOMIZE 
        }



        clouds3_dest.y = GenRandomNumber(100, screen_height * 0.4);
        cloud3_speed = GenRandomNumber(3, 7);
    }

    if (loop % cloud3_speed == 0)
    {
        if (cloud3_flipped)
            clouds3_dest.x -= 2;
        else
            clouds3_dest.x += 2;
        clouds3_dest.y += 1;
    }*/

    // CLOUD 2
    /*if (clouds3_dest.x >= screen_width || clouds3_dest.y >= (screen_height + 256))
    {
        clouds3_dest.x = -300; //INSTEAD OF ALWAYS AT ORIGIN, RANDOMIZE
        clouds3_dest.y = 300;
    }*/
    //clouds3_dest.x += 1;
    //clouds3_dest.y += 1;
    
}

void Graphics::RenderPlayerText(Player* player)
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
    std::string player_health = std::string("Health ") + std::to_string(static_cast<int>(player->GetHealth()));
    std::string player_speed = std::string("Speed ") + std::to_string(static_cast<int>(player->GetSpeed()));

    std::string state = std::string("State: ") + player->GetPlayerState();
    
    
    
    RenderText(player_pos, { 1500, 930, static_cast<int>(player_pos.length()) * 30, 50 }, { 0,0,0,0 });
    RenderText(player_damage, { 1450, 830, static_cast<int>(player_damage.length()) * 30, 50 }, { 0,0,0,0 });
    RenderText(player_crit, { 1450, 770, static_cast<int>(player_crit.length()) * 30, 50 }, { 0,0,0,0 });
    RenderText(player_health, { 1450, 740, static_cast<int>(player_health.length()) * 30, 50 }, { 0,0,0,0 });
    RenderText(player_speed, { 1450, 710, static_cast<int>(player_speed.length()) * 30, 50 }, { 0,0,0,0 });
    RenderText(state, { 680, 930, static_cast<int>(state.length()) * 30, 50 }, { 0,0,0,0 });
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

